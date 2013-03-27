#include <iostream>
#include <putki/blob.h>
#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <putki/builder/package.h>

#include <putki/types/core.h>
#include <putki/types/test.h>

#include <fstream>

namespace putki
{
	void bind_test_project(); // generated impl.
}

// builder api.
void do_build_steps();
void commit_package(putki::package::data *package, const char *out_path);

void app_bind_types()
{
	putki::bind_test_project();
}

void app_on_db_loaded(putki::db::data *db)
{
	std::cout << "Source object db loaded!" << std::endl;

	putki::type_handler_i *th;
	putki::testobj *to;
	if (putki::db::fetch(db, "haspointer", &th, (putki::instance_t*) &to))
	{
		std::cout << "loaded it [" << to->curken << "]" << std::endl;
	}
}

void app_build_packages(putki::db::data *out)
{
	putki::package::data *p = putki::package::create(out);
	putki::package::add(p, "bleh", true);
	putki::package::add(p, "description", true);
	putki::package::add(p, "haspointer", true);
	commit_package(p, "everything.pkg");
}

void bind_transforms()
{

}

int app_builder_main()
{
	putki::type_handler_i *handler = putki::typereg_get_handler("difficulty_setting");

	do_build_steps();

	return 0;
}

