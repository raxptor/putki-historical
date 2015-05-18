#include "db.h"

#include <map>
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <sstream>

#include <cstdlib>
#include <cstdio>

#include <ctime>

#include <putki/sys/compat.h>
#include <putki/sys/thread.h>
#include <putki/sys/sstream.h>

#include <putki/builder/write.h>
#include <putki/builder/log.h>

extern "C" {
	#include <md5/md5.h>
}

namespace putki
{

	namespace db
	{
		struct entry
		{
			type_handler_i *th;
			instance_t obj;
			std::vector<std::string> auxrefs;
			std::string signature;
		};
		
		struct alloc_entry
		{
			type_handler_i *th;
			instance_t obj;
		};

		struct deferred
		{
			sys::condition cond;
			bool loading;
			int waiting;
			deferred_load_fn fn;
			void *userptr;
		};

		struct on_destroy
		{
			on_destroy_fn fn;
			void *userptr;
		};

		struct data
		{
			std::map<std::string, entry> objs;
			std::vector<alloc_entry> overwritten;
			std::map<instance_t, std::string> paths;
			std::set<const char *> unresolved;
			std::map<std::string, const char *> strpool;
			std::vector<on_destroy> ondestroy;
			sys::mutex *mtx;
			std::map<std::string, struct deferred> deferred;
			
			sys::condition isloading_cond;
			std::set<std::string> isloading;
			char auxpathbuf[256];
			data *parent;
			bool erase_on_overwrite;
		};

		db::data * create(data *parent, sys::mutex *mtx)
		{
			data *d = new data();
			d->parent = parent;
			d->mtx = mtx;
			d->erase_on_overwrite = false;
			return d;
		}
		
		void enable_erase_on_overwrite(data *d)
		{
			d->erase_on_overwrite = true;
		}

		void register_on_destroy(data *d, on_destroy_fn fn, void *userptr)
		{
			on_destroy od;
			od.fn = fn;
			od.userptr = userptr;
			d->ondestroy.push_back(od);
		}

		void free_and_destroy_objs(data *d)
		{
			for (std::map<std::string, entry>::iterator i=d->objs.begin(); i!=d->objs.end(); i++)
				i->second.th->free(i->second.obj);
			db::free(d);
		}

		void free(data *d, data *unres_target)
		{
			// all the strdup:ed strings
			if (unres_target)
			{
				sys::scoped_maybe_lock _lk(unres_target->mtx);
				for (std::set<const char*>::iterator i = d->unresolved.begin(); i!=d->unresolved.end(); i++)
					unres_target->unresolved.insert((*i));
			}
			else
			{
				for (std::set<const char*>::iterator i = d->unresolved.begin(); i!=d->unresolved.end(); i++)
					::free(const_cast<char*>(*i));
			}
			
			for (std::vector<on_destroy>::iterator i = d->ondestroy.begin(); i != d->ondestroy.end(); i++)
				i->fn(i->userptr);

			delete d;
		}

		void split_aux_path(std::string path, std::string * base, std::string * ref)
		{
			int p = path.find_last_of('#');
			if (p != std::string::npos)
			{
				*base = path.substr(0, p);
				*ref = path.substr(p, path.size() - p);
			}
			else
			{
				*base = path;
				ref->clear();
			}
		}

		bool base_asset_path(const char *path, char *result, unsigned int bufsize)
		{
			if (!is_aux_path(path)) {
				return false;
			}

			std::string base, ref;
			split_aux_path(path, &base, &ref);
			if (base.size() < bufsize)
			{
				strcpy(result, base.c_str());
				return true;
			}
			return false;
		}

		bool is_aux_path(const char *path)
		{
			return strchr(path, '#') != 0;
		}

		const char *auxref(data *d, const char *path, unsigned int index)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				if (index < i->second.auxrefs.size())
					return i->second.auxrefs[index].c_str();
			}
			return 0;
		}

		const char *pathof(data *d, instance_t obj)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			std::map<instance_t, std::string>::iterator i = d->paths.find(obj);
			if (i != d->paths.end()) {
				return i->second.c_str();
			}
			_lk.unlock();

			if (d->parent)
			{
				return pathof(d->parent, obj);
			}
			return 0;
		}

		const char *pathof_including_unresolved(data *d, instance_t obj)
		{
			const char *unres = is_unresolved_pointer(d, obj);
			if (unres) return unres;

			if (d->parent) unres = is_unresolved_pointer(d->parent, obj);
			if (unres) return unres;

			return pathof(d, obj);
		}

		const char *signature(data *d, const char *path, char *buffer=0)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				entry e = i->second;
				_lk.unlock();
				
				putki::sstream ss;
				write::write_object_into_stream(ss, d, e.th, e.obj);

				char signature[16];
				static char unsafe_buffer[64];

				if (!buffer)
				{
					APP_WARNING("Not using buffer for db::signature")
					buffer = unsafe_buffer;
				}

				md5_buffer(ss.c_str(), ss.size(), signature);
				md5_sig_to_string(signature, buffer, 64);
				return buffer;

			}
			return "NO-SIG";
		}
		
		void insert_deferred(data *data, const char *path, deferred_load_fn fn, void *userptr)
		{
			sys::scoped_maybe_lock _lk(data->mtx);
			deferred d;
			d.fn = fn;
			d.userptr = userptr;
			d.loading = false;
			d.waiting = 0;
			data->deferred[path] = d;
		}

		void copy_obj(data *source, data *dest, const char *path)
		{
			sys::scoped_maybe_lock _lk0(source->mtx);
			sys::scoped_maybe_lock _lk1(dest->mtx);
		
			std::map<std::string, entry>::iterator i = source->objs.find(path);
			if (i != source->objs.end())
			{
				dest->objs[path] = i->second;
				dest->paths[i->second.obj] = path;
//				std::cout << " +++ Copy obj " << path << std::endl;
				return;
			}

			std::map<std::string, deferred>::iterator j = source->deferred.find(path);
			if (j != source->deferred.end())
			{
				i = dest->objs.find(path);
				if (i != dest->objs.end())
					dest->objs.erase(i);

//				std::cout << " +++ Copy deferred " << path << std::endl;
				dest->deferred[path].fn = j->second.fn;
				dest->deferred[path].userptr = j->second.userptr;
				dest->deferred[path].loading = false;
				dest->deferred[path].waiting = 0;
				return;
			}

			std::cout << "DB FAILED TO COPY OBJ " << path << " BECAUSE DID NOT EXIST" << std::endl;
		}

		void copy_unresolved(data *source, data *target)
		{
		
		}

		void insert(data *d, const char *path, type_handler_i *th, instance_t i)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			APP_DEBUG("DB:" << d << " db insert on path [" << path << "] obj=" << i << "th=" << th);
			//APP_DEBUG("Type:" << th->name() << " id:" << th->id())
			
			if (d->erase_on_overwrite)
			{
				std::map<std::string, entry>::iterator old = d->objs.find(path);
				if (old != d->objs.end() && old->second.obj != i)
				{
					APP_DEBUG("Erasing old overwritten object..")
					old->second.th->free(old->second.obj);
				}
			}
			else
			{
				std::map<std::string, entry>::iterator old = d->objs.find(path);
				if (old != d->objs.end() && old->second.obj != i)
					APP_WARNING("Overwriting object, but I will not erase on overwrite! (" << path << ")")
			}
			
			entry e;
			e.th = th;
			e.obj = i;
			d->objs[path] = e;
			d->paths[i] = path;

			if (is_aux_path(path))
			{
				// add to auxrefs to quickly find them.
				std::string base, ref;
				split_aux_path(path, &base, &ref);
				std::map<std::string, entry>::iterator i = d->objs.find(base);
				if (i != d->objs.end()) {
					i->second.auxrefs.push_back(ref);
				}
			}
		}

		bool is_aux_path_of(data *d, instance_t baseobj, const char *path)
		{
			std::string base, ref;
			split_aux_path(path, &base, &ref);
			return !strcmp(base.c_str(), pathof(d, baseobj));
		}

		const char *make_aux_path(data *d, instance_t onto)
		{
			sys::scoped_maybe_lock _lk(d->mtx);		
			std::map<instance_t, std::string>::iterator i = d->paths.find(onto);
			if (i != d->paths.end())
			{
				int q = 0;
				const char *digits = "0123456789abcdef";
				do
				{
					// nice!
					int t = time(0) + rand();
					char ap[16];
					for (int j=0;j<6;j++)
						ap[j] = digits[ (t >> j*4) & 0xf];
					ap[6] = 0;
						
					sprintf(d->auxpathbuf, "%s#%s", i->second.c_str(), ap);
				}
				while (d->objs.find(d->auxpathbuf) != d->objs.end());
				return d->auxpathbuf;
			}
			return "<INVALID-AUX-PATH>";
		}
		
		bool exists(data *d, const char *path, bool include_loading)
		{
			sys::scoped_maybe_lock _lk(d->mtx);

			if (!include_loading && d->isloading.count(path))
				return false;

			return d->objs.find(path) != d->objs.end() || d->deferred.find(path) != d->deferred.end();
		}

		bool start_loading(data *d, const char *path)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			
			// already loaded
			if (d->objs.find(path) != d->objs.end())
				return false;
			
			// If still loading and not inserted, it is being processed by a loader
			// We are thus waiting for disk I/O
			bool was_loading = false;
			while (d->isloading.count(path) && d->objs.find(path) == d->objs.end())
			{
				d->isloading_cond.wait(d->mtx);
				was_loading = true;
			}
			
			if (was_loading || d->objs.find(path) != d->objs.end())
			{
				// load completed with fail, or we see the object in the db.
				return false;
			}
			else
			{
				// object either existed or was not being loaded.
				if (d->isloading.count(path)) APP_ERROR("Logic erorr because is loading already");
				d->isloading.insert(path);
				return true;
			}
		}
		
		void done_loading(data *d, const char *path)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			d->isloading.erase(d->isloading.find(path));
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end()) 
			{
				// clear loads for all auxrefs
				for (unsigned int k=0;k!=i->second.auxrefs.size();k++)
				{
					std::string & str = i->second.auxrefs[k];
					d->isloading.erase(d->isloading.find(std::string(path) + str));
				}
			}
			
			d->isloading_cond.broadcast();
//			APP_DEBUG("erased from load queue " << path);
		}

		bool fetch(data *d, const char *path, type_handler_i **th, instance_t *obj, bool allow_execute_deferred, bool iamtheloader)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			while (true)
			{
				if (!iamtheloader)
				{
					if (d->isloading.count(path))
					{
						d->isloading_cond.wait(d->mtx);
						continue;
					}
				}

				std::map<std::string, entry>::iterator i = d->objs.find(path);
				if (i != d->objs.end())
				{
					*th = i->second.th;
					*obj = i->second.obj;
					return true;
				}

				if (!allow_execute_deferred)
				{
					return false;
				}
			
				std::map<std::string, deferred>::iterator j = d->deferred.find(path);
				if (j == d->deferred.end())
				{
					return false;
				}
					
				if (j != d->deferred.end())
				{
					if (j->second.loading)
					{
						j->second.waiting++;
						j->second.cond.wait(d->mtx);
						if (!-- j->second.waiting)
							d->deferred.erase(j);
						continue;
					}
				}
				
				j->second.loading = true;
				
				deferred_load_fn fn = j->second.fn;
				void *userptr = j->second.userptr;
				
				if (d->mtx)
					d->mtx->unlock();
					
				bool succ = fn(d, path, th, obj, userptr);
				
				if (d->mtx)
					d->mtx->lock();
			
				j = d->deferred.find(path);
				if (!j->second.loading)
					APP_ERROR("Not loading any more");
				
				j->second.cond.broadcast();
				
				if (!j->second.waiting)
					d->deferred.erase(j);
					
				if (!succ)
				{
					APP_WARNING("Deferred loading of " << path << " failed!");
				}
				
				return succ;
			}
		}

		instance_t ptr_to_allow_unresolved(data *d, const char *path)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end()) {
				return i->second.obj;
			}
			_lk.unlock();
			return create_unresolved_pointer(d, path);
		}

		void read_all_no_fetch(data *d, enum_i *eobj)
		{
			std::vector< std::pair<std::string, std::pair<type_handler_i*, instance_t> > > objs;
			std::vector< std::string > defs;

			sys::scoped_maybe_lock _lk(d->mtx);
			std::map<std::string, entry>::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				if (d->isloading.count(i->first))
				{
					i++;
					continue;
				}
			
				objs.push_back(std::make_pair(i->first, std::make_pair(i->second.th, i->second.obj)));
				++i;
			}
			
			// deferred
			std::map<std::string, deferred>::iterator j = d->deferred.begin();
			while (j != d->deferred.end())
			{
				if (d->isloading.count(j->first))
				{
					j++;
					continue;
				}
				defs.push_back((j)->first);
				j++;
			}
			
			_lk.unlock();
			
			
			for (unsigned int k=0;k<objs.size();k++)
				eobj->record(objs[k].first.c_str(), objs[k].second.first, objs[k].second.second);
			for (unsigned int k=0;k<defs.size();k++)
				eobj->record(defs[k].c_str(), 0, 0);
		}

		void read_all(data *d, enum_i *eobj)
		{
			std::set<std::string> paths;
	
			// insert all deferred
			sys::scoped_maybe_lock _lk(d->mtx);
			std::map<std::string, deferred>::iterator i = d->deferred.begin();
			while (i != d->deferred.end())
			{
				if (d->isloading.count(i->first))
				{
					i++;
					continue;
				}
				paths.insert((i++)->first);
			}
			
			// and
			std::map<std::string, entry>::iterator j = d->objs.begin();
			while (j != d->objs.end())
			{
				if (d->isloading.count(j->first))
				{
					j++;
					continue;
				}
			
				paths.insert((j++)->first);
			}
	
			_lk.unlock();
		
			for (std::set<std::string>::iterator i=paths.begin();i!=paths.end();i++)
			{
				type_handler_i *th;
				instance_t obj;
				if (db::fetch(d, (*i).c_str(), &th, &obj))
				{
					eobj->record((*i).c_str(), th, obj);
				}
			}
		}

		unsigned int size(data *d)
		{
			return (unsigned int) d->objs.size();
		}

		instance_t create_unresolved_pointer(data *d, const char *path)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
		
			std::map<std::string, const char *>::iterator i = d->strpool.find(path);
			if (i != d->strpool.end()) {
				return (instance_t) i->second;
			}

			char *str = strdup(path);
			d->unresolved.insert(str);
			d->strpool[path] = str;
			return (instance_t) str;
		}

		const char *is_unresolved_pointer(data *d, void *p)
		{
			sys::scoped_maybe_lock _lk(d->mtx);
			if (d->unresolved.count((const char*)p) != 0)
			{
				return (const char*) p;
			}
			else
			{
				if (d->parent)
				{
					_lk.unlock();
					return is_unresolved_pointer(d->parent, p);
				}
				return 0;
			}
		}
	}
}

