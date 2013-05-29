#ifndef __PUTKI_DLL_INTERFACE_H__
#define __PUTKI_DLL_INTERFACE_H__

namespace putki
{
	// putki internal
	struct type_handler_i;

	// dll exposed functionality.
	struct ext_type_handler_i
	{
		virtual const char *name() = 0;
	};

	struct mem_instance;

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
