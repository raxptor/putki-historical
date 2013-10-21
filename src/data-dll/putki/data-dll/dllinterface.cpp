#include "dllinterface.h"
#include "dllinternal.h"

#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/write.h>
#include <putki/builder/db.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/sys/compat.h>
#include <putki/sys/files.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace putki
{
	namespace
	{
		const char *_lu_path = 0;
		liveupdate::data *s_live_update = 0;
	}

	data_dll_i::~data_dll_i()
	{

	}

	void liveupdate_thread_real(int socket)
	{
		std::cout << "Hello from the thread, socket=" << socket << std::endl;
		putki::liveupdate::service_client(s_live_update , _lu_path, socket);
		std::cout << "Client exiting" << std::endl;
	}

#ifdef _WIN32

	DWORD WINAPI liveupdate_thread(LPVOID arg)
	{
		liveupdate_thread_real((int)arg);
		return 0;
	}

	DWORD WINAPI accept_thread(LPVOID arg)
	{
		std::cout << "[data-dll] started live update accept thread" << std::endl;
		liveupdate::data *d = (liveupdate::data*) arg;
		while (true)
		{
			int s = putki::liveupdate::accept(d);
			intptr_t skt = s;
			CreateThread(0, 0, &liveupdate_thread, (void*)skt, 0, 0);
		}
	}
#else

	void* liveupdate_thread(void *r)
	{
		liveupdate_thread_real((int)r);
		return 0;
	}

	void* accept_thread(void *arg)
	{
		std::cout << "[data-dll] started live update accept thread" << std::endl;
		liveupdate::data *d = (liveupdate::data*) arg;
		while (true)
		{
			int s = putki::liveupdate::accept(d);
			intptr_t skt = s;
			pthread_t t;
			pthread_create(&t, 0, &liveupdate_thread, (void*)skt);
		}
		return 0;
	}

#endif

	struct data_dll : public data_dll_i
	{
		db::data *_db;
		std::string _path;
		liveupdate::data *_lu;

		data_dll(const char *path)
		{
			_db = db::create();
			_path = path;
			_path.append("/data/objs");

#ifdef _WIN32

			#if defined(_WIN32)
				WSADATA wsaData;
				if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
				{
					std::cerr << "WSA init failure" << std::endl;
					return;
				}
			#endif

			_lu = putki::liveupdate::start_server(_db);

			_lu_path = _strdup(path); // ugly!
			s_live_update  = _lu;

			std::cout << "live update (" << _lu << ") " << std::endl;

			if (_lu)
			{
				CreateThread(0, 0, &accept_thread, (void*)_lu, 0, 0);
			}
#else
			_lu = putki::liveupdate::start_server(_db);
			_lu_path = strdup(path);
			s_live_update = _lu;

			if (_lu)
			{
				pthread_t thr;
				pthread_create(&thr, 0, &accept_thread, _lu);
			}
#endif
		}

		~data_dll()
		{
			if (_lu)
				liveupdate::stop_server(_lu);

			db::free(_db);
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

		mem_instance* disk_load(const char *path, bool enable_read_cache = true) 
		{
			if (!enable_read_cache)
				load_file_into_db(_path.c_str(), path, _db, false);

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
			std::stringstream tmp;
			putki::write::write_object_into_stream(tmp, _db, mi->th, mi->inst);

			std::string outpath = (_path + "/" + mi->path) + ".json";

#ifdef _WIN32
			for (unsigned int i=0;i<outpath.size();i++)
				if (outpath[i] == '\\')
					outpath[i] = '/';
#else
			for (unsigned int i=0;i<outpath.size();i++)
				if (outpath[i] == '\\')
					outpath[i] = '/';
#endif

			sys::mk_dir_for_path(outpath.c_str());

			std::ofstream f(outpath.c_str());
			f << tmp.str();
			
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

		void mem_build_asset(const char *path, ext_build_result *res)
		{
			/*
			putki::builder::data* builder = putki::builder::create(putki::RUNTIME_CPP_WIN32);

			putki::db::data *output = putki::db::create();
			putki::builder::build_source_object(builder, _db, path, output);
			putki::builder::free(builder);

			if (s_live_update)
				putki::liveupdate::send_update(s_live_update, path);
			*/
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
				if (db::fetch(_db, org->path, &xth, &p))
					aux_parent = p;
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
			if (s_live_update)
				putki::liveupdate::send_update(s_live_update, path);
		}

		const char *path_of(mem_instance *mi)
		{
			return ((mem_instance_real*)mi)->path;
		}
	};

	data_dll_i * create_dll_interface(const char *datapath)
	{
		return new data_dll(datapath);
	}

}
