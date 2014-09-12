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

		// parent db will be used for forwarding path lookups
		data * create(data *parent=0);

		void free_and_destroy_objs(data *d);
		void free(data *);

		void insert(data *d, const char *path, type_handler_i *th, instance_t i);
		bool fetch(data *d, const char *path, type_handler_i **th, instance_t *obj);
		const char *auxref(data *d, const char *path, unsigned int index);
		const char *pathof(data *d, instance_t obj);
		const char *pathof_including_unresolved(data *d, instance_t obj);
		const char *signature(data *d, const char *path);

		void read_all(data *d, enum_i *);
		void read_all_by_type(type_handler_i *th, enum_i*);

		const char *make_aux_path(data *d, instance_t onto);
		bool is_aux_path_of(data *d, instance_t base, const char *path);
		bool is_aux_path(const char *path);

		// returns true if was aux ptr and then writes the base path in result if there is room
		bool base_asset_path(const char *path, char *result, unsigned int bufsize);

		// always returns a pointer which may or may not be unresolved
		instance_t ptr_to_allow_unresolved(data *d, const char *path);

		instance_t create_unresolved_pointer(data *d, const char *path);
		const char *is_unresolved_pointer(data *d, void *p);

		unsigned int size(data *d);
	}
}
