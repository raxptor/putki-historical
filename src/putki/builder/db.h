#pragma once

#include <putki/builder/typereg.h>

namespace putki
{
	namespace db
	{
		struct data;

		struct enum_i
		{
			virtual void record(const char *path, i_type_handler *th, instance_t i) = 0;
		};

		data * create();
		void insert(data *d, const char *path, i_type_handler *th, instance_t i);
		bool fetch(data *d, const char *path, i_type_handler **th, instance_t *obj);

		void read_all(data *d, enum_i *);
		void read_all_by_type(i_type_handler *th, enum_i*);
	}
}
