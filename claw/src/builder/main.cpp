#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

#include <inki/types/claw/tiles.h>


// generated.
namespace putki
{
	void bind_claw();
}

void app_register_handlers(putki::builder::data *builder)
{

}

void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf)
{
	{
		putki::package::data *pkg = putki::package::create(out);
		putki::package::add(pkg, "globalsettings", true);
		putki::build::commit_package(pkg, pconf, "static.pkg");
	}
}

int databuilder_main(int argc, char **argv)
{
	return run_putki_builder(argc, argv);
}
