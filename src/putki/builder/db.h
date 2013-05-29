#pragma once

#include <putki/builder/typereg.h>

namespace putki
{
	namespace db
	{
		struct data;

		struct enum_i
		{
			virtual void record(const char *path, type_handler_i *th, instance_t i) = 0;
		};

		data * create();
		void insert(data *d, const char *path, type_handler_i *th, instance_t i);
		bool fetch(data *d, const char *path, type_handler_i **th, instance_t *obj);
		const char *pathof(data *d, instance_t obj);

		void read_all(data *d, enum_i *);
		void read_all_by_type(type_handler_i *th, enum_i*);

		unsigned int size(data *d);
	}
}