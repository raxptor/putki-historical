#include "putki_bridge.h"

#include <putki/data-dll/dllinterface.h>

#pragma unmanaged
#include <windows.h>
#pragma managed

namespace putki
{

namespace
{
	putki::data_dll_i * s_dll = 0;

#pragma unmanaged
	typedef putki::data_dll_i* (*GetItFunc)(void);
	void LoadDataDll(const char *name)
	{
		HMODULE mod = LoadLibrary(name);
		GetItFunc f = (GetItFunc) GetProcAddress(mod, "load_data_dll");
		s_dll = f();
	}
#pragma managed
}

Sys::Sys()
{

}

Sys::~Sys()
{

}

void Sys::load(String^ dll)
{
	msclr::interop::marshal_context context;
	std::string myStr = context.marshal_as<std::string>(dll);
	LoadDataDll(myStr.c_str());
}

TypeDefinition^ Sys::get_type_definition(String^ str)
{
	msclr::interop::marshal_context context;
	std::string myStr = context.marshal_as<std::string>(str);
	
	return gcnew TypeDefinition(s_dll->type_by_name(myStr.c_str()));
}

TypeDefinition^ Sys::get_type_by_index(int i)
{
	const ext_type_handler_i *td = s_dll->type_by_index(i);
	if (td)
		return gcnew TypeDefinition(td);

	return nullptr;
}

TypeDefinition::TypeDefinition(const ext_type_handler_i *h)
{
	handler = h;
}

TypeDefinition::~TypeDefinition()
{

}

String^ TypeDefinition::GetName()
{
	return gcnew String(const_cast<ext_type_handler_i *>(handler)->name());
}

}