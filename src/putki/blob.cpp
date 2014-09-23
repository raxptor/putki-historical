#include "blob.h"

#include <cstdlib>
#include <memory>
#include <cstring>
#include <iostream>

namespace putki
{
	char *pack_string_field(char *where, const char *src, char *aux_beg, char *aux_end)
	{
		if (!aux_beg) {
			return 0;
		}

		unsigned int len = strlen(src);

		// write the length into the pointer slot.
		pack_int32_field(where, len+1);

		if ((unsigned int)(aux_end - aux_beg) < (len+1)) {
			return 0;
		}

		memcpy(aux_beg, src, len + 1);

		return aux_beg + len + 1;
	}
}
