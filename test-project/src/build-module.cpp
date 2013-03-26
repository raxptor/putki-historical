#include <iostream>
#include <putki/blob.h>
#include <putki/builder/typereg.h>
#include <putki/builder/db.h>

#include <putki/types/core.h>
#include <outki/types/core.h>

#include <fstream>

namespace putki
{
	void bind_test_project(); // generated impl.
}

void do_build_steps();

void bind_app_types()
{
	// generated function for this module
	putki::bind_test_project();
}

void bind_transforms()
{

}

void app_on_db_loaded(putki::db::data *db)
{
	std::cout << "Source object db loaded!" << std::endl;
}

int app_builder_main()
{
	putki::i_type_handler *handler = putki::typereg_get_handler("difficulty_setting");


	putki::difficulty_setting *ds = new putki::difficulty_setting();

	ds->level = 10;
	ds->name = "hej";

	do_build_steps();

	return 0;
}

