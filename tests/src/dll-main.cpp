#include <putki/data-dll/dllinterface.h>
#include <putki/builder/builder.h>

namespace inki
{
	void bind_test();
	void bind_test_editor();
}

void test_register_handlers(putki::builder::data *builder);

void setup_builder(putki::builder::data *builder)
{
	test_register_handlers(builder);
}

extern "C"
{
	#if defined(_MSC_VER)
	__declspec(dllexport) putki::data_dll_i* __cdecl load_data_dll(const char *data_path)
	#else
	putki::data_dll_i* load_data_dll(const char *data_path)
	#endif
	{
		inki::bind_test();
		inki::bind_test_editor();

		putki::builder::set_builder_configurator(&setup_builder);

		// bind at startup.
		return putki::create_dll_interface(data_path);
	}

}