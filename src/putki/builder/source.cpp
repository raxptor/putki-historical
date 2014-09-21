#include "source.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include <putki/sys/files.h>
#include <putki/sys/thread.h>

#include <putki/builder/parse.h>
#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <putki/builder/log.h>

namespace putki
{
	namespace
	{
		struct load_resolver_store_db_ref : public load_resolver_i
		{
			db::data *db;
			std::string objpath;

			void resolve_pointer(instance_t *ptr, const char *path)
			{
				if (path[0] == '#')
				{
					// parse out aux directly
					std::string tmp = objpath + path;
					*ptr = (instance_t) db::create_unresolved_pointer(db, tmp.c_str());
				}
				else
				{
					*ptr = (instance_t) db::create_unresolved_pointer(db, path);
				}
			}
		};

		struct enum_db_entries_resolve : public db::enum_i
		{
			db::data *db;
			db::data *extra_resolve_db;
			std::string path;

			bool traverse_children;
			bool add_to_load;
			bool zero_unresolved;
			std::vector<std::string> to_load;
			int unresolved;

			enum_db_entries_resolve()
			{
				traverse_children = false;
				db = extra_resolve_db = 0;
				unresolved = 0;
				add_to_load = false;
				zero_unresolved = true;
			}

			struct process_ptr : public putki::depwalker_i
			{
				enum_db_entries_resolve *parent;

				bool pointer_pre(instance_t *on)
				{
					if (!*on) {
						return true;
					}

					// see if it is an unresolved pointer.
					const char *path = db::is_unresolved_pointer(parent->db, *on);
					if (!path) {
						return true;
					}
					
					type_handler_i *th;
					instance_t obj;
					
					if (db::fetch(parent->db, path, &th, &obj, false, true))
					{
						*on = obj;
					}
					else if (parent->extra_resolve_db && db::fetch(parent->extra_resolve_db, path, &th, &obj, false, true))
					{
						*on = obj;
					}
					else
					{
						if (parent->add_to_load)
						{
							parent->to_load.push_back(path);
							return false;
						}

						APP_WARNING("Unresolved reference to [" << path << "] when loading [" << parent->path << "]")
						parent->unresolved++;
						
						if (parent->zero_unresolved)
						{
							*on = 0;
						}
						return false;
					}
					
					return true;
				}

				void pointer_post(instance_t *on)
				{

				}
			};

			void record(const char *path, type_handler_i *th, instance_t i)
			{
				// only go for the pointers directly stored here.
				process_ptr p;
				p.parent = this;
				th->walk_dependencies(i, &p, traverse_children);
			}
		};
	}

	// adds unresolved pointer to the db through resolve_pointer.
	void load_into_db(db::data *db, const char *fullpath, const char *name, deferred_loader *loader = 0)
	{
		//
		std::string asset_name(name);
		int p = asset_name.find_last_of('.');
		if (p == std::string::npos) {
			return;
		}

		std::string ending = asset_name.substr(p, asset_name.size() - p);
		if (ending != ".json") {
			return;
		}

		asset_name = asset_name.substr(0, p);
		
		if (loader)
		{
			load_file_deferred(loader, db, asset_name.c_str());
			return;
		}
		
		parse::data *pd = parse::parse(fullpath);
		if (pd)
		{

			parse::node *root = parse::get_root(pd);
			std::string objtype = parse::get_value_string(parse::get_object_item(root, "type"));
			instance_t rootobj;

			type_handler_i *h = typereg_get_handler(objtype.c_str());
			if (h)
			{
				rootobj = h->alloc();
				load_resolver_store_db_ref d;
				d.objpath = asset_name;
				d.db = db;
				h->fill_from_parsed(parse::get_object_item(root, "data"), rootobj, &d);
				db::insert(db, asset_name.c_str(), h, rootobj);
			}
			else
			{
				APP_WARNING("Unrecognized type [" << objtype << "]")
				return;
			}

			// go grab all the auxs
			parse::node *aux = parse::get_object_item(root, "aux");
			if (aux) {
				for (int i=0;; i++)
				{
					parse::node *aux_obj = parse::get_array_item(aux, i);
					if (!aux_obj) {
						break;
					}

					std::string objtype = parse::get_value_string(parse::get_object_item(aux_obj, "type"));
					std::string refpath = asset_name + parse::get_value_string(parse::get_object_item(aux_obj, "ref"));

					type_handler_i *h = typereg_get_handler(objtype.c_str());
					if (h)
					{
						instance_t obj = h->alloc();

						load_resolver_store_db_ref d;
						d.objpath = asset_name;
						d.db = db;
						h->fill_from_parsed(parse::get_object_item(aux_obj, "data"), obj, &d);

						db::start_loading(db, refpath.c_str());
						db::insert(db, refpath.c_str(), h, obj);
					}
				}
			}
			putki::parse::free(pd);
			
		}
		else
		{
			APP_WARNING("Failed to load into db <" << fullpath << "> <" << name << ">")
		}
	}
	
	struct deferred_loader
	{
		std::string sourcepath;
		int refcount;
		sys::mutex resolve_mtx;
		std::map<std::string, db::data*> resolve_db;
	};
	
	deferred_loader *create_loader(const char *sourcepath)
	{
		deferred_loader *n = new deferred_loader();
		n->sourcepath = sourcepath;
		n->refcount = 1;
		return n;
	}

	void loader_incref(deferred_loader *loader)
	{
		loader->refcount++;
	}
	
	void loader_decref(deferred_loader *loader)
	{
		if (!--loader->refcount)
		{
			APP_DEBUG("Destroying loader with [" << loader->sourcepath << "]")
			delete loader;
		}
	}
	
	bool do_deferred_load(db::data *db, const char *path, type_handler_i **th, instance_t *obj, void *userptr)
	{
		deferred_loader *loader = (deferred_loader *)userptr;
		
		// 1. Load the json file raw into the database. 
		APP_DEBUG("deferred: loading " << path << " from [" << loader->sourcepath << "]")
		
		std::string fpath = std::string(path) + ".json";
		std::string fullpath = std::string(loader->sourcepath) + "/" + fpath;
		
		if (!db::start_loading(db, path))
		{
			if (!db::fetch(db, path, th, obj, false))
			{
				APP_ERROR("Race load fail!")
			}
			APP_DEBUG("Raced to " << path);
			return true;
		}

		{
			sys::scoped_maybe_lock lk0(&loader->resolve_mtx);
			load_into_db(db, fullpath.c_str(), fpath.c_str());
		}

		//  Now the database object will contain only unresolved pointers, so attempt to resolve it with the 
		//  target database itself.
		enum_db_entries_resolve resolver;
		resolver.add_to_load = true;
		resolver.db = db;
	
		{
			sys::scoped_maybe_lock lk0(&loader->resolve_mtx);
			resolver.extra_resolve_db = loader->resolve_db[path];
		}
		
		resolver.traverse_children = true;
		
		if (!db::fetch(db, path, th, obj, false, true))
		{
			APP_ERROR("Failed to load object " << path)
			*th = 0;
			*obj = 0;
			return false;
		}

		std::set<std::string> loaded;
		std::set<std::string> i_loaded;

		while (true)
		{
			resolver.path = path;
			resolver.to_load.clear();
			resolver.unresolved = 0;
			
			{
				sys::scoped_maybe_lock lk0(&loader->resolve_mtx);
				resolver.record(path, *th, *obj);
			}

			unsigned int ld = 0;
			for (unsigned int i=0; i<resolver.to_load.size(); i++)
			{
				std::string file = resolver.to_load[i] + ".json";
				if (loaded.count(resolver.to_load[i]) == 0)
				{
					loaded.insert(resolver.to_load[i]);
					ld++;
					const char *dep = resolver.to_load[i].c_str();
					if (db::start_loading(db, dep))
					{
						type_handler_i *xth;
						instance_t xobj;

						if (db::fetch(db, dep, &xth, &xobj, false, true))
						{
							APP_ERROR("Dependency " << path << " -> " << dep << " raced! start_loading returned true, but object exists in db")
							db::done_loading(db, dep);
							continue;
						}
					
						{
							sys::scoped_maybe_lock lk0(&loader->resolve_mtx);	
							load_into_db(db, (std::string(loader->sourcepath) + "/" + file).c_str(), file.c_str());
						}

						if (!db::fetch(db, resolver.to_load[i].c_str(), &xth, &xobj, false, true))
						{
							APP_WARNING("Dependency " << path << " -> " << dep << " FAILED!")
							db::done_loading(db, dep);
						}
						else
						{
							APP_DEBUG("Dependency " << path << " -> " << dep << " loaded")
							i_loaded.insert(resolver.to_load[i]);
						}
					}
					else
					{
						APP_DEBUG("Dependency " << path << " -> " << dep << " inserted by other")
					}
				}
			}
			
			if (!ld)
			{
				break;
			}
		}

		// everything that could be loaded is loaded, do a final pass which will clear
		// out any invalid pointers.
		
		resolver.add_to_load = false;
		resolver.unresolved = 0;
		{
			sys::scoped_maybe_lock lk0(&loader->resolve_mtx);
			resolver.record(path, *th, *obj);
		}
				
		std::set<std::string>::iterator i = i_loaded.begin();
		while (i != i_loaded.end())
		{
			db::done_loading(db, i->c_str());
			i++;
		}
		
		db::done_loading(db, path);
		return true;
	}
	
	void load_file_deferred(deferred_loader *loader, db::data *target, const char *path, db::data *resolve)
	{
		loader->resolve_mtx.lock();
		loader->resolve_db[path] = resolve;
		loader->resolve_mtx.unlock();
		
		db::insert_deferred(target, path, &do_deferred_load, loader);
	}
	
	namespace
	{
		struct add
		{
			db::data *db;
			deferred_loader *loader;
			int count;	
		};
		
		void add_file(const char *fullpath, const char *name, void *userptr)
		{
			add *a = (add*) userptr;
			a->count++;
			load_into_db(a->db, fullpath, name, a->loader);
		}
	}

	void on_destroy_db(void *p)
	{
		deferred_loader *l = (deferred_loader *)p;
		loader_decref(l);
	}

	void load_tree_into_db(const char *sourcepath, db::data *d)
	{
		add *a = new add();
		a->db = d;
		a->loader = create_loader(sourcepath);
		a->count = 0;
		putki::sys::search_tree(sourcepath, add_file, a);
		db::register_on_destroy(d, on_destroy_db, a->loader);
		APP_INFO("Inserted " << a->count << " objects with deferred loads")
		delete a;
	}

	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve, db::data *resolve_db)
	{
		std::string fpath = std::string(path) + ".json";
		std::string fullpath = std::string(sourcepath) + "/" + fpath;
		std::set<std::string> loaded;
		
		db::start_loading(d, path);
		load_into_db(d, fullpath.c_str(), fpath.c_str());
		db::done_loading(d, path);
		
		while (true)
		{
			// resolve
			enum_db_entries_resolve resolver;
			resolver.add_to_load = resolve;
			resolver.db = d;
			resolver.path = path;
			resolver.extra_resolve_db = resolve_db;
			resolver.zero_unresolved = resolve;
			
			type_handler_i *th;
			instance_t obj;
			if (!db::fetch(d, path, &th, &obj))
			{
				APP_WARNING("Failed to load " << fullpath)
				return;
			}
			
			resolver.record(path, th, obj);
			
			unsigned int ld = 0;
			for (unsigned int i=0; i<resolver.to_load.size(); i++)
			{
				std::string file = resolver.to_load[i] + ".json";
				if (loaded.count(file) == 0)
				{
					ld++;
					//
					// std::cout << "Loading additional [" << file << "] into db " << d << std::endl;
					//
					
					db::start_loading(d, path);
					load_into_db(d, (std::string(sourcepath) + "/" + file).c_str(), file.c_str());
					loaded.insert(file);
					db::done_loading(d, path);
				}
			}

			if (!ld) {
				break;
			}
		}

	}

}
