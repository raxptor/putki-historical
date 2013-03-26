#include "db.h"

#include <map>
#include <string>

namespace putki
{

	namespace db
	{
		struct entry
		{
			i_type_handler *th;
			type_inst obj;
		};

		struct data
		{
			std::map<std::string, entry> objs;
		};

		db::data * create()
		{
			return new data();
		}

		void insert(data *d, const char *path, i_type_handler *th, type_inst i)	
		{
			entry e;
			e.th = th;
			e.obj = i;
			d->objs[path] = e;
		}

		bool fetch(data *d, const char *path, i_type_handler **th, type_inst *obj)
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

		void read_all(data *d, record_enum_t func)
		{
			std::map<std::string, entry>::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				func(i->first.c_str(), i->second.th, i->second.obj);
				++i;
			}
		}
		
	}
}

