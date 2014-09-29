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
#include <putki/builder/tool.h>
#include <putki/builder/build.h>

namespace putki
{
	struct source_db_def
	{
		std::string sourcepath;
		db::data *db;
	};

	struct deferred_loader
	{
		std::string sourcepath;
		int refcount;

		unsigned int source_db_count;
		source_db_def source_db[4];
	};

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
	}
	

	bool load_parsed_into_db(db::data *db, const char *path, parse::data *pd, deferred_loader *loader = 0, sys::mutex *insert_mtx = 0, bool recycle = false)
	{
		sys::mutex *lk_mtx = 0;
	
		parse::node *root = parse::get_root(pd);
		std::string objtype = parse::get_value_string(parse::get_object_item(root, "type"));
		instance_t rootobj = 0;

		type_handler_i *h = typereg_get_handler(objtype.c_str());
		if (h)
		{
			type_handler_i* old_th;
			if (recycle && db::fetch(db, path, &old_th, &rootobj))
			{
				if (old_th->id() != h->id())
				{
					APP_WARNING("Replacing object with different type..?")
					return false;
				}
			}
	
			if (!rootobj)
			{
				if (recycle)
				{
					APP_DEBUG("recycle flag is set, but could not find old object " << path)
				}
				
				rootobj = h->alloc();
			}
				
			load_resolver_store_db_ref d;
			d.objpath = path;
			d.db = db;
			h->fill_from_parsed(parse::get_object_item(root, "data"), rootobj, &d);
			
			if (insert_mtx)
			{
				insert_mtx->lock();
				lk_mtx = insert_mtx;
			}
				
			db::insert(db, path, h, rootobj);
		}
		else
		{
			APP_WARNING("Unrecognized type [" << objtype << "]")
			return false;
		}

		// go grab all the auxs
		parse::node *aux = parse::get_object_item(root, "aux");
		if (aux)
		{
			for (int i=0;; i++)
			{
				parse::node *aux_obj = parse::get_array_item(aux, i);
				if (!aux_obj)
				{
					break;
				}

				std::string objtype = parse::get_value_string(parse::get_object_item(aux_obj, "type"));
				std::string refpath = std::string(path) + parse::get_value_string(parse::get_object_item(aux_obj, "ref"));

				type_handler_i *h = typereg_get_handler(objtype.c_str());

				if (h)
				{
					instance_t obj = h->alloc();

					load_resolver_store_db_ref d;
					d.objpath = path;
					d.db = db;
					h->fill_from_parsed(parse::get_object_item(aux_obj, "data"), obj, &d);

					db::start_loading(db, refpath.c_str());
					db::insert(db, refpath.c_str(), h, obj);
				}
			}
		}
		
		if (lk_mtx)
		{
			lk_mtx->unlock();
		}
		
		return true;
	}


	// adds unresolved pointer to the db through resolve_pointer.
	bool load_json_into_db(db::data *db, const char *fullpath, const char *name, deferred_loader *loader = 0, sys::mutex *insert_mtx = 0)
	{
		//
		std::string asset_name(name);
		int p = asset_name.find_last_of('.');
		if (p == std::string::npos) {
			return false;
		}

		std::string ending = asset_name.substr(p, asset_name.size() - p);
		if (ending != ".json") {
			return false;
		}

		asset_name = asset_name.substr(0, p);
		
		if (loader)
		{
			load_file_deferred(loader, db, asset_name.c_str());
			return true;
		}
		
		parse::data *pd = parse::parse(fullpath);
		
		if (!pd)
		{
			APP_WARNING("Failed to parse into db <" << fullpath << "> <" << name << ">")
			return false;
		}
		
		bool success = load_parsed_into_db(db, asset_name.c_str(), pd, loader, insert_mtx, false);
		putki::parse::free(pd);
		return success;
	}
	
	
	bool update_with_json(db::data *db, const char *path, char *json, int size)
	{
		parse::data *pd = parse::parse_json(json, size);
		if (!pd)
		{
			return false;
		}
		
		const bool succ = load_parsed_into_db(db, path, pd, 0, 0, true);
		putki::parse::free(pd);

		APP_DEBUG("Json update on " << path << " success=" << succ);
		
		if (succ)
		{
			APP_DEBUG("Resolving object.")
			build::resolve_object(db, path);
		}
		
		return succ;
	}
	

	namespace
	{
		typedef std::map<std::string, source_db_def* > DepsToLoad;
		typedef std::set<std::string> PathSet;

		struct object_resolver : public putki::depwalker_i
		{
			deferred_loader *loader;
			db::data *db;
			DepsToLoad deps_required;
			unsigned int unresolved_count;

			bool pointer_pre(instance_t *on, const char *ptr_type)
			{
				if (!*on)
				{
					return true;
				}

				const char *path = db::is_unresolved_pointer(db, *on);
				if (!path)
				{
					return true;
				}

				// Now look through the loader's resolve chain. We are only going to
				// be pointing to input objects here, so resolve according to the input
				// chain.
				for (unsigned int i=0;i!=loader->source_db_count;i++)
				{
					if (db::exists(loader->source_db[i].db, path, true))
					{
						// Do not allow triggering of load yet, do it later.
						type_handler_i *th;
						if (!db::fetch(loader->source_db[i].db, path, &th, on, false, true))
						{
							// no duplicates
							deps_required.insert(std::make_pair(path, &loader->source_db[i]));
							unresolved_count++;
							return false;
						}
						else
						{
							if (!is_valid_pointer(th, ptr_type))
							{
								APP_WARNING("Have pointer to " << path << ", which is of type " << th->name() << " but expected object compatible with " << ptr_type << "! Zeroing pointer.")
								*on = 0;
							}

							// can convert?
							return true;
						}
					}
				}

				unresolved_count++;
				APP_WARNING("Permanent resolve failure on path [" << path << "]")
				*on = 0;
				
				return false;
			}

			void pointer_post(instance_t *on)
			{

			}
		};
	}

	sys::mutex resolve_mtx;

	deferred_loader *create_loader(const char *sourcepath)
	{
		deferred_loader *n = new deferred_loader();
		n->sourcepath = sourcepath;
		n->refcount = 1;
		n->source_db_count = 0;
		return n;
	}

	void loader_add_resolve_src(deferred_loader *loader, db::data *resolve, const char *sourcepath)
	{
		loader->source_db[loader->source_db_count].db = resolve;
		loader->source_db[loader->source_db_count].sourcepath = sourcepath;
		loader->source_db_count++;
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

	// The end-all, be all
	bool do_load_into_db(db::data *db, const char *path, type_handler_i **th, instance_t *obj, deferred_loader *loader, bool do_resolve)
	{
		// 1. Load the json file raw into the database.
		APP_DEBUG("deferred_loader: loading " << loader->sourcepath << "/" << path << ".json")
		
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

		load_json_into_db(db, fullpath.c_str(), fpath.c_str(), 0, &resolve_mtx);

		if (!db::fetch(db, path, th, obj, false, true))
		{
			APP_WARNING("Failed to load object " << path << ", it will be nothing")
			*th = 0;
			*obj = 0;
			
			db::done_loading(db, path);
			return false;
		}

		{
			sys::scoped_maybe_lock lk0(&resolve_mtx);
			resolve_object_aux_pointers(db, path);
		}

		// End of the line for when not resolving object external pointers
		if (!do_resolve)
		{
			db::done_loading(db, path);
			return true;
		}

		// Resolve and load additional dependencies
		object_resolver resolver;
		resolver.db = db;
		resolver.loader = loader;

		PathSet loaded;
		DepsToLoad loaded_here;

		while (true)
		{
			resolver.unresolved_count = 0;

			{
				sys::scoped_maybe_lock lk0(&resolve_mtx);
				resolver.reset_visited();
				(*th)->walk_dependencies(*obj, &resolver, true);
			}

			if (!resolver.unresolved_count)
			{
				APP_DEBUG("Resolving of object [" << path << "] completed")
				break;
			}

			int new_loads = 0;

			for (DepsToLoad::const_iterator i=resolver.deps_required.begin();i!=resolver.deps_required.end();i++)
			{
				// Don't do it again
				if (loaded.count(i->first))
					continue;

				loaded.insert(i->first);
				new_loads++;

				if (!db::start_loading(i->second->db, i->first.c_str()))
				{
					APP_DEBUG("Lost the race for loading " << i->first << " " << i->second->sourcepath)
					continue;
				}

				APP_DEBUG("Depload " << path << " => " << i->first << " from disk at " << i->second->sourcepath)
				if (!load_json_into_db(i->second->db, (i->second->sourcepath + "/" + i->first + ".json").c_str(), (i->first + ".json").c_str(), 0, &resolve_mtx))
				{
					APP_WARNING("Dependency " << path << " -> " << i->first << " FAILED!")
					db::done_loading(i->second->db, i->first.c_str());
				}
				else
				{
					APP_DEBUG("Dependency " << path << " -> " << i->first << " loaded " << i->second->sourcepath)
					loaded_here.insert(std::make_pair(i->first, i->second));
				}
			}

			if (!new_loads)
			{
				break;
			}
		}

		// everything that could be loaded is loaded, do a final pass which will clear
		// out any invalid pointers.
		if (resolver.unresolved_count != 0)
		{
			sys::scoped_maybe_lock lk0(&resolve_mtx);
			clear_unresolved_pointers(db, *th, *obj);
		}

		DepsToLoad::iterator i = loaded_here.begin();
		while (i != loaded_here.end())
		{
			type_handler_i *_th;
			instance_t _obj;

			if (!db::fetch(i->second->db, i->first.c_str(), &_th, &_obj, false, true))
				APP_ERROR("loaded_here says it was loaded, but apparently not!")

			if (resolver.unresolved_count != 0)
			{
				sys::scoped_maybe_lock lk0(&resolve_mtx);
				clear_unresolved_pointers(i->second->db, _th, _obj);
			}

			i++;
		}

		i = loaded_here.begin();
		while (i != loaded_here.end())
		{
			db::done_loading(i->second->db, i->first.c_str());
			i++;
		}

		verify_obj(db, 0, *th, *obj, REQUIRE_HAS_PATHS | REQUIRE_RESOLVED, true, true);

		db::done_loading(db, path);
		return true;
	}

	bool do_deferred_load(db::data *db, const char *path, type_handler_i **th, instance_t *obj, void *userptr)
	{
		return do_load_into_db(db, path, th, obj, (deferred_loader *) userptr, true);
	}

	void load_file_deferred(deferred_loader *loader, db::data *target, const char *path)
	{
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
			load_json_into_db(a->db, fullpath, name, a->loader);
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
		loader_add_resolve_src(a->loader, d, sourcepath);
		a->count = 0;
		putki::sys::search_tree(sourcepath, add_file, a);
		db::register_on_destroy(d, on_destroy_db, a->loader);
		APP_INFO("Inserted " << a->count << " objects with deferred loads")
		delete a;
	}

	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve)
	{
		type_handler_i *th;
		instance_t obj;

		// everything through the deferred loader now.
		deferred_loader *loader = create_loader(sourcepath);
		do_load_into_db(d, path, &th, &obj, loader, resolve);
		loader_decref(loader);
	}

}
