#include "inputset.h"
#include "tok.h"

#include <putki/sys/files.h>
#include <putki/builder/source.h>
#include <putki/builder/db.h>
#include <putki/builder/write.h>
#include <putki/builder/log.h>
#include <putki/sys/thread.h>


#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <map>

extern "C" {
	#include <md5/md5.h>
}

namespace putki
{
	namespace inputset
	{
		struct o_record
		{
			std::string path;
			std::string type;
			std::string objname;
			sys::file_info info;
			std::string content_sig;
			bool exists;
		};
		
		struct r_record
		{
			std::string path;
			sys::file_info info;
			std::string content_sig;
			bool exists;
		};

		typedef std::map<std::string, o_record> ObjMap;
		typedef std::map<std::string, r_record> ResMap;

		struct data
		{
			std::string objpath;
			std::string respath;
			std::string dbfile;
			bool has_changes;
			ObjMap objs;
			ResMap res;
			sys::mutex mtx;
		};

		std::string obj_path(const char *base, const char *path)
		{
			return std::string(base) + "/" + path + ".json";
		}

		void obj_file(const char *fullname, const char *name, void *userptr)
		{
			data *d = (data *)userptr;

			// parse
			std::string asset_name(name);
			int p = asset_name.find_last_of('.');
			if (p == std::string::npos)
			{
				return;
			}

			std::string ending = asset_name.substr(p, asset_name.size() - p);
			if (ending != ".json")
			{
				return;
			}

			sys::scoped_maybe_lock lk(&d->mtx);

			asset_name = asset_name.substr(0, p);
			ObjMap::iterator i = d->objs.find(asset_name);
			if (i == d->objs.end())
			{
				APP_DEBUG("Added [" << asset_name << "]")
				o_record tmp;
				tmp.path = name;
				tmp.info.size = 0;
				tmp.info.mtime = 0;
				
				d->objs.insert(std::make_pair(asset_name, tmp));
				d->has_changes = true;
				
				i = d->objs.find(asset_name);
			}

			o_record & record = i->second;

			sys::file_info info;
			if (!sys::stat(fullname, &info))
			{
				APP_WARNING("Could not stat [" << fullname << "]")
				info.size = -1;
				info.mtime = -1;
			}

			std::string sig = record.content_sig;

			if (record.content_sig.empty() || info.size != record.info.size || info.mtime != record.info.mtime)
			{
				d->has_changes = true;
		
				if (!record.content_sig.empty())
				{
					APP_DEBUG("Parsing file " << fullname << " for changes")
					APP_DEBUG(record.content_sig << " " << info.size << ":" << record.info.size << " " << info.mtime << ":" << record.info.mtime)
				}

				db::data *tmp = db::create();
				load_file_into_db(d->objpath.c_str(), i->first.c_str(), tmp, false);

				type_handler_i *th;
				instance_t obj;
				if (db::fetch(tmp, i->first.c_str(), &th, &obj))
				{
					char buffer[128];
					sig = db::signature(tmp, i->first.c_str(), buffer);
					record.info.size = info.size;
					record.info.mtime = info.mtime;
					record.type = th->name();
				}
				else
				{
					sig = "<broken>";
					record.type = "-unloadable-";
				}
				db::free_and_destroy_objs(tmp);
			}

			if (sig != record.content_sig && !record.content_sig.empty())
			{
				APP_DEBUG("New signature on object [" << sig << "], old sig = [" << record.content_sig << "]")
				d->has_changes = true;
			}

			record.content_sig = sig;
			record.exists = true;
		}
		
		bool load_file(const char *path, long long *outSize, char **outBytes)
		{
			std::ifstream f(path, std::ios::binary);
			if (!f.good())
			{
				APP_WARNING("Failed to load " << path)
				return false;
			}

			f.seekg(0, std::ios::end);
			std::streampos size = f.tellg();
			f.seekg(0, std::ios::beg);

			char *b = new char[(size_t)size];
			f.read(b, size);
			*outBytes = b;
			*outSize = (long long) size;
			
			return true;
		}
		
		std::string signature_from_file(const char *path)
		{
			long long size;
			char *bytes;
			if (!load_file(path, &size, &bytes))
				return "missing";
				
			char signature[64];
			char signature_string[64];

			md5_buffer(bytes, (long)size, signature);
			md5_sig_to_string(signature, signature_string, 64);
			
			delete [] bytes;
			return signature_string;
		}

		void res_file(const char *fullname, const char *name, void *userptr)
		{
			data *d = (data *)userptr;
			
			if (name[0] == '.')
				return;

			sys::scoped_maybe_lock lk(&d->mtx);

			// parse
			ResMap::iterator i = d->res.find(name);
			if (i == d->res.end())
			{
				APP_DEBUG("Added [" << name << "]")
				r_record tmp;
				tmp.path = name;
				tmp.info.size = 0;
				tmp.info.mtime = 0;
				d->res.insert(std::make_pair(name, tmp));
				d->has_changes = true;
				i = d->res.find(name);
			}

			r_record & record = i->second;

			sys::file_info info;
			if (!sys::stat(fullname, &info))
			{
				APP_WARNING("Could not stat [" << fullname << "]")
				info.size = -1;
				info.mtime = -1;
			}

			std::string sig = record.content_sig;
			if (record.content_sig.empty() || info.size != record.info.size || info.mtime != record.info.mtime)
			{
				APP_DEBUG("Recomputing signature on " << fullname)
				d->has_changes = true;
				record.content_sig = signature_from_file(fullname);
				
				if (!record.content_sig.empty())
				{
					APP_DEBUG("Signature changed on " << fullname << " => " << record.content_sig << " " << info.size << ":" << record.info.size << " " << info.mtime << ":" << record.info.mtime)
				}
				
				record.info = info;
			}

			if (sig != record.content_sig && !sig.empty())
			{
				APP_DEBUG("New signature on object [" << record.content_sig << "], old sig = [" << sig << "]")
				d->has_changes = true;
			}

			record.exists = true;
		}

		void load_directory(data *d)
		{
			tok::data *tk = tok::load(d->dbfile.c_str());
			if (!tk) return;
			
			tok::tokenize_newlines(tk);
				
			int index = 0;
			while (true)
			{
				const char *ln = tok::get(tk, index++);
				if (!ln)
					break;
					
				std::string line(ln);
				
				int spl[64], spls = 0;
				for (int i = 0;i < line.size();i++)
				{
					if (line[i] == ':')
					{
						spl[spls++] = i;
					}
				}

				spl[spls++] = line.size();

				if (spls >= 4)
				{
					if (line[0] == 'i' && spls >= 5)
					{
						o_record tmp;
						tmp.info.size = atoi(line.substr(spl[1] + 1, spl[2] - spl[1] - 1).c_str());
						tmp.info.mtime = atoi(line.substr(spl[2] + 1, spl[3] - spl[2] - 1).c_str());
						tmp.content_sig = line.substr(spl[3] + 1, spl[4] - spl[3] - 1);
						tmp.type = line.substr(spl[4] + 1, spl[5] - spl[4] - 1);
						tmp.exists = false;
						d->objs.insert(std::make_pair(line.substr(spl[0] + 1, spl[1] - spl[0] - 1), tmp));
					}
					if (line[0] == 'r')
					{
						r_record tmp;
						tmp.info.size = atoi(line.substr(spl[1] + 1, spl[2] - spl[1] - 1).c_str());
						tmp.info.mtime = atoi(line.substr(spl[2] + 1, spl[3] - spl[2] - 1).c_str());
						tmp.content_sig = line.substr(spl[3] + 1, spl[4] - spl[3] - 1);
						tmp.exists = false;
						d->res.insert(std::make_pair(line.substr(spl[0] + 1, spl[1] - spl[0] - 1), tmp));
					}
				}
			}
			
			// cleanup
			if (tk) tok::free(tk);
		}

		void write(data *d)
		{
			APP_DEBUG("Writing input-db to [" << d->dbfile << "]")
			std::ofstream f(d->dbfile.c_str());
			ObjMap::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				f << "i:" << i->first << ":" << i->second.info.size << ":" << i->second.info.mtime << ":" << i->second.content_sig << ":" << i->second.type << "\n";
				++i;
			}
			ResMap::iterator j = d->res.begin();
			while (j != d->res.end())
			{
				f << "r:" << j->first << ":" << j->second.info.size << ":" << j->second.info.mtime << ":" << j->second.content_sig << "\n";
				++j;
			}
			f.close();
		}

		void force_obj(data *d, const char *path, const char *signature, const char *type)
		{
			sys::scoped_maybe_lock lk(&d->mtx);
			d->objs[path].content_sig = signature;
			d->objs[path].type = type;
			sys::stat(obj_path(d->objpath.c_str(), path).c_str(), &d->objs[path].info);
		}

		void touched_resource(data *d, const char *path)
		{
			std::string full_path = d->respath + "/" + (path+1);
			res_file(full_path.c_str(), (path+1), d);
		}

		data *open(const char *objpath, const char *respath, const char *dbfile)
		{
			data *d = new data();
			d->respath = respath;
			d->objpath = objpath;
			d->dbfile = dbfile;
			d->has_changes = false;

			APP_DEBUG("Input set [" << objpath << "]/[" << respath << "] tracked in [" << dbfile << "]")

			load_directory(d);

			//
			sys::search_tree(respath, res_file, d);
			sys::search_tree(objpath, obj_file, d);
			ObjMap::iterator i = d->objs.begin();
			while (i != d->objs.end())
			{
				if (!i->second.exists)
				{
					APP_INFO("Removed object [" << i->first << "]")
					d->objs.erase(i++);
					d->has_changes = true;
					continue;
				}
				++i;
			}

			ResMap::iterator j = d->res.begin();
			while (j != d->res.end())
			{
				if (!j->second.exists)
				{
					APP_INFO("Removed resource [" << j->first << "]")
					d->res.erase(j++);
					d->has_changes = true;
					continue;
				}
				++j;
			}

			sys::mk_dir_for_path(dbfile);

			if (d->has_changes)
			{
				write(d);
				d->has_changes = false;
			}

			return d;
		}

		void release(data *d)
		{
			delete d;
		}
		
		const char *get_object_type(data *d, const char *path)
		{
			sys::scoped_maybe_lock lk(&d->mtx);
			ObjMap::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				return i->second.type.c_str();
			}
			return 0;
		}

		bool get_object_sig(data *d, const char *path, char *buffer)
		{
			sys::scoped_maybe_lock lk(&d->mtx);
		
			ObjMap::iterator i = d->objs.find(path);
			if (i != d->objs.end())
			{
				strcpy(buffer, i->second.content_sig.c_str());
				return true;
			}
			return false;
		}
		
		bool get_res_sig(data *d, const char *path, char *buffer)
		{
			sys::scoped_maybe_lock lk(&d->mtx);
		
			ResMap::iterator i = d->res.find(path);
			if (i != d->res.end())
			{
				strcpy(buffer, i->second.content_sig.c_str());
				return true;
			}
			return false;
		}
	}
}
