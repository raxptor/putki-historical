#ifndef __PUTKI_RESOLVE_H__
#define __PUTKI_RESOLVE_H__

#include <cstdlib>

namespace putki
{
	bool is_resolved_pointer(void *ptr);
	void* resolve_pointer();

	void resolver_alloc(size_t sizes);
	char *make_unresolved();
}

#endif
