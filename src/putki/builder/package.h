#pragma once

#include <putki/runtime.h>
#include <putki/sys/sstream.h>

namespace putki
{
	namespace db { struct data; }
	namespace build_db { struct data; }

	namespace package
	{
		struct data;

		data * create(db::data *db);
		void free(data *);

		// need storepath = true to be able to look it up from the package in runtime.
		void add(package::data *data, const char *path, bool storepath);
		const char *get_needed_asset(data *d, unsigned int i);
		
		void add_previous_package(package::data *data, const char *basepath, const char *path);
		
		long write(data *data, runtime::descptr rt, char *buffer, long available, build_db::data *build_db, sstream & manifest);
	}
}
