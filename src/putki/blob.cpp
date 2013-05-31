#include "blob.h"

#include <cstdlib>
#include <memory>
#include <cstring>
#include <iostream>

namespace putki
{
	char *pack_int16_field(char *where, short val)
	{
		where[0] = (val) & 0xff;
		where[1] = (val >> 8) & 0xff;
		return where + 2;
	}
	
	char *pack_int32_field(char *where, int val)
	{
		where[0] = (val) & 0xff;
		where[1] = (val >> 8) & 0xff;
		where[2] = (val >> 16) & 0xff;
		where[3] = (val >> 24) & 0xff;
		return where + 4;
	}
	
	char *pack_string_field(char *where, const char *src, char *aux_beg, char *aux_end)
	{
		if (!aux_beg)
			return 0;
		
		unsigned int len = strlen(src);
  
		// write the length into the pointer slot.
		pack_int32_field(where, len+1);
		
		if ((unsigned int)(aux_end - aux_beg) < (len+1))
			return 0;
		
		memcpy(aux_beg, src, len + 1);
		
		return aux_beg + len + 1;
	}
}
