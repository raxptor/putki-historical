#pragma once

#include "types.h"

namespace putki
{
	namespace pkgmgr
	{
		struct loaded_package;
		struct resolve_status;

		// parse from buffer, takes ownership.
		// if opt_out is passed in, it will be filled with resolve stauts.
		loaded_package * parse(char *beg, char *end, resolve_status *opt_out);
		void free_on_release(loaded_package *);
		void release(loaded_package *);

		// must be resolved & done.
		void register_for_liveupdate(loaded_package *);

		resolve_status *alloc_resolve_status();
		void free_resolve_status(resolve_status *);

		// returns number of unresolved pointers remaining.
		int resolve_pointers_with(loaded_package *target, resolve_status *s, loaded_package *aux);

		// resolev from package.
		instance_t resolve(loaded_package *, const char *path);
		const char *path_in_package_slot(loaded_package *, unsigned int slot);
		const char *unresolved_reference(loaded_package *, unsigned int index);
	}
}
