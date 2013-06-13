#include <putki/data-dll/dllinterface.h>

namespace inki
{
	void bind_test_project();
	void bind_test_project_dll();
}

extern "C"
{
	__declspec(dllexport) putki::data_dll_i* __cdecl load_data_dll(const char *data_path)
	{
		inki::bind_test_project();
		inki::bind_test_project_dll();

		// bind at startup.
		return putki::create_dll_interface(data_path);
	}
}
