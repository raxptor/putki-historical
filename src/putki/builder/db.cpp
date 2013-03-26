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
			instance_t obj;
		};

		struct data
		{
			std::map<std::string, entry> objs;
		};

		db::data * create()
		{
			return new data();
		}

		void insert(data *d, const char *path, i_type_handler *th, instance_t i)	
		{
			entry e;
			e.th = th;
			e.obj = i;
			d->objs[path] = e;
		}

		bool fetch(data *d, const char *path, i_type_handler **th, instance_t *obj)
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

