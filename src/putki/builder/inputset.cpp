#include "inputset.h"

#include <string>
#include <iostream>

namespace putki
{
	namespace inputset
	{
		struct data
		{
			std::string basepath;
			std::string dbfile;
		};
		
		data *open(const char *basepath, const char *dbfile)
		{
			data *d = new data();
			d->basepath = basepath;
			d->dbfile = dbfile;

			std::cout << "Input set [" << basepath << "] tracked in [" << dbfile << "]" << std::endl;
			return d;
		}

		void release(data *d)
		{
			delete d;
		}

		void get_record(unsigned int index, manifest_record *out)
		{

		}
		
		bool get_record(const char *path, manifest_record *out)
		{
			return false;
		}
	}
}
