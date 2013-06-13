#include <putki/data-dll/dllinterface.h>

namespace inki
{
	void bind_claw();
	void bind_claw_dll();
}

extern "C"
{
	
	__declspec(dllexport) putki::data_dll_i* __cdecl load_data_dll(const char *data_path)
	{
		inki::bind_claw();
		inki::bind_claw_dll();

		// bind at startup.
		return putki::create_dll_interface(data_path);
	}

}