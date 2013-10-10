#include <data-dll/dllinterface.h>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

// SIMPLIFIED INTERFACE FOR MONO ED 
#ifdef _WIN32
#define DSPEC __declspec(dllexport)
#else
#define DSPEC __cdecl
#include<dlfcn.h>
#endif

putki::data_dll_i * g_loaded_dll = 0;

typedef putki::data_dll_i* (CreateF)(const char *dataPath);

extern "C"
{
	DSPEC int MED_Initialize(const char *dllPath, const char *dataPath)
	{
		std::cout << "MED_Initialize: Loading library [" << dllPath << "]" << std::endl;

#if defined(_WIN32)
		std::cout << "MED_Initialize: Loading library [" << dllPath << "]" << std::endl;
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
#else

		std::cout << "MED_Initialize: dlopen" << std::endl;
		void *p = dlopen(dllPath, RTLD_NOW);
		std::cout << "MED_Initialize: returned (" << p << ")" << std::endl;
		if (p)
		{
			std::cout << "MED_Initialize: Loaded library (" << p << ")" << std::endl;
			CreateF *f = (CreateF*) dlsym(p, "load_data_dll");

			std::cout << "MED_Initialize: Resolved symbol to (" << f << ")" << std::endl;
			if (f)
			{
				g_loaded_dll = f(dataPath);
				std::cout << "Loaded dll interface at " << g_loaded_dll << "!" << std::endl;
			}
			else
			{
				std::cerr << "Failed finding entry point in module [" << dllPath << "]." << std::endl;
			}
		}
		else
		{
			std::cout << "Failed loading library [" << dllPath << "]" << std::endl;
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

	DSPEC const char* MED_Field_GetName(putki::ext_field_handler_i *field)
	{
		return field->name();
	}

	DSPEC putki::ext_field_handler_i * MED_Type_GetField(putki::ext_type_handler_i * type, int i)
	{
		return type->field(i);
	}

	DSPEC int MED_Field_GetType(putki::ext_field_handler_i * field)
	{
		return field->type();
	}

	DSPEC int MED_Field_IsArray(putki::ext_field_handler_i * field)
	{
		return field->is_array();
	}

	DSPEC int MED_Field_GetArraySize(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_array_size(mi);
	}

	DSPEC void MED_Field_SetArrayIndex(putki::ext_field_handler_i * field, int index)
	{
		return field->set_array_index(index);
	}

	DSPEC const char * MED_Field_GetString(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_string(mi);
	}

	DSPEC int MED_Field_GetInt32(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_int32(mi);
	}

	DSPEC bool MED_Field_GetBool(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_bool(mi);
	}

	DSPEC putki::mem_instance* MED_Field_GetStructInstance(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->make_struct_instance(mi);
	}
}
