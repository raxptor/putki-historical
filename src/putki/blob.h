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

	inline char *pack_int64_field(char *where, long long val)
	{
		where[0] = (val) & 0xff;
		where[1] = (val >> 8) & 0xff;
		where[2] = (val >> 16) & 0xff;
		where[3] = (val >> 24) & 0xff;
		where[4] = (val >> 32) & 0xff;
		where[5] = (val >> 40) & 0xff;
		where[6] = (val >> 48) & 0xff;
		where[7] = (val >> 56) & 0xff;
		return where + 8;
	}

	char *pack_string_field(int size_size, char *where, const char *src, char *aux_beg, char *aux_end);
}