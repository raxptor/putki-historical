#include "dllinterface.h"
#include "dllinternal.h"

#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/write.h>
#include <putki/builder/db.h>
#include <putki/builder/build.h>
#include <putki/sys/compat.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <windows.h>

void app_bind_putki_types();
void app_bind_putki_types_dll();
void app_register_handlers(putki::builder::data *builder);

namespace putki
{

	data_dll_i::~data_dll_i()
	{

	}

	struct data_dll : public data_dll_i
	{
		db::data *_db;
		std::string _path;

		data_dll(const char *path)
		{
			_db = db::create();
			_path = path;
		}

		~data_dll()
		{
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

		mem_instance* disk_load(const char *path) 
		{
			std::string _fn = std::string(path) + ".json";
			load_file_into_db(_path.c_str(), _fn.c_str(), _db, false);

			std::cout << "Loaded " << path << std::endl;
			type_handler_i *th;
			instance_t obj;
			
			if (db::fetch(_db, path, &th, &obj))
			{
				std::cout << "Found in db!" << std::endl;
				mem_instance_real *mi = new mem_instance_real();
				mi->is_struct_instance = false;
				mi->th = th;
				mi->eth = get_ext_type_handler_by_name(th->name());
				mi->inst = obj;
				mi->path = strdup(path);
				mi->refs_db = _db;

				std::cout << mi->th << " " << mi->eth << " " << mi->inst;
				return mi;
			}

			std::cout << "!Not found in DB!" << std::endl;
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
				if (outpath[i] == '/')
					outpath[i] = '\\';
#else
			for (unsigned int i=0;i<outpath.size();i++)
				if (outpath[i] == '\\')
					outpath[i] = '/';
#endif

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
			putki::builder::data* builder = putki::builder::create(putki::RUNTIME_CPP_WIN32);
			app_register_handlers(builder);

			putki::db::data *output = putki::db::create();
			putki::builder::build_source_object(builder, _db, path, output);
			putki::builder::free(builder);
		}

		const char *path_of(mem_instance *mi)
		{
			return ((mem_instance_real*)mi)->path;
		}
	};

	data_dll_i* __cdecl load_data_dll(const char *data_path)
	{
		// bind at startup.
		app_bind_putki_types();
		app_bind_putki_types_dll();	

		return new data_dll(data_path);
	}

}