#ifndef __PUTKI_DLL_INTERFACE_H__
#define __PUTKI_DLL_INTERFACE_H__

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
		EXT_FIELDTYPE_STRUCT_INSTANCE = 4,
		EXT_FIELDTYPE_FILE = 5,
		EXT_FIELDTYPE_INVALID = 6
	};

	struct mem_instance { };

	struct ext_field_handler_i
	{
		virtual const char * name() = 0;
		virtual ext_field_type type() = 0;
		virtual const char * ref_type_name() = 0;

		// String
		virtual void set_string(mem_instance *obj, const char *value) = 0;
		virtual const char* get_string(mem_instance *obj) = 0;

		// Pointer
		virtual void set_pointer(mem_instance *obj, const char *value) = 0;
		virtual const char* get_pointer(mem_instance *obj) = 0;

		// Byte
		virtual void set_byte(mem_instance *obj, unsigned char value) = 0;
		virtual unsigned char get_byte(mem_instance *obj) = 0;
		
		virtual mem_instance* make_struct_instance(mem_instance *obj) = 0;
	};

	// dll exposed functionality.
	struct ext_type_handler_i
	{
		virtual const char * name() = 0;
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

		virtual void free_instance(mem_instance *mi) = 0;

		virtual mem_instance* disk_load(const char *path) = 0;
		virtual void disk_save(mem_instance *mi) = 0;

		virtual ext_type_handler_i* type_of(mem_instance *mi) = 0;
		virtual ext_type_handler_i* type_by_index(unsigned int i) = 0;
		virtual ext_type_handler_i* type_by_name(const char *name) = 0;

		virtual void mem_build_asset(const char *path, ext_build_result *res) = 0;
		virtual void on_object_modified(const char *path) = 0;

		virtual const char *path_of(mem_instance *mi) = 0;
	};

	extern "C"
	{
		__declspec(dllexport) data_dll_i* load_data_dll(const char *datapath);
	}
}

#endif
