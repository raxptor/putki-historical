#ifndef __PUTKI_DLL_INTERFACE_H__
#define __PUTKI_DLL_INTERFACE_H__

#include <stdint.h>

namespace putki
{
	// putki internal
	struct type_handler_i;
	namespace db { struct data; }

	enum ext_field_type
	{
		EXT_FIELDTYPE_INT32 = 0,
		EXT_FIELDTYPE_BYTE = 1,
		EXT_FIELDTYPE_STRING = 2,
		EXT_FIELDTYPE_POINTER = 3,
		EXT_FIELDTYPE_PATH = 4,
		EXT_FIELDTYPE_STRUCT_INSTANCE = 5,
		EXT_FIELDTYPE_FILE = 7,
		EXT_FIELDTYPE_BOOL = 8,
		EXT_FIELDTYPE_FLOAT = 9,
		EXT_FIELDTYPE_ENUM = 10,
		EXT_FIELDTYPE_UINT32 = 11,
		EXT_FIELDTYPE_INVALID = 12
	};

	struct mem_instance { };

	struct ext_field_handler_i
	{
		virtual const char * name() = 0;
		virtual bool show_in_editor() = 0;

		virtual ext_field_type type() = 0;
		virtual const char * ref_type_name() = 0;

		virtual const char * get_enum_possible_value(int idx) = 0;
		virtual void set_enum(mem_instance *obj, const char *value) = 0;
		virtual const char* get_enum(mem_instance *obj) = 0;

		virtual bool is_array() = 0;
		virtual void set_array_index(int i) = 0;

		virtual int get_array_size(putki::mem_instance *obj) = 0;
		virtual void array_insert(putki::mem_instance *obj) = 0;
		virtual void array_erase(putki::mem_instance *obj) = 0;
		virtual bool is_aux_ptr() = 0;

		// String
		virtual void set_string(mem_instance *obj, const char *value) = 0;
		virtual const char* get_string(mem_instance *obj) = 0;

		// Integer based values (bools, ints etc)
		virtual int set_integer(mem_instance *obj, int64_t value) = 0;
		virtual int64_t get_integer(mem_instance *obj) = 0;
		
		virtual void set_float(mem_instance *obj, float value) = 0;
		virtual float get_float(mem_instance *obj) = 0;

		// Pointer
		virtual void set_pointer(mem_instance *obj, const char *value) = 0;
		virtual const char* get_pointer(mem_instance *obj) = 0;

		virtual mem_instance* make_struct_instance(mem_instance *obj) = 0;
	};

	// dll exposed functionality.
	struct ext_type_handler_i
	{
		virtual const char * name() = 0;
		virtual const char * module_name() = 0;
		virtual const char * parent_name() = 0;
		virtual const char * inline_editor() = 0;

		virtual bool permit_as_aux_instance() = 0;
		virtual bool permit_as_asset() = 0;
		
		virtual ext_field_handler_i * field(unsigned int i) = 0;
	};

	struct ext_build_result
	{
		bool successful;
		char text[4096];
	};

	struct data_dll_i
	{
		virtual ~data_dll_i();

		virtual mem_instance* create_instance(const char *path, ext_type_handler_i *th) = 0;
		virtual mem_instance* create_aux_instance(mem_instance *onto, ext_type_handler_i *th) = 0;
		virtual void free_instance(mem_instance *mi) = 0;
		
		virtual const char *get_status() = 0;

		virtual mem_instance* disk_load(const char *path, bool enable_read_cache = true) = 0;
		virtual void disk_save(mem_instance *mi) = 0;

		virtual ext_type_handler_i* type_of(mem_instance *mi) = 0;
		virtual ext_type_handler_i* type_by_index(unsigned int i) = 0;
		virtual ext_type_handler_i* type_by_name(const char *name) = 0;
		
		virtual const char* make_json(mem_instance *mi) = 0;
		virtual const char* content_hash(mem_instance *mi) = 0;

		virtual void mem_build_asset(const char *path, ext_build_result *res) = 0;
		virtual void on_object_modified(const char *path) = 0;

		virtual const char *path_of(mem_instance *mi) = 0;
	};

	data_dll_i * create_dll_interface(const char *datapath);
}

#endif
