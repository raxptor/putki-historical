#pragma once

#include <putki/runtime.h>

namespace putki
{
	namespace db { struct data; }
	namespace build_db { struct data; }

	namespace package
	{
		struct data;

		data * create(db::data *source);
		void free(data *);

		// need storepath = true to be able to look it up from the package in runtime.
		void add(package::data *data, const char *path, bool storepath);
		const char *get_needed_asset(data *d, unsigned int i);
		
		void debug(package::data *data, build_db::data *bdb);

		long write(data *data, runtime::descptr rt, char *buffer, long available);
	}
}
