#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/app.h>
#include <putki/runtime.h>

int run_putki_builder(int argc, char **argv)
{
	// configure builder with app handlers.
	putki::builder::data *builder = putki::builder::create(putki::RUNTIME_CPP_WIN32);
	app_register_handlers(builder);
	
	putki::build::full_build(builder, "data/", "out/win32/temp/", "out/win32/packages/");

	putki::builder::free(builder);
	return 0;
}
