#pragma once

#include <putki/runtime.h>

namespace putki
{
	namespace db { struct data; }
	
	namespace package
	{
		struct data;
		
		data * create(db::data *source);
		void free(data *);
	
		// need storepath = true to be able to look it up from the package in runtime.
		void add(package::data *data, const char *path, bool storepath);

		long inbytes(data *d);
		long write(data *data, putki::runtime rt, char *buffer, long available);
	}
}
