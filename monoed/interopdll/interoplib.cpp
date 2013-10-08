#include <data-dll/dllinterface.h>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

// SIMPLIFIED INTERFACE FOR MONO ED 

#define DSPEC __declspec(dllexport)

putki::data_dll_i * g_loaded_dll = 0;

typedef putki::data_dll_i* (CreateF)(const char *dataPath);

extern "C"
{
	DSPEC int MED_Initialize(const char *dllPath, const char *dataPath)
	{
		std::cout << "MED_Initialize: Loading library [" << dllPath << "]" << std::endl;

#if defined(_WIN32)
		HMODULE module = LoadLibrary(dllPath);
		if (module)
		{
			CreateF *f = (CreateF*) GetProcAddress(module, "load_data_dll");
			if (f)
			{
				g_loaded_dll = f(dataPath);
			}
			else
			{
				std::cout << "MED_Initialize: Failed loading entry point." << std::endl;
			}
		}
		else
		{
			std::cout << "MED_Initialize: Failed loading library" << std::endl;
		}
#endif
		return g_loaded_dll != 0;
	}

	DSPEC putki::ext_type_handler_i* MED_TypeByIndex(int i)
	{
		return g_loaded_dll->type_by_index(i);
	}

	DSPEC putki::ext_type_handler_i* MED_TypeOf(putki::mem_instance *mi)
	{
		return g_loaded_dll->type_of(mi);
	}

	DSPEC putki::mem_instance* MED_DiskLoad(const char *path)
	{
		return g_loaded_dll->disk_load(path);
	}

	DSPEC const char* MED_Type_GetName(putki::ext_type_handler_i* type)
	{
		return type->name();
	}

}