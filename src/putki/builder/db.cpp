#include "db.h"

#include <map>
#include <string>
#include <iostream>

namespace putki
{

	namespace db
	{
		struct entry
		{
			type_handler_i *th;
			instance_t obj;
		};

		struct data
		{
			std::map<std::string, entry> objs;
			std::map<instance_t, std::string> paths;
		};

		db::data * create()
		{
			return new data();
		}

		const char *pathof(data *d, instance_t obj)
		{
			std::map<instance_t, std::string>::iterator i = d->paths.find(obj);
			if (i != d->paths.end())
				return i->second.c_str();
			return 0;
		}
		
		void insert(data *d, const char *path, type_handler_i *th, instance_t i)	
		{
			std::cout << " db insert on path [" << path << "]" << std::endl;
			entry e;
			e.th = th;
			e.obj = i;
			d->objs[path] = e;
			d->paths[i] = path;
		}

		bool fetch(data *d, const char *path, type_handler_i **th, instance_t *obj)
		{
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				*th = i->second.th;
				*obj = i->second.obj;
				return true;
			}
			return false;
		}

		void read_all(data *d, enum_i *eobj)
		{
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
		
	}
}

