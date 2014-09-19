#include "db.h"

#include <map>
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <sstream>

#include <cstdlib>
#include <cstdio>

#include <putki/sys/compat.h>

#include <putki/builder/write.h>

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

		struct deferred
		{
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
			std::map<instance_t, std::string> paths;
			std::set<const char *> unresolved;
			std::map<std::string, const char *> strpool;
			std::map<std::string, deferred> deferred;
			std::vector<on_destroy> ondestroy;
			char auxpathbuf[256];
			data *parent;
		};

		db::data * create(data *parent)
		{
			data *d = new data();
			d->parent = parent;
			return d;
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

		void free(data *d)
		{
			// all the strdup:ed strings
			for (std::set<const char*>::iterator i = d->unresolved.begin(); i!=d->unresolved.end(); i++)
				::free(const_cast<char*>(*i));

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
			std::map<instance_t, std::string>::iterator i = d->paths.find(obj);
			if (i != d->paths.end()) {
				return i->second.c_str();
			}

			if (d->parent)
			{
				return pathof(d->parent, obj);
			}

			return 0;
		}

		const char *pathof_including_unresolved(data *d, instance_t obj)
		{
			const char *unres = is_unresolved_pointer(d, obj);
			if (unres) {
				return unres;
			}
			return pathof(d, obj);
		}

		const char *signature(data *d, const char *path)
		{
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				std::stringstream ss;
				write::write_object_into_stream(ss, d, i->second.th, i->second.obj);

				char signature[16];
				static char signature_string[64];

				md5_buffer(ss.str().c_str(), ss.str().size(), signature);
				md5_sig_to_string(signature, signature_string, 64);

				return signature_string;

			}
			return "NO-SIG";
		}
		
		void insert_deferred(data *data, const char *path, deferred_load_fn fn, void *userptr)
		{
			deferred d;
			d.fn = fn;
			d.userptr = userptr;
			data->deferred[path] = d;
		}

		void copy_obj(data *source, data *dest, const char *path)
		{
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
				dest->deferred[path] = j->second;
				return;
			}

			std::cout << "DB FAILED TO COPY OBJ " << path << " BECAUSE DID NOT EXIST" << std::endl;
		}

		void insert(data *d, const char *path, type_handler_i *th, instance_t i)
		{
//			std::cout << "DB:" << d << " db insert on path [" << path << "] obj " << i << std::endl;
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
			std::map<instance_t, std::string>::iterator i = d->paths.find(onto);
			if (i != d->paths.end())
			{
				do
				{
					// nice!
					sprintf(d->auxpathbuf, "%s#%c%c%c%c", i->second.c_str(), 'a' + (rand()%20), 'a' + (rand()%20), 'a' + (rand()%20), 'a' + (rand()%20));
				}
				while (d->objs.find(d->auxpathbuf) != d->objs.end());
				return d->auxpathbuf;
			}
			return "<INVALID-AUX-PATH>";
		}
		
		bool exists(data *d, const char *path)
		{
			return d->objs.find(path) != d->objs.end() || d->deferred.find(path) != d->deferred.end();
		}

		bool fetch(data *d, const char *path, type_handler_i **th, instance_t *obj, bool allow_execute_deferred)
		{		
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				*th = i->second.th;
				*obj = i->second.obj;
				return true;
			}

			if (!allow_execute_deferred)
				return false;
			
			std::map<std::string, deferred>::iterator j = d->deferred.find(path);
			if (j != d->deferred.end())
			{
				bool succ = j->second.fn(d, path, th, obj, j->second.userptr);
				d->deferred.erase(j);
				
				if (!succ)
				{
					std::cout << "DEFERRED LOAD OF " << path << " FAILED!" << std::endl;
				}
				
				return succ;
			}
		
			return false;
		}

		instance_t ptr_to_allow_unresolved(data *d, const char *path)
		{
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end()) {
				return i->second.obj;
			}
			return create_unresolved_pointer(d, path);
		}

		void read_all_no_fetch(data *d, enum_i *eobj)
		{
			// actually loaded
			std::map<std::string, entry>::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				eobj->record(i->first.c_str(), i->second.th, i->second.obj);
				++i;
			}
			
			// deferred
			std::map<std::string, deferred>::iterator j = d->deferred.begin();
			while (j != d->deferred.end())
			{
				eobj->record(j->first.c_str(), 0, 0);
				++j;
			}
		}

		void read_all(data *d, enum_i *eobj)
		{
			if (!d->deferred.empty())
			{
				std::cout << "db::read_all forced to realize reads! (" << d->deferred.size() << " entries)" << std::endl;
				while (!d->deferred.empty())
				{
					std::string name = d->deferred.begin()->first.c_str();
					type_handler_i *th;
					instance_t obj;
					// this will erase from the map
					db::fetch(d, name.c_str(), &th, &obj);
				}
			}
		
			std::map<std::string, entry>::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				eobj->record(i->first.c_str(), i->second.th, i->second.obj);
				++i;
			}
		}

		unsigned int size(data *d)
		{
			return (unsigned int) d->objs.size();
		}

		instance_t create_unresolved_pointer(data *d, const char *path)
		{
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
			if (d->unresolved.count((const char*)p) != 0)
			{
				return (const char*) p;
			}
			else
			{
				if (d->parent)
				{
					return is_unresolved_pointer(d->parent, p);
				}
				return 0;
			}
		}


	}
}

