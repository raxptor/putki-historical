#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

namespace inki
{
	void bind_test();
}

void test_register_handlers(putki::builder::data *builder);

void app_register_handlers(putki::builder::data *builder)
{
	test_register_handlers(builder);
}

void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf)
{
/*
	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "", true);
		putki::build::commit_package(pkg, pconf, "static.pkg");
	}
*/
}

int run_putki_builder(int argc, char **argv);

int main(int argc, char **argv)
{
	inki::bind_test();
	putki::builder::set_builder_configurator(&app_register_handlers);
	putki::builder::set_packager(&app_build_packages);
	return run_putki_builder(argc, argv);
}
