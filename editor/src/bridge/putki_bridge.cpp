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

TypeDefinition^ FieldHandler::GetRefType()
{
	const char *refType = m_handler->ref_type_name();
	if (!refType)
		return nullptr;

	return gcnew TypeDefinition(s_dll->type_by_name(refType));
}

MemInstance^ FieldHandler::GetStructInstance(MemInstance^ Obj)
{
	// how to free this?!	
	return gcnew MemInstance(GetRefType(), m_handler->make_struct_instance(Obj->GetPutkiMemInstance()));
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
		return gcnew MemInstance(nullptr, nullptr);
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