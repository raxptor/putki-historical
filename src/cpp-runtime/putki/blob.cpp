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
		if (modules < max_modules)
			pbl[modules++] = func;
			
		std::cout << "Got " << modules << " post loaders" << std::endl;
	}
	
	char* post_load_by_type(int type, depwalker_i *ptrwalker, char *begin, char *end)
	{
		for (int i=0;i<modules;i++)
		{
			char *res = pbl[i](type, ptrwalker, begin, end);
			if (res)
				return res;
		}
		return 0;
	}
	
	bool need_swap()
	{
		char b[] = {1, 0};
		return *reinterpret_cast<short*>(b) != 1;
	}
	
	void prep_int16_field(char *where)
	{
		if (need_swap())
		{
			char tmp = where[0];
			where[0] = where[1];
			where[1] = tmp;
		}
	}

	void prep_int32_field(char *where)
	{
		if (need_swap())
		{
			char tmp = where[0];
			where[0] = where[3];
			where[3] = tmp;
		
			tmp = where[1];
			where[1] = where[2];
			where[2] = tmp;
		}
	}
	
	char* post_blob_load_string(const char **string, char *aux_beg, char *aux_end)
	{
		if (!aux_beg)
			return 0;
		
		prep_int32_field((char*) string);
		unsigned int *lptr = (unsigned int*) string;
		unsigned int len = *lptr;
		
		*string = "<UNPACK FAIL>";

		if (aux_beg + len <= aux_end)
		{
			const char *last = aux_beg + len - 1;
			if (*last == 0x00)
				*string = &aux_beg[0];
			else
				*string = "<UNPACK LAST NON-ZERO>";
			
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
  