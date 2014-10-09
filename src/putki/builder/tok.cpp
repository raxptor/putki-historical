#include <fstream>
#include <vector>

#include "resource.h"
#include "log.h"

namespace putki
{
	namespace tok
	{
		struct data
		{
			char *buf;
			size_t size;
			std::vector<const char*> ptrs;
		};
		
		data* load(const char *fn)
		{
			std::ifstream f(fn, std::ios::binary);
			if (!f.good())
			{
				APP_WARNING("Failed to load resource [" << fn << "]")
				return 0;
			}

			f.seekg(0, std::ios::end);
			std::streampos size = f.tellg();
			f.seekg(0, std::ios::beg);
			data *d = new data();
			
			d->buf = new char[(size_t)size+1];
			d->size = (size_t) size;
			
			f.read(d->buf, size);
			
			if (f.gcount() != size)
				APP_WARNING("did not read the whole file");

			return d;
		}
		
		void free(data *d)
		{
			delete [] d->buf;
			delete d;
		}
		
		void tokenize_newlines(data *d)
		{
			char *buf = d->buf;
			int start = 0;
			for (int i=0;i!=(d->size+1);i++)
			{
				if (i == d->size || buf[i] == 0xd || buf[i] == 0xa)
				{
					buf[i] = 0;
					if (i - start > 0)
						d->ptrs.push_back(&buf[start]);
						
					start = i + 1;
				}
			}
		}
		
		int size(data *d)
		{
			return d->ptrs.size();
		}
		
		const char *get(data *d, unsigned int index)
		{
			if (index < d->ptrs.size())
				return d->ptrs[index];
			return 0;
		}
	}
}

