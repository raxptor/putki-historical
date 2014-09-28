#include "pkgloader.h"

#include <putki/config.h>
#include "log/log.h"

#include <fstream>
#include <iostream>

namespace putki
{
	namespace pkgloader
	{
		pkgmgr::loaded_package * from_file(const char *file)
		{
			char buf[1024];
			putki::format_package_path(file, buf);

			std::ifstream in(buf, std::ios::binary);
			if (!in.good())
			{
				PTK_ERROR("Failed to open file [" << buf << "]!")
				return 0;
			}
			
			char header_peek[16];
			in.read(header_peek, sizeof(header_peek));
			
			uint32_t hdr_size, data_size;
			if (!pkgmgr::get_header_info(header_peek, header_peek + sizeof(header_peek), &hdr_size, &data_size))
			{
				PTK_ERROR("Header could not be parsed in " << file)
				return 0;
			}
			
			char *header = new char[(unsigned long) hdr_size];
			char *data = new char[(unsigned long) data_size];
			
			// -- pick out header --
			in.seekg(0, std::ios::end);
			unsigned long readsize = in.tellg();
			
			in.seekg(0, std::ios::beg);
			in.read(header, hdr_size);
			in.read(data, readsize - hdr_size);
			in.close();
			
			pkgmgr::loaded_package *p = pkgmgr::parse(header, data, 0);
			if (!p)
			{
				delete [] header;
				delete [] data;
			}
			else
			{
				delete [] header;
				// assume it's been wholly resolved.
				pkgmgr::register_for_liveupdate(p);
				pkgmgr::free_on_release(p);
			}

			return p;
		}
	}
}