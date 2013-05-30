#include "dllinterface.h"
#include "dllinternal.h"

#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/db.h>

#include <string>
#include <iostream>

void app_bind_putki_types();
void app_bind_putki_types_dll();

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

		mem_instance* create_instance(ext_type_handler_i *eth)
		{
			type_handler_i *th = putki::typereg_get_handler(eth->name());

			mem_instance_real *mi = new mem_instance_real();
			mi->th = th;
			mi->eth = eth;
			mi->inst = th->alloc();

			return mi;
		}

		void free_instance(mem_instance *mi)
		{
			((mem_instance_real*)mi)->th->free(((mem_instance_real*)mi)->inst);
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
				mi->th = th;
				mi->eth = get_ext_type_handler_by_name(th->name());
				mi->inst = obj;

				std::cout << mi->th << " " << mi->eth << " " << mi->inst;
				return mi;
			}

			std::cout << "!Not found in DB!" << std::endl;
			return 0;
		}

		ext_type_handler_i* type_by_index(unsigned int i)
		{
			return get_ext_type_handler_by_index(i);
		}

		ext_type_handler_i* type_of(mem_instance *mi)
		{
			return ((mem_instance_real*)mi)->eth;
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