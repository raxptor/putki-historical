#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

// generated.
namespace inki
{
	void bind_ui_example();
	void bind_ccg_ui();
}

void ccg_ui_register_handlers(putki::builder::data *builder);

void app_register_handlers(putki::builder::data *builder)
{
	// all the ccg-ui stuff
	ccg_ui_register_handlers(builder);
}

void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf)
{
	/*
	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "screens/ingame", true);
		putki::build::commit_package(pkg, pconf, "static.pkg");
	} 
	*/
	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "tetris/gamescreen", true);		
		putki::build::commit_package(pkg, pconf, "tetris.pkg");
	}

	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "dialog/widget", true);
		putki::package::add(pkg, "dialog/containerscreen", true);
		putki::build::commit_package(pkg, pconf, "dialogs.pkg");
	}
}

int run_putki_builder(int argc, char **argv);

int main(int argc, char **argv)
{
	inki::bind_ccg_ui();
	inki::bind_ui_example();

	putki::builder::set_builder_configurator(&app_register_handlers);
	putki::builder::set_packager(&app_build_packages);

	return run_putki_builder(argc, argv);
}
