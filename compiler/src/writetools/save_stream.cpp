#include "save_stream.h"

#include <putki/sys/files.h>

#include <fstream>
#include <iostream>
#include <cstring>

namespace putki
{
	void save_stream(std::string path, std::stringstream &str)
	{
		char *tmp = new char[str.str().size()];
		std::ifstream p(path.c_str());
		if (p.good())
		{
			p.read(tmp, str.str().size());
			if (p.gcount() == str.str().size())
			{
				if (!memcmp(tmp, str.str().c_str(), str.str().size()))
				{
					std::cout << "Identical, skipping: " << path << std::endl;
					return;
				}
			}
		}
		p.close();
		delete [] tmp;

		putki::sys::mk_dir_for_path(path.c_str());
		std::ofstream f(path.c_str());
		f << str.str();
		std::cout << "Wrote: " << path << std::endl;
	}
}