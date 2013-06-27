#include "db.h"

#include <map>
#include <string>
#include <iostream>
#include <set>
#include <vector>

#include <cstdlib>
#include <cstdio>

#include <putki/sys/compat.h>

namespace putki
{

	namespace db
	{
		struct entry
		{
			type_handler_i *th;
			instance_t obj;
			std::vector<std::string> auxrefs;
		};

		struct data
		{
			std::map<std::string, entry> objs;
			std::map<instance_t, std::string> paths;
			std::set<const char *> unresolved;
			std::map<std::string, const char *> strpool;

			char auxpathbuf[256];
		};

		db::data * create()
		{
			return new data();
		}

		void free(data *d)
		{
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

		bool is_aux_path(const char *path)
		{
			return strchr(path, '#') != 0;
		}
		
		const char *pathof(data *d, instance_t obj)
		{
			std::map<instance_t, std::string>::iterator i = d->paths.find(obj);
			if (i != d->paths.end())
				return i->second.c_str();
			return 0;
		}

		const char *pathof_including_unresolved(data *d, instance_t obj)
		{
			const char *unres = is_unresolved_pointer(d, obj);
			if (unres)
				return unres;
			return pathof(d, obj);
		}
		
		void insert(data *d, const char *path, type_handler_i *th, instance_t i)
		{
			std::cout << " db insert on path [" << path << "]" << std::endl;
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
				if (i != d->objs.end())
				{
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

		instance_t ptr_to_allow_unresolved(data *d, const char *path)
		{
			std::map<std::string, entry>::iterator i = d->objs.find(path);
			if (i != d->objs.end())
				return i->second.obj;
			return create_unresolved_pointer(d, path);
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

		instance_t create_unresolved_pointer(data *d, const char *path)
		{
			std::map<std::string, const char *>::iterator i = d->strpool.find(path);
			if (i != d->strpool.end())
			{
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
				return (const char*) p;
			else
				return 0;
		}

		
	}
}

