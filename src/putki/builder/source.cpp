#include "source.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include <putki/sys/files.h>

#include <putki/builder/parse.h>
#include <putki/builder/typereg.h>
#include <putki/builder/db.h>

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
			instance_t obj;

			bool traverse_children;
			bool add_to_load;
			std::vector<std::string> to_load;
			std::vector<std::string> auxes_touched;
			int unresolved;

			enum_db_entries_resolve()
			{
				traverse_children = false;
				db = extra_resolve_db = 0;
				unresolved = 0;
				add_to_load = false;
				obj = 0;
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

					if (db::fetch(parent->db, path, &th, &obj))
					{
						*on = obj;
					}
					else if (parent->extra_resolve_db && db::fetch(parent->extra_resolve_db, path, &th, &obj))
					{
						*on = obj;
					}
					else
					{
						if (parent->add_to_load)
						{
							parent->to_load.push_back(path);
							return true;
						}

						std::cout << "Unresolved reference to [" << path << "]!" << std::endl;
						parent->unresolved++;
						*on = 0;
					}
					
					if (obj && parent->obj && db::is_aux_path_of(parent->db, parent->obj, path))
					{
						parent->auxes_touched.push_back(path);
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

			type_handler_i *h = typereg_get_handler(objtype.c_str());
			if (h)
			{
				instance_t obj = h->alloc();

				load_resolver_store_db_ref d;
				d.objpath = asset_name;
				d.db = db;
				h->fill_from_parsed(parse::get_object_item(root, "data"), obj, &d);

				db::insert(db, asset_name.c_str(), h, obj);
			}
			else
			{
				std::cout << " => Unrecognized type [" << objtype << "]" << std::endl;
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

						db::insert(db, refpath.c_str(), h, obj);
					}
				}
			}
			putki::parse::free(pd);
		}
		else
		{
			std::cout << "[failed to load into db <" << fullpath << "> <" << name << ">" << std::endl;
		}
	}
	
	struct deferred_loader
	{
		std::string sourcepath;
		int refcount;
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
		if (!loader->refcount--)
		{
			delete loader;
		}
	}
	
	bool do_deferred_load(db::data *db, const char *path, type_handler_i **th, instance_t *obj, void *userptr)
	{
		deferred_loader *loader = (deferred_loader *)userptr;
		
		// 1. Load the json file raw into the database. 
		
		std::string fpath = std::string(path) + ".json";
		std::string fullpath = std::string(loader->sourcepath) + "/" + fpath;
		load_into_db(db, fullpath.c_str(), fpath.c_str());
		
		//  Now the database object will contain only unresolved pointers, so attempt to resolve it with the 
		//  target database itself.
		enum_db_entries_resolve resolver;
		resolver.add_to_load = true;
		resolver.db = db;
		resolver.extra_resolve_db = 0;
		resolver.traverse_children = true;
		
		if (!db::fetch(db, path, th, obj))
		{
			std::cerr << "*** do_deferred_load could not fetch itself!" << std::endl;
			return false;
		}
		
		// resolve pointers - what 
		resolver.obj = *obj; // to get the auxes
		resolver.record(path, *th, *obj);
		if (resolver.unresolved)
		{
			std::cerr << "*** there are unresolved pointers after the deferred load!" << std::endl;
			return false;
		}
		
		return true;
	}
	
	void load_file_deferred(deferred_loader *loader, db::data *target, const char *path)
	{
		db::insert_deferred(target, path, &do_deferred_load, loader);
	}
	
	namespace
	{
		db::data *_db;
		deferred_loader *_loader;
		unsigned int _count = 0;
		void add_file(const char *fullpath, const char *name, void *userptr)
		{
			_count++;
			load_into_db(_db, fullpath, name, _loader);
		}
	}

	void load_tree_into_db(const char *sourcepath, db::data *d)
	{
		_db = d;	
		_loader = create_loader(sourcepath);
		_count++;
		putki::sys::search_tree(sourcepath, add_file, 0);
		std::cout << "Inserted " << _count << " objects with deferred loads" << std::endl;
	}

	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve, db::data *resolve_db)
	{
		std::string fpath = std::string(path) + ".json";
		std::string fullpath = std::string(sourcepath) + "/" + fpath;
		load_into_db(d, fullpath.c_str(), fpath.c_str());

		std::set<std::string> loaded;

		while (true)
		{
			// resolve
			enum_db_entries_resolve resolver;
			resolver.add_to_load = true;
			resolver.db = d;
			resolver.extra_resolve_db = resolve_db;
			db::read_all(d, &resolver);

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
					load_into_db(d, (std::string(sourcepath) + "/" + file).c_str(), file.c_str());
					loaded.insert(file);
				}
			}

			if (!ld) {
				break;
			}
		}

	}

}
