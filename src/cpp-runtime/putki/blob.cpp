#include "blob.h"

#include <cstdlib>
#include <memory>
#include <cstring>
#include <iostream>

namespace putki
{
	typedef unsigned short strsize_t;

	namespace
	{
		const int max_modules = 16;
		int modules = 0;
		post_blob_load_t* pbl[max_modules];
	}

	void add_blob_loader(post_blob_load_t func)
	{
		if (modules < max_modules) {
			pbl[modules++] = func;
		}
	}

	char* post_load_by_type(int type, depwalker_i *ptrwalker, char *begin, char *end)
	{
		for (int i=0; i<modules; i++)
		{
			char *res = pbl[i](type, ptrwalker, begin, end);
			if (res) {
				return res;
			}
		}
		return 0;
	}

	char* post_blob_load_string(const char **string, char *aux_beg, char *aux_end)
	{
		if (!aux_beg) {
			return 0;
		}

		unsigned int *lptr = (unsigned int*) string;
		unsigned int len = *lptr;

		*string = "<UNPACK FAIL>";

		if (aux_beg + len <= aux_end)
		{
			const char *last = aux_beg + len - 1;
			if (*last == 0x00) {
				*string = &aux_beg[0];
			}
			else{
				*string = "<UNPACK LAST NON-ZERO>";
			}

			aux_beg += len;
		}
		else
		{
			std::cout << "not enough bytes in stream" << std::endl;
			return 0;
		}
		return aux_beg;
	}
}

