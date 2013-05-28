#pragma once

#include "types.h"

namespace outki
{	
	namespace pkgmgr
	{
		struct loaded_package;

		// parse from buffer, takes ownership.
		loaded_package * parse(char *beg, char *end);
		void free_on_release(loaded_package *);
		void release(loaded_package *);		

		// resolev from package.
		instance_t resolve(loaded_package *, const char *path);
	}
}
