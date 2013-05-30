#include "source.h"

#include <iostream>
#include <string>
#include <vector>

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

			void resolve_pointer(instance_t *ptr, const char *path)
			{
				*ptr = (instance_t) db::create_unresolved_pointer(db, path);
			}
		};	

		struct enum_db_entries_resolve : public db::enum_i
		{
			db::data *db;

			struct process_ptr : public putki::depwalker_i
			{
				enum_db_entries_resolve *parent;
				void pointer(instance_t *on)
				{
					// see if it is an unresolved pointer.
					const char *path = db::is_unresolved_pointer(parent->db, *on);
					
					type_handler_i *th;
					instance_t obj;
					if (db::fetch(parent->db, path, &th, &obj))
					{
						*on = obj;
					}
					else
					{
						std::cout << "Unresolved reference to [" << path << "]!" << std::endl;
						*on = 0;
					}
				}
			};

			void record(const char *path, type_handler_i *th, instance_t i)
			{
				// only go for the pointers directly stored here.
				process_ptr p;
				p.parent = this;
				th->walk_dependencies(i, &p, false);
			}
		};
	}

	// adds unresolved pointer to the db through resolve_pointer.
	void load_into_db(db::data *db, const char *fullpath, const char *name)
	{
		// 
		std::string asset_name(name);
		int p = asset_name.find_last_of('.');
		if (p == std::string::npos)
			return;

		std::string ending = asset_name.substr(p, asset_name.size() - p);
		if (ending != ".json")
			return;

		asset_name = asset_name.substr(0, p);

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
				d.db = db;
				h->fill_from_parsed(parse::get_object_item(root, "data"), obj, &d);

				db::insert(db, asset_name.c_str(), h, obj);
			}
			else
			{
				std::cout << " => Unrecognized type [" << objtype << "]" << std::endl;
			}

			putki::parse::free(pd);
		}
	}

	namespace
	{
		db::data *_db;
		void add_file(const char *fullpath, const char *name)
		{
			load_into_db(_db, fullpath, name);
		}
	}
	
	void load_tree_into_db(const char *sourcepath, db::data *d)
	{
		_db = d;
		putki::sys::search_tree(sourcepath, add_file);
		std::cout << "Loaded " << db::size(_db) << " records." << std::endl;

		// might have unresolved.

		enum_db_entries_resolve resolver;
		resolver.db = d;

		db::read_all(d, &resolver);

		// and hopefully here there are no unresolved pointers!
	}

	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve)
	{		
		std::cout << "Loading [" << path << "] from [" << sourcepath << "] into db." << std::endl;

		std::string fullpath = std::string(sourcepath) + "/" + path;
		load_into_db(d, fullpath.c_str(), path);
	}

}
