#include "pkgloader.h"
#include <fstream>
#include <iostream>

namespace putki
{
	namespace pkgloader
	{
		const char *platform_path()
		{
			#if defined(_WIN32)
				if (sizeof(void*) == 4)
					return "win32";
				else
					return "win64";
			#elif defined(__APPLE__) && defined(__amd64__)
				if (sizeof(void*) == 4)
					return "macosx32";
				else
					return "macosx64";
			#else
				return "unknown_platform";
			#endif
		}

		pkgmgr::loaded_package * from_file(const char *file)
		{
			char buf[1024];
			sprintf(buf, "out/%s/packages/%s", platform_path(), file);

			std::ifstream in(buf, std::ios::binary);
			if (!in.good())
			{
				std::cout << "Failed to open file [" << buf << "]!" << std::endl;
				return 0;
			}

			in.seekg(0, std::ios::end);
			std::streamoff size = in.tellg();

			char *buffer = new char[(unsigned long)size];
			in.seekg(0, std::ios_base::beg);
			in.read(buffer, size);
			in.close();
			
			std::cout << "Read " << size << " bytes from [" << file << "]" << std::endl;

			pkgmgr::loaded_package *p = pkgmgr::parse(buffer, buffer + size, 0);
			if (!p)
			{
				delete [] buffer;
			}
			else
			{
				// assume it's been wholly resolved.
				pkgmgr::register_for_liveupdate(p);
				pkgmgr::free_on_release(p);
			}

			return p;
		}
	}
}