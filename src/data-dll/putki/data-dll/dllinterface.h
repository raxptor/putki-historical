#ifndef __PUTKI_DLL_INTERFACE_H__
#define __PUTKI_DLL_INTERFACE_H__

namespace putki
{
	// putki internal
	struct type_handler_i;

	enum ext_field_type
	{
		EXT_FIELDTYPE_INT32,
		EXT_FIELDTYPE_BYTE,
		EXT_FIELDTYPE_STRING,
		EXT_FIELDTYPE_POINTER,
		EXT_FIELDTYPE_STRUCT_INSTANCE,
		EXT_FIELDTYPE_FILE,
		EXT_FIELDTYPE_INVALID
	};

	struct mem_instance;

	struct ext_field_handler_i
	{
		virtual const char * name() = 0;
		virtual ext_field_type type() = 0;

		virtual void set_string(mem_instance *obj, const char *value) = 0;
		virtual const char* get_string(mem_instance *obj) = 0;
	};

	// dll exposed functionality.
	struct ext_type_handler_i
	{
		virtual const char * name() = 0;
		virtual ext_field_handler_i * field(unsigned int i) = 0;
	};	

	struct data_dll_i
	{
		virtual mem_instance* create_instance(const char *type) = 0;
		virtual mem_instance* disk_load(const char *path) = 0;
		virtual ext_type_handler_i* type_by_index(unsigned int i) = 0;
		virtual ext_type_handler_i* type_by_name(const char *name) { return 0; }
	};

	extern "C"
	{
		__declspec(dllexport) data_dll_i* load_data_dll();
	}
}

#endif
