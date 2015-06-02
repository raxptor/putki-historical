#include <data-dll/dllinterface.h>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

#ifdef _WIN32
#define DSPEC __declspec(dllexport)
#else
#define DSPEC __cdecl
#include <dlfcn.h>
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
				g_loaded_dll = f(dataPath);
			else
				std::cout << "MED_Initialize: Failed loading entry point." << std::endl;
		}
		else
		{
			std::cout << "MED_Initialize: Failed loading library" << std::endl;
		}
#else

		std::cout << "MED_Initialize: dlopen" << std::endl;
		void *p = dlopen(dllPath, RTLD_LAZY | RTLD_FIRST);
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
			std::cout << "Failed loading library [" << dllPath << "] because (" << dlerror() << ")" << std::endl;
		}
#endif
		return g_loaded_dll != 0;
	}

	DSPEC const char *MED_GetStatus()
	{
		return g_loaded_dll->get_status();
	}

	DSPEC putki::ext_type_handler_i* MED_TypeByIndex(int i)
	{
		return g_loaded_dll->type_by_index(i);
	}

	DSPEC putki::ext_type_handler_i* MED_TypeByName(const char *name)
	{
		return g_loaded_dll->type_by_name(name);
	}

	DSPEC putki::ext_type_handler_i* MED_Type_GetParentType(putki::ext_type_handler_i *input)
	{
		const char *pn = input->parent_name();
		if (pn)
			return g_loaded_dll->type_by_name(pn);
		return 0;
	}

	DSPEC putki::ext_type_handler_i* MED_TypeOf(putki::mem_instance *mi)
	{
		return g_loaded_dll->type_of(mi);
	}
	
	DSPEC const char *MED_MakeJSON(putki::mem_instance *mi)
	{
		return g_loaded_dll->make_json(mi);
	}
	
	DSPEC const char *MED_ContentHash(putki::mem_instance *mi)
	{
		return g_loaded_dll->content_hash(mi);
	}

	DSPEC const char *MED_PathOf(putki::mem_instance *mi)
	{
		return g_loaded_dll->path_of(mi);
	}

	DSPEC putki::mem_instance* MED_DiskLoad(const char *path)
	{
		return g_loaded_dll->disk_load(path);
	}

	DSPEC putki::mem_instance* MED_CreateInstance(const char *path, putki::ext_type_handler_i * type)
	{
		return g_loaded_dll->create_instance(path, type);
	}

	DSPEC putki::mem_instance* MED_CreateAuxInstance(putki::mem_instance *onto, putki::ext_type_handler_i * type)
	{
		return g_loaded_dll->create_aux_instance(onto, type);
	}

	DSPEC void MED_DiskSave(putki::mem_instance *mi)
	{
		return g_loaded_dll->disk_save(mi);
	}

	DSPEC void MED_OnObjectModified(putki::mem_instance *mi)
	{
		return g_loaded_dll->on_object_modified(g_loaded_dll->path_of(mi));
	}

	DSPEC const char* MED_Type_GetName(putki::ext_type_handler_i* type)
	{
		return type->name();
	}

	DSPEC const char* MED_Type_GetModuleName(putki::ext_type_handler_i* type)
	{
		return type->module_name();
	}

	DSPEC int MED_Type_PermitAsAsset(putki::ext_type_handler_i* type)
	{
		return type->permit_as_asset();
	}

	DSPEC int MED_Type_PermitAsAuxInstance(putki::ext_type_handler_i* type)
	{
		return type->permit_as_aux_instance();
	}

	DSPEC const char* MED_Type_GetInlineEditor(putki::ext_type_handler_i* type)
	{
		return type->inline_editor();
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

	DSPEC int MED_Field_ShowInEditor(putki::ext_field_handler_i * field)
	{
		return field->show_in_editor();
	}

	DSPEC const char * MED_Field_GetRefType(putki::ext_field_handler_i * field)
	{
		return field->ref_type_name();
	}

	DSPEC int MED_Field_GetArraySize(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_array_size(mi);
	}

	DSPEC void MED_Field_SetArrayIndex(putki::ext_field_handler_i * field, int index)
	{
		return field->set_array_index(index);
	}

	DSPEC void MED_Field_ArrayInsert(putki::ext_field_handler_i * field, putki::mem_instance *mi)
	{
		return field->array_insert(mi);
	}

	DSPEC void MED_Field_ArrayErase(putki::ext_field_handler_i * field, putki::mem_instance *mi)
	{
		return field->array_erase(mi);
	}

	DSPEC int MED_Field_IsAuxPtr(putki::ext_field_handler_i * field)
	{
		return field->is_aux_ptr();
	}

	DSPEC const char * MED_Field_GetString(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_string(mi);
	}

	DSPEC float MED_Field_GetFloat(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_float(mi);
	}

	DSPEC const char * MED_Field_GetPointer(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_pointer(mi);
	}

	DSPEC uint64_t MED_Field_GetInteger(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->get_integer(mi);
	}

	DSPEC putki::mem_instance* MED_Field_GetStructInstance(putki::ext_field_handler_i * field, putki::mem_instance * mi)
	{
		return field->make_struct_instance(mi);
	}

	DSPEC const char* MED_Field_GetEnumPossibility(putki::ext_field_handler_i* field, int i)
	{
		return field->get_enum_possible_value(i);
	}

	DSPEC const char* MED_Field_GetEnum(putki::ext_field_handler_i *field, putki::mem_instance *mi)
	{
		return field->get_enum(mi);
	}

	DSPEC void MED_Field_SetEnum(putki::ext_field_handler_i * field, putki::mem_instance * mi, const char *value)
	{
		return field->set_enum(mi, value);
	}

	DSPEC void MED_Field_SetString(putki::ext_field_handler_i * field, putki::mem_instance * mi, const char *value)
	{
		return field->set_string(mi, value);
	}

	DSPEC void MED_Field_SetPointer(putki::ext_field_handler_i * field, putki::mem_instance * mi, const char *value)
	{
		return field->set_pointer(mi, value);
	}

	DSPEC void MED_Field_SetInteger(putki::ext_field_handler_i * field, putki::mem_instance * mi, int64_t value)
	{
		int ret = field->set_integer(mi, value);
		if (!ret) {
			std::cerr << "MED_Field_SetInteger:set_integer returned failure (ret=" << ret << ") setting " << value << std::endl;
			std::cerr << "Field is " << field->name() << std::endl;
			std::cerr << "Type is " << field->type() << std::endl;
			std::cerr << "Path is " << g_loaded_dll->path_of(mi) << std::endl;
		}
	}

	DSPEC void MED_Field_SetFloat(putki::ext_field_handler_i * field, putki::mem_instance * mi, float value)
	{
		return field->set_float(mi, value);
	}
}


