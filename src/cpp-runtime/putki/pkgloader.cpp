#include "pkgloader.h"
#include <fstream>

namespace outki
{
	namespace pkgloader
	{
		pkgmgr::loaded_package * from_file(const char *file)
		{
			std::ifstream in(file, std::ios::binary);
			if (!in.good())
				return 0;

			in.seekg(0, std::ios::end);
			std::streamoff size = in.tellg();

			char *buffer = new char[(unsigned long)size];
			in.seekg(0, std::ios_base::beg);
			in.read(buffer, size);
			in.close();

			pkgmgr::loaded_package *p = pkgmgr::parse(buffer, buffer + size);
			if (!p)
			{
				delete [] buffer;
			}
			else
			{
				pkgmgr::free_on_release(p);
			}

			return p;
		}
	}
}