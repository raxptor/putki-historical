#include "dllinterface.h"
#include "dllinternal.h"

#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/write.h>
#include <putki/builder/db.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/log.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/sys/compat.h>
#include <putki/sys/files.h>
#include <putki/sys/sstream.h>
#include <putki/sys/thread.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <stdint.h>
#include <stdlib.h>

extern "C"
{
      #include <md5/md5.h>
}

namespace putki
{
	data_dll_i::~data_dll_i()
	{

	}
	
	void* connection_thread(void *arg)
	{
		liveupdate::ed_client *ed = (liveupdate::ed_client *) arg;
		liveupdate::run_editor_connection(ed);
		liveupdate::release_editor_connection(ed);
		return 0;
	}

	struct data_dll : public data_dll_i
	{
		db::data *_db;
		sys::mutex _db_mtx;
		std::string _path;
		std::string _status;
		
		liveupdate::ed_client *_client;
		
		data_dll(const char *path)
		{
			_path = path;
			
			// hax.
			_path.append("/data/objs");

			_status = "Objects at ";
			_status.append(_path);
			
			_db = db::create(0, &_db_mtx);
			_client = liveupdate::create_editor_connection();
			sys::thread_create(connection_thread, _client);
			
			set_loglevel(LOG_INFO);
		}

		~data_dll()
		{
			db::free(_db);
		}

		const char *get_status()
		{
			static char buf[1024];
			sys::scoped_maybe_lock lk(&_db_mtx);
			strcpy(buf, _status.c_str());
			return buf;
		}

		mem_instance* create_instance(const char *path, ext_type_handler_i *eth)
		{
			type_handler_i *th = putki::typereg_get_handler(eth->name());

			mem_instance_real *mi = new mem_instance_real();
			mi->is_struct_instance = false;
			mi->path = strdup(path);
			mi->th = th;
			mi->eth = eth;
			mi->inst = th->alloc();
			mi->refs_db = _db;
			return mi;
		}

		void free_instance(mem_instance *_mi)
		{
			mem_instance_real *mi = (mem_instance_real*) _mi;
			free(mi->path);
			mi->th->free(mi->inst);
			delete mi;
		}
		
		virtual void mem_build_asset(const char *path, ext_build_result *res)
		{
			APP_WARNING("mem_build_asset not implemented. remove this.")
		}

		mem_instance* disk_load(const char *path, bool enable_read_cache = true)
		{
			if (!enable_read_cache) {
				load_file_into_db(_path.c_str(), path, _db, false);
			}

			type_handler_i *th;
			instance_t obj;

			if (db::fetch(_db, path, &th, &obj))
			{
				mem_instance_real *mi = new mem_instance_real();
				mi->is_struct_instance = false;
				mi->th = th;
				mi->eth = get_ext_type_handler_by_name(th->name());
				mi->inst = obj;
				mi->path = strdup(path);
				mi->refs_db = _db;
				return mi;
			}
			else
			{
				if (!enable_read_cache)
				{
					std::cout << "disk_load: Failed to load (" << path << ")!" << std::endl;
					return 0;
				}
				else
				{
					return disk_load(path, false);
				}
			}

			return 0;
		}

		void disk_save(mem_instance *mi_)
		{
			mem_instance_real *mi = (mem_instance_real*)mi_;
			putki::sstream tmp;
			putki::write::write_object_into_stream(tmp, _db, mi->th, mi->inst);

			std::string outpath = (_path + "/" + mi->path) + ".json";

#ifdef _WIN32
			for (unsigned int i=0; i<outpath.size(); i++)
				if (outpath[i] == '\\') {
					outpath[i] = '/';
				}
#else
			for (unsigned int i=0; i<outpath.size(); i++)
				if (outpath[i] == '\\') {
					outpath[i] = '/';
				}
#endif

			sys::mk_dir_for_path(outpath.c_str());

			std::ofstream f(outpath.c_str(), std::ios::binary);
			f.write(tmp.c_str(), tmp.size());

			putki::db::insert(_db, mi->path, mi->th, mi->inst);
			return;
		}

		ext_type_handler_i* type_by_index(unsigned int i)
		{
			return get_ext_type_handler_by_index(i);
		}

		ext_type_handler_i* type_by_name(const char *name)
		{
			return get_ext_type_handler_by_name(name);
		}

		ext_type_handler_i* type_of(mem_instance *mi)
		{
			return ((mem_instance_real*)mi)->eth;
		}

		mem_instance* create_aux_instance(mem_instance *onto, ext_type_handler_i *eth)
		{
			mem_instance_real *org = (mem_instance_real*) onto;
			type_handler_i *th = putki::typereg_get_handler(eth->name());

			instance_t aux_parent = org->inst;
			if (org->is_struct_instance)
			{
				// struct instances have correct path so use that to look up the struct parent.
				type_handler_i *xth;
				instance_t p;
				if (db::fetch(_db, org->path, &xth, &p)) {
					aux_parent = p;
				}
			}

			mem_instance_real *mi = new mem_instance_real;
			mi->is_struct_instance = false;
			mi->path = strdup(db::make_aux_path(_db, aux_parent));
			mi->th = th;
			mi->eth = eth;
			mi->inst = th->alloc();
			mi->refs_db = _db;
			db::insert(_db, mi->path, mi->th, mi->inst);
			return mi;
		}

		virtual void on_object_modified(const char *path)
		{
			liveupdate::editor_on_edited(_client, _db, path);
		}

		const char *path_of(mem_instance *mi)
		{
			return ((mem_instance_real*)mi)->path;
		}
		
		const char* make_json(mem_instance *mi)
		{
			static char buf[1024*1024];
			
			mem_instance_real *mir = (mem_instance_real*) mi;
			
			sstream str;
			write::write_object_into_stream(str, mir->refs_db, mir->th, mir->inst);
			
			const char *cstr = str.c_str();
			if (str.size() < sizeof(buf)) {
				memcpy(buf, cstr, str.size()+1);
				return buf;
			} else {
				return "ERR: Exceeded memory buffer";
			}
		}
		
		const char* content_hash(mem_instance *mi)
		{
                  static char signature[64];
                  static char signature_string[64];
			sstream str;
			mem_instance_real *mir = (mem_instance_real*) mi;
			write::write_object_into_stream(str, mir->refs_db, mir->th, mir->inst);
			const char *cstr = str.c_str();
                  md5_buffer(cstr, (long)str.size(), signature);
                  md5_sig_to_string(signature, signature_string, 64);			
			return signature_string;
		}
	};

	data_dll_i * create_dll_interface(const char *datapath)
	{
		return new data_dll(datapath);
	}

}
