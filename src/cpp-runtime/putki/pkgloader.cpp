#include "pkgloader.h"

#include <putki/config.h>
#include "log/log.h"

#include <fstream>
#include <iostream>

namespace putki
{
	namespace pkgloader
	{
		bool load_external_file(int file_index, const char *path, uint32_t beg, uint32_t end, void *target)
		{
			if (!path)
			{
				PTK_DEBUG("Got flush call")
				return true;
			}
			
			PTK_DEBUG("Reading from " << path << " beg:" << beg << " end:" << end << " target:" << target)
			
			
			char buf[1024];
			putki::format_package_path(path, buf);
			
			std::ifstream in(buf, std::ios::binary);
			in.seekg(beg, std::ios::beg);
			in.read((char*)target, end - beg);
			if (in.gcount() == (end-beg))
				return true;
				
			PTK_ERROR("Reading failed!");
			return false;
		}

		pkgmgr::loaded_package * from_file(const char *file)
		{
			char ptr[256];
			putki::format_package_path(file, ptr);
			strcat(ptr, ".ptr");

			std::ifstream ptrf(ptr);
			if (ptrf.good())
			{
				ptrf.read(ptr, 128);
				if (ptrf.gcount() >0)
				{
					ptr[ptrf.gcount()] = 0;
					file = ptr;
				}
				PTK_DEBUG("Redirecting " << file << " to " << ptr << " from ptr file")
				ptrf.close();
			}
		
			char buf[1024];
			putki::format_package_path(file, buf);
				
			// see if redirect exists
			std::ifstream redir(buf);
			
			

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
			
			pkgmgr::loaded_package *p = pkgmgr::parse(header, data, &load_external_file, 0);
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