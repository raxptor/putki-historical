#pragma once

#include <putki/builder/typereg.h>

namespace putki
{
	namespace sys { struct mutex; }
	
	namespace db
	{
		struct data;
		
		typedef bool (*deferred_load_fn)(data *db, const char *path, type_handler_i **th, instance_t *obj, void *userptr);
		typedef void (*on_destroy_fn)(void *userptr);

		struct enum_i
		{
			virtual void record(const char *path, type_handler_i *th, instance_t i) = 0;
		};

		// parent db will be used for forwarding path lookups
		data * create(data *parent=0, sys::mutex *mtx=0);
		void enable_erase_on_overwrite(data *d);

		void register_on_destroy(data *d, on_destroy_fn fn, void *userptr);
		void free_and_destroy_objs(data *d);
		void free(data *d, data *unresolved_target = 0);

		void insert_deferred(data *d, const char *path, deferred_load_fn, void *userptr);
		void insert(data *d, const char *path, type_handler_i *th, instance_t i);
		void copy_obj(data *source, data *dest, const char *path);
		bool start_loading(data *d, const char *path);
		void done_loading(data *d, const char *path);
		
		// includes deferred loads
		bool exists(data *d, const char *path, bool include_loading=false);
		
		// will trigger deferred load to execute if not loaded
		bool fetch(data *d, const char *path, type_handler_i **th, instance_t *obj, bool allow_execute_deferred=true, bool iamtheloader=false);
		
		const char *auxref(data *d, const char *path, unsigned int index);
		const char *pathof(data *d, instance_t obj);
		const char *pathof_including_unresolved(data *d, instance_t obj);
		const char *signature(data *d, const char *path, char *buffer);

		void read_all_no_fetch(data *d, enum_i *); // this can return empty th/obj for deferred loads
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
