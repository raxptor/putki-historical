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
				default:
					strcpy(buf, "unknown");
					break;
			}

			if (rt->ptrsize == 8)
				strcat(buf, "64");
			else 
				strcat(buf, "32");

			return buf;
		}

		const desc * get(unsigned int index)
		{
			static const int count = 4;
			static const desc rtd[count] = {
				{PLATFORM_MACOSX, LANGUAGE_CPP, 8, true},
				{PLATFORM_WINDOWS, LANGUAGE_CPP, 4, true},
				{PLATFORM_WINDOWS, LANGUAGE_CPP, 8, true},
				{PLATFORM_CSHARP, LANGUAGE_CSHARP, 4, true}
			};

			if (index < count)
				return &rtd[index];
			return 0;
		}

		const desc * running()
		{
			static desc rt;
			rt.platform = platform();
			rt.ptrsize = sizeof(void*);
			rt.low_byte_first = true; // todo: fix
			return &rt;
		}
	}

}
