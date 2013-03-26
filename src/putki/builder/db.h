#pragma once

#include <putki/builder/typereg.h>

namespace putki
{
	namespace db
	{
		struct data;

		typedef void (*record_enum_t) (const char *path, i_type_handler *th, type_inst i);

		data * create();
		void insert(data *d, const char *path, i_type_handler *th, type_inst i);
		bool fetch(data *d, const char *path, i_type_handler **th, type_inst *obj);

		void read_all(data *d, record_enum_t func);
	}
}
