#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/typereg.h>
#include <putki/builder/write.h>
#include <putki/sys/sstream.h>
#include <putki/builder/db.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/runtime.h>
#include <stdint.h>
#include <iostream>

void make_data(const char *name)
{
	putki::type_handler_i *type = putki::typereg_get_handler(name);
	if (!type) {
		std::cerr << "No type with name '" << name << "'" << std::endl;
		return;
	}

	putki::instance_t obj = type->alloc();

	const char *path = "tmp_path";
	putki::db::data *db = putki::db::create();
	putki::db::insert(db, path, type, obj);

	putki::sstream tmp;
	putki::write::write_object_into_stream(tmp, db, type, obj);
	std::cout << tmp.c_str();

	putki::db::free_and_destroy_objs(db);
}

int run_putki_datatool(int argc, char **argv)
{
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "make")) {
			if (i+1 < argc)
				make_data(argv[i+1]);
			else
				std::cerr << "Syntax: make <type>" << std::endl;
		} else if (!strcmp(argv[i], "list-types")) {
			for (unsigned int i=0; true; i++) {
				putki::type_handler_i *type = putki::typereg_get_handler_by_index(i);
				if (!type) {
					std::cout << "Total: " << i << " types." << std::endl;
					break;
				} else {
					std::cout << type->name() << std::endl;
				}
			}
		}
	}
	return 0;
}
