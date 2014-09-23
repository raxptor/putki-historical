#pragma once

namespace putki
{
	inline char *pack_int16_field(char *where, short val)
	{
		where[0] = (val) & 0xff;
		where[1] = (val >> 8) & 0xff;
		return where + 2;
	}

	inline char *pack_int32_field(char *where, int val)
	{
		where[0] = (val) & 0xff;
		where[1] = (val >> 8) & 0xff;
		where[2] = (val >> 16) & 0xff;
		where[3] = (val >> 24) & 0xff;
		return where + 4;
	}

	char *pack_string_field(char *where, const char *src, char *aux_beg, char *aux_end);
}