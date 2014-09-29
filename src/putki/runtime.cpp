#include "runtime.h"

#include <cstring>
#include <cstdlib>

namespace putki
{

	namespace runtime
	{
		const char * desc_str(desc const * rt)
		{
			static char buf[256];
			switch (rt->platform)
			{
				case PLATFORM_32BIT:
					strcpy(buf, "x");
					break;
				case PLATFORM_64BIT:
					strcpy(buf, "x");
					break;
				case PLATFORM_CSHARP:
					strcpy(buf, "csharp");
					break;
				default:
					strcpy(buf, "unknown");
					break;
			}

			if (buf[0] == 'x')
			{
				if (rt->ptrsize == 8)
					strcat(buf, "64");
				else if (rt->ptrsize == 4)
					strcat(buf, "32");
				else if (rt->ptrsize == 2)
					strcat(buf, "16");
			}

			return buf;
		}

		const desc * get(unsigned int index)
		{
			// if you change this table, re-build the compiler and re-compile everything too.
			static const int count = 3;
			static const desc rtd[count] = {
				{PLATFORM_64BIT,   LANGUAGE_CPP,     8,     1,     true       },
				{PLATFORM_32BIT,   LANGUAGE_CPP,     4,     1,     true       },
				// We use 16-bit pointers here!
				{PLATFORM_CSHARP,  LANGUAGE_CSHARP,  2,     1,     true       }
			};

			if (index < count) {
				return &rtd[index];
			}
			return 0;
		}

		const desc * running()
		{
			static desc rt;
			rt.platform = platform();
			rt.ptrsize = sizeof(void*);
			rt.boolsize = sizeof(bool);
			rt.low_byte_first = true; // todo: fix
			return &rt;
		}
	}
}
