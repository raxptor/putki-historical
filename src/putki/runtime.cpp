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
				case PLATFORM_MACOSX:
					strcpy(buf, "macosx");
					break;
				case PLATFORM_WINDOWS:
					strcpy(buf, "win");
					break;
				case PLATFORM_CSHARP:
					strcpy(buf, "csharp");
					break;
				case PLATFORM_UNIXY:
					strcpy(buf, "unixy");
					break;
				default:
					strcpy(buf, "unknown");
					break;
			}

			if (rt->ptrsize == 8) {
				strcat(buf, "64");
			}
			else{
				strcat(buf, "32");
			}

			return buf;
		}

		const desc * get(unsigned int index)
		{
			// if you change this table, re-build the compiler and re-compile everything too.
			static const int count = 7;
			static const desc rtd[count] = {
<<<<<<< HEAD
				{PLATFORM_MACOSX,  LANGUAGE_CPP,     8,     1,     true       },
				{PLATFORM_MACOSX,  LANGUAGE_CPP,     4,     1,     true       },
				{PLATFORM_LINUX,   LANGUAGE_CPP,     8,     1,     true       },
				{PLATFORM_LINUX,   LANGUAGE_CPP,     4,     1,     true       },
				{PLATFORM_WINDOWS, LANGUAGE_CPP,     4,     1,     true       },
				{PLATFORM_WINDOWS, LANGUAGE_CPP,     8,     1,     true       },
				{PLATFORM_CSHARP,  LANGUAGE_CSHARP,  4,     1,     true       }
=======
				{PLATFORM_MACOSX, LANGUAGE_CPP, 8, 1, true},
				{PLATFORM_MACOSX, LANGUAGE_CPP, 4, 1, true},
				{PLATFORM_UNIXY, LANGUAGE_CPP, 8, 1, true},
				{PLATFORM_UNIXY, LANGUAGE_CPP, 4, 1, true},
				{PLATFORM_WINDOWS, LANGUAGE_CPP, 4, 1, true},
				{PLATFORM_WINDOWS, LANGUAGE_CPP, 8, 1, true},
				{PLATFORM_CSHARP, LANGUAGE_CSHARP, 4, 1, true}
>>>>>>> 7182c4a... - Merged platforms to UNIXY (for freebsd,linux now)
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
