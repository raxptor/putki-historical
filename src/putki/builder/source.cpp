#include "source.h"

#include <iostream>
#include <string>

#include <putki/sys/files.h>

#include <putki/builder/parse.h>
#include <putki/builder/typereg.h>
#include <putki/builder/db.h>

namespace putki
{
	namespace
	{
		db::data *_db;

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

			std::cout << "[Adding source [" << fullname << "] as [" << asset_name << "]" << std::endl;

			parse::data *pd = parse::parse(fullname);
			if (pd)
			{
				parse::node *root = parse::get_root(pd);
				std::string objtype = parse::get_value_string(parse::get_object_item(root, "type"));
				
				i_type_handler *h = typereg_get_handler(objtype.c_str());
				if (h)
				{
					instance_t obj = h->alloc();
					h->fill_from_parsed(parse::get_object_item(root, "data"), obj);

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
	}
}
