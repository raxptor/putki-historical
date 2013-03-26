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
		db::data *_db;

		struct resolve_entry
		{
			instance_t *inplace_ptr;
			std::string path;
		};

		std::vector<resolve_entry> toresolve;

		struct delayed_resolve : public i_load_resolver
		{
			void resolve_pointer(instance_t *ptr, const char *path)
			{
				*ptr = (instance_t) 0x6633112;
				resolve_entry e;
				e.inplace_ptr = ptr;
				e.path = path;
				toresolve.push_back(e);
			}
		};
	

		void add_file(const char *fullname, const char *name)
		{
			std::string asset_name(name);
			int p = asset_name.find_last_of('.');
			if (p == std::string::npos)
				return;

			std::string ending = asset_name.substr(p, asset_name.size() - p);
			if (ending != ".json")
				return;

			asset_name = asset_name.substr(0, p);

			parse::data *pd = parse::parse(fullname);
			if (pd)
			{
				parse::node *root = parse::get_root(pd);
				std::string objtype = parse::get_value_string(parse::get_object_item(root, "type"));
				
				i_type_handler *h = typereg_get_handler(objtype.c_str());
				if (h)
				{
					instance_t obj = h->alloc();

					delayed_resolve d;
					h->fill_from_parsed(parse::get_object_item(root, "data"), obj, &d);

					db::insert(_db, asset_name.c_str(), h, obj);
				}
				else
				{
					std::cout << " => Unrecognized type [" << objtype << "]" << std::endl;
				}

				putki::parse::free(pd);
			}
		}
	}

	void init_db_with_source(const char *sourcepath, db::data *d)
	{
		_db = d;
		putki::sys::search_tree(sourcepath, add_file);
		
		std::cout << "Loaded " << db::size(_db) << " records. " << toresolve.size() << " pointers to resolve!" << std::endl;

		std::vector<resolve_entry> next_pass = toresolve;
		for (unsigned int i=0;i!=next_pass.size();i++)
		{
			i_type_handler *th;
			instance_t obj;
			if (db::fetch(_db, next_pass[i].path.c_str(), &th, &obj))
			{
				// todo: check types here.
				*(next_pass[i].inplace_ptr) = obj;
			}
			else
			{
				std::cout << "Unable to resolve [" << next_pass[i].path << "]!" << std::endl;
			}
		}
	}
}
