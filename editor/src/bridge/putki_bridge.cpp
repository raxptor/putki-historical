#include "putki_bridge.h"

#include <putki/data-dll/dllinterface.h>

#pragma unmanaged
#include <windows.h>
#pragma managed

namespace Putki
{

namespace
{
	putki::data_dll_i * s_dll = 0;

#pragma unmanaged
	typedef putki::data_dll_i* (*GetItFunc)(const char *path);

	void LoadDataDll(const char *name, const char *datapath)
	{
		HMODULE mod = LoadLibrary(name);
		GetItFunc f = (GetItFunc) GetProcAddress(mod, "load_data_dll");
		s_dll = f(datapath);
	}
#pragma managed
}

	
TypeDefinition::TypeDefinition(putki::ext_type_handler_i *h)
{
	handler = h;
}

TypeDefinition::~TypeDefinition()
{

}

String^ TypeDefinition::GetName()
{
	return gcnew String(const_cast<putki::ext_type_handler_i *>(handler)->name());
}

String^ TypeDefinition::GetInlineEditor()
{
	return gcnew String(const_cast<putki::ext_type_handler_i *>(handler)->inline_editor());
}


FieldHandler^ TypeDefinition::GetField(int i)
{
	putki::ext_field_handler_i *field = handler->field(i);
	if (field)
		return gcnew FieldHandler(field);
	else
		return nullptr;
}

String^ FieldHandler::GetName()
{
	return gcnew String(m_handler->name());
}

FieldType FieldHandler::GetType()
{
	return (FieldType) m_handler->type(); 
}

String^ FieldHandler::GetEnumValueByIndex(int index)
{
	const char *name = m_handler->get_enum_possible_value(index);
	if (!name)
		return nullptr;
	return gcnew String(name);
}

bool FieldHandler::IsArray()
{
	return m_handler->is_array();
}

int FieldHandler::GetArraySize(MemInstance^ instance)
{
	return m_handler->get_array_size(instance->GetPutkiMemInstance());
}

void FieldHandler::SetArrayIndex(int index)
{
	m_handler->set_array_index(index);
}

void FieldHandler::ArrayInsert(MemInstance^ instance)
{
	m_handler->array_insert(instance->GetPutkiMemInstance());
}

void FieldHandler::ArrayErase(MemInstance^ instance)
{
	m_handler->array_erase(instance->GetPutkiMemInstance());
}

TypeDefinition^ FieldHandler::GetRefType()
{
	const char *refType = m_handler->ref_type_name();
	if (!refType)
		return nullptr;

	return gcnew TypeDefinition(s_dll->type_by_name(refType));
}

TypeDefinition^ TypeDefinition::GetParentType()
{
	const char *pt = handler->parent_name();
	if (pt)
	{
		return gcnew TypeDefinition(s_dll->type_by_name(pt));
	}
	return nullptr;
}

MemInstance^ FieldHandler::GetStructInstance(MemInstance^ Obj)
{
	// how to free this?!	
	return gcnew MemInstance(GetRefType(), m_handler->make_struct_instance(Obj->GetPutkiMemInstance()));
}

bool FieldHandler::ShowInEditor()
{
	return m_handler->show_in_editor();
}

String^ FieldHandler::GetString(MemInstance^ instance)
{
	return gcnew String(m_handler->get_string(instance->GetPutkiMemInstance()));
}

void FieldHandler::SetString(MemInstance^ instance, String^ Value)
{
	msclr::interop::marshal_context context;
	std::string _value = context.marshal_as<std::string>(Value);
	m_handler->set_string(instance->GetPutkiMemInstance(), _value.c_str());
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

String^ FieldHandler::GetEnum(MemInstance^ instance)
{
	return gcnew String(m_handler->get_enum(instance->GetPutkiMemInstance()));
}

void FieldHandler::SetEnum(MemInstance^ instance, String^ Value)
{
	msclr::interop::marshal_context context;
	std::string _value = context.marshal_as<std::string>(Value);
	m_handler->set_enum(instance->GetPutkiMemInstance(), _value.c_str());
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

String^ FieldHandler::GetPointer(MemInstance^ instance)
{
	return gcnew String(m_handler->get_pointer(instance->GetPutkiMemInstance()));
}

void FieldHandler::SetPointer(MemInstance^ instance, String^ Value)
{
	msclr::interop::marshal_context context;
	std::string _value = context.marshal_as<std::string>(Value);
	m_handler->set_pointer(instance->GetPutkiMemInstance(), _value.c_str());
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

int FieldHandler::GetByte(MemInstance^ instance)
{
	return m_handler->get_byte(instance->GetPutkiMemInstance());
}

void FieldHandler::SetByte(MemInstance^ instance, int Value)
{
	m_handler->set_byte(instance->GetPutkiMemInstance(), Value);
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

int FieldHandler::GetInt32(MemInstance^ instance)
{
	return m_handler->get_int32(instance->GetPutkiMemInstance());
}

void FieldHandler::SetInt32(MemInstance^ instance, int Value)
{
	m_handler->set_int32(instance->GetPutkiMemInstance(), Value);
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

bool FieldHandler::GetBool(MemInstance^ instance)
{
	return m_handler->get_bool(instance->GetPutkiMemInstance());
}

void FieldHandler::SetBool(MemInstance^ instance, bool Value)
{
	m_handler->set_bool(instance->GetPutkiMemInstance(), Value);
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

float FieldHandler::GetFloat(MemInstance^ instance)
{
	return m_handler->get_float(instance->GetPutkiMemInstance());
}

void FieldHandler::SetFloat(MemInstance^ instance, float Value)
{
	m_handler->set_float(instance->GetPutkiMemInstance(), Value);
	s_dll->on_object_modified(s_dll->path_of(instance->GetPutkiMemInstance()));
}

bool FieldHandler::IsAuxPtr()
{
	return m_handler->is_aux_ptr();
}

MemInstance^ Sys::CreateAuxInstance(MemInstance ^onto, TypeDefinition^ type)
{
	return gcnew MemInstance(type, s_dll->create_aux_instance(onto->GetPutkiMemInstance(), type->GetPutkiTypeDefinition()));
}

MemInstance^ Sys::CreateInstance(String^ path, TypeDefinition^ type)
{
	msclr::interop::marshal_context context;
	std::string p = context.marshal_as<std::string>(path);
	return gcnew MemInstance(type, s_dll->create_instance(p.c_str(), type->GetPutkiTypeDefinition()));
}

void Sys::MemBuildAsset(String^ path)
{
	msclr::interop::marshal_context context;
	std::string _value = context.marshal_as<std::string>(path);
	s_dll->mem_build_asset(_value.c_str(), nullptr);
}

void Sys::Load(String^ dll, String^ datapath)
{
	msclr::interop::marshal_context context;
	std::string _dll = context.marshal_as<std::string>(dll);
	std::string _path = context.marshal_as<std::string>(datapath);
	LoadDataDll(_dll.c_str(), _path.c_str());
}

MemInstance^ Sys::LoadFromDisk(String^ path)
{
	msclr::interop::marshal_context context;
	std::string myStr = context.marshal_as<std::string>(path);
	
	putki::mem_instance *mi = s_dll->disk_load(myStr.c_str());
	if (mi == nullptr)
		return nullptr;
	else
	{
		return gcnew MemInstance(gcnew TypeDefinition(s_dll->type_of(mi)), mi);
	}
}

void Sys::SaveObject(MemInstance^ mi)
{
	s_dll->disk_save(mi->GetPutkiMemInstance());
}

TypeDefinition^ Sys::GetTypeByIndex(int i)
{
	putki::ext_type_handler_i *td = s_dll->type_by_index(i);
	if (td)
		return gcnew TypeDefinition(td);

	return nullptr;
}

MemInstance::MemInstance(TypeDefinition^ type, putki::mem_instance *mem_instance)
{
	m_type = type;
	m_instance = mem_instance;
}

MemInstance::~MemInstance()
{
}

String^ MemInstance::GetPath()
{
	return gcnew String(s_dll->path_of(m_instance));
}


}	