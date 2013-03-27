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
        int *sz = (int*) where;
        pack_int32_field(where, len+1);
        
        if ((unsigned int)(aux_end - aux_beg) < (len+1))
            return 0;
        
        memcpy(aux_beg, src, len + 1);
        
        return aux_beg + len + 1;
    }

}

namespace outki
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
	
	char* post_load_by_type(int type, char *begin, char *end)
	{
		for (int i=0;i<modules;i++)
		{
			char *res = pbl[i](type, begin, end);
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
  