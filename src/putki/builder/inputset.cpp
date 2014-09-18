#include "inputset.h"

#include <putki/sys/files.h>
#include <putki/builder/source.h>
#include <putki/builder/db.h>
#include <putki/builder/write.h>

#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <map>

namespace putki
{
	namespace inputset
	{
		struct o_record
		{
			std::string path;
			std::string objname;
			sys::file_info info;
			std::string content_sig;
			bool exists;
		};

		typedef std::map<std::string, o_record> ObjMap;

		struct data
		{
			std::string objpath;
			std::string respath;
			std::string dbfile;

			ObjMap objs;
		};

		void obj_file(const char *fullname, const char *name, void *userptr)
		{
			data *d = (data *)userptr;

			// parse
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
			ObjMap::iterator i = d->objs.find(asset_name);
			if (i == d->objs.end())
			{
				std::cout << "added [" << asset_name << "]" << std::endl;
				o_record tmp;
				tmp.path = name;
				tmp.info.size = 0;
				tmp.info.mtime = 0;
				d->objs.insert(std::make_pair(asset_name, tmp));
				i = d->objs.find(asset_name);
			}

			o_record & record = i->second;

			sys::file_info info;
			if (!sys::stat(fullname, &info))
			{
				std::cerr << "could not stat [" << fullname << "]" << std::endl;
				info.size = record.info.size;
				info.mtime = record.info.mtime;
			}

			std::string sig = record.content_sig;

			if (record.content_sig.empty() || info.size != record.info.size || info.mtime != record.info.mtime)
			{
				if (!record.content_sig.empty())
					std::cout << "=> Parsing file " << fullname << " for changes" << std::endl;

				db::data *tmp = db::create();
				load_file_into_db(d->objpath.c_str(), i->first.c_str(), tmp, false);
			
				type_handler_i *th;
				instance_t obj;
				if (db::fetch(tmp, i->first.c_str(), &th, &obj))
				{
					sig = db::signature(tmp, i->first.c_str());
					record.info.size = info.size;
					record.info.mtime = info.mtime;
				}
				else
				{
					sig = "<broken>";
				}
				db::free_and_destroy_objs(tmp);
			}

			if (sig != record.content_sig && !record.content_sig.empty())
			{
				std::cout << "=> New signature on object [" << sig << "], old sig = [" << record.content_sig << "]" << std::endl;
				record.content_sig = sig;
			}

			record.exists = true;
		}

		void load_directory(data *d)
		{
			std::ifstream f(d->dbfile.c_str());
			while (f.good() && !f.eof())
			{
				std::string line;
				std::getline(f, line);
				if (f.eof())
					break;
				if (line.empty())
					continue;

				int spl[64], spls=0;
				for (int i=0;i<line.size();i++)
					if (line[i] == ':')
						spl[spls++] = i;

				spl[spls++] = line.size();
		
				if (spls >= 4)
				{
					if (line[0] == 'i')
					{
						o_record tmp;
						tmp.info.size = atoi(line.substr(spl[1]+1, spl[2]-spl[1]-1).c_str());
						tmp.info.mtime = atoi(line.substr(spl[2]+1, spl[3]-spl[2]-1).c_str());
						tmp.content_sig = line.substr(spl[3]+1, spl[4]-spl[3]-1);
						tmp.exists = false;
						d->objs.insert(std::make_pair(line.substr(spl[0]+1, spl[1]-spl[0]-1), tmp));
					}
				}
			}
		}

		void write_directory(data *d)
		{
			std::ofstream f(d->dbfile.c_str());
			ObjMap::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				f << "i:" << i->first << ":" << i->second.info.size << ":" << i->second.info.mtime << ":" << i->second.content_sig << "\n";
				++i;
			}
			f.close();
		}
		
		data *open(const char *objpath, const char *respath, const char *dbfile)
		{
			data *d = new data();
			d->respath = respath;
			d->objpath = objpath;
			d->dbfile = dbfile;

			std::cout << "Input set [" << objpath << "]/[" << respath << "] tracked in [" << dbfile << "]" << std::endl;

			load_directory(d);

			//
			sys::search_tree(objpath, obj_file, d);
			ObjMap::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				if (!i->second.exists)
				{
					std::cout << "=> [" << i->first << "] has been removed" << std::endl;
					i = d->objs.erase(i);
					continue;
				}
				++i;
			}

			write_directory(d);
			return d;
		}

		void release(data *d)
		{
			delete d;
		}

		const char *get_object_sig(data *d, const char *path)
		{
			ObjMap::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				return i->second.content_sig.c_str();
			}
			else
			{
				std::cout << "I HAVE NO RECORD OF SIGNATURE FOR [" << path << "] !" << std::endl;
			}

			return 0;
		}
	}
}
