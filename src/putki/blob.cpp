#include "blob.h"
#include "builder/log.h"

#include <cstdlib>
#include <memory>
#include <cstring>
#include <iostream>

namespace putki
{
	char *pack_string_field(int size_size, char *where, const char *src, char *aux_beg, char *aux_end)
	{
		if (!aux_beg) {
			return 0;
		}

		unsigned int len = strlen(src);

		// write the length into the pointer slot.
		if (size_size == 8)
			pack_int64_field(where, len+1);
		else if (size_size == 4)
			pack_int32_field(where, len+1);
		else if (size_size == 2)
			pack_int16_field(where, len+1);
		else if (size_size == 1)
			*where = len + 1;
		else
			APP_ERROR("Invalid size_size=" << size_size);
		

		if ((unsigned int)(aux_end - aux_beg) < (len+1)) {
			return 0;
		}

		memcpy(aux_beg, src, len + 1);

		return aux_beg + len + 1;
	}
}
