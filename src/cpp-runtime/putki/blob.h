#pragma once

#include "types.h"

namespace putki
{
	char* post_blob_load_string(const char **string, char *aux_beg, char *aux_end);
	
	// register app-type blob loaders.
	typedef char* (post_blob_load_t)(int type, putki::depwalker_i *, char *begin, char *end);
	void add_blob_loader(post_blob_load_t func);
	char* post_load_by_type(int type, depwalker_i *ptrwalker, char *begin, char *end);
}

