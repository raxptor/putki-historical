#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

// generated.
namespace inki
{
	void bind_claw();
}

void app_register_handlers(putki::builder::data *builder)
{
	/*
	static blob_handler bh;
	add_data_builder(builder, "gurka", 0xff, &bh);
	*/
}

void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf)
{
	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "globalsettings", true);
		putki::build::commit_package(pkg, pconf, "static.pkg");
	}

	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "ui/mainmenu", true);
		putki::build::commit_package(pkg, pconf, "mainmenu.pkg");
	}

}

int run_putki_builder(int argc, char **argv);

int main(int argc, char **argv)
{
	chdir("/Users/dannilsson/git/putki/claw");
	inki::bind_claw();

	putki::builder::set_builder_configurator(&app_register_handlers);
	putki::builder::set_packager(&app_build_packages);

	return run_putki_builder(argc, argv);
}
