#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

// generated.
namespace inki
{
	void bind_ccg_ui();
}

//
void register_font_builder(putki::builder::data *builder);


void app_register_handlers(putki::builder::data *builder)
{
	register_font_builder(builder);
}

void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf)
{
	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "description", true);
		putki::package::add(pkg, "widget", true);
		putki::build::commit_package(pkg, pconf, "static.pkg");
	}
}

int run_putki_builder(int argc, char **argv);

int main(int argc, char **argv)
{
	inki::bind_ccg_ui();

	putki::builder::set_builder_configurator(&app_register_handlers);
	putki::builder::set_packager(&app_build_packages);

	return run_putki_builder(argc, argv);
}
