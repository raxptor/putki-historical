#include "resource.h"

#include <putki/builder/builder.h>
#include <putki/sys/files.h>

#include <fstream>
#include <iostream>
#include <string>

namespace putki
{
	namespace resource
	{
		bool load(builder::data *bld, const char *path, const char **outBytes, long long *outSize)
		{
			std::string full_path;
			const char *ps = path;

			if (path[0] == '%')
			{
				full_path = builder::tmp_path(bld);
				ps++;
			}
			else
			{
				full_path = builder::res_path(bld);
			}
			
			full_path.append("/");
			full_path.append(path);
			std::cout << "I want to load [" << full_path << "]!" << std::endl;
			return false;
		}

		std::string save_temp(builder::data *builder, const char *path, const char *bytes, long long length)
		{
			std::string out_path = std::string(builder::tmp_path(builder)) + "/" + path;
			sys::mk_dir_for_path(out_path.c_str());

			std::ofstream f(out_path.c_str(), std::ios::binary);
			f.write(bytes, length);
			return std::string("%") + path;
		}
	}
}
