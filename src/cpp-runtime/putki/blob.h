#pragma once

#include "types.h"

namespace outki
{
	char* post_blob_load_string(const char **string, char *aux_beg, char *aux_end);
	void prep_int16_field(char *where);
	void prep_int32_field(char *where);
	
	// register app-type blob loaders.
	typedef char* (post_blob_load_t)(int type, depwalker_i *, char *begin, char *end);
	void add_blob_loader(post_blob_load_t func);
	char* post_load_by_type(int type, depwalker_i *ptrwalker, char *begin, char *end);
}

namespace putki
{
    char *pack_int16_field(char *where, short val);
    char *pack_int32_field(char *where, int val);
    char *pack_string_field(char *where, const char *src, char *aux_beg, char *aux_end);
}
