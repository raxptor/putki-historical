#include <putki/config.h>

#include <cassert>
#include <map>
#include <string>
#include <fstream>
#include <cstdlib>

namespace putki
{
	namespace cfg
	{
		typedef std::map<std::string, std::string> CMap;
		typedef std::map<std::string, CMap> SMap;

		struct data
		{
			SMap s; // sections
		};

		namespace
		{
			struct pstate
			{
				std::string section;
			};

			void parse_row(data *d, pstate *s, const char *txt,  size_t length)
			{
				const char *beg = txt;
				const char *end = txt + length;

				// Remove beginnig and endig spaces.
				while (*beg == ' ' && beg != end) beg++;
				while (end > beg && end[-1] == ' ') end--;

				if (beg == end) {
					return;
				}

				// These are all way of making comments
				if (*beg == '#' || *beg == '/' || *beg == ';') {
					return;
				}

				// What row is this.
				if (beg[0] == '[' && end[-1] == ']')
				{
					s->section = std::string(beg + 1, end - beg - 2);
					return;
				}

				const std::string str(beg, end - beg);
				std::string::size_type pos = str.find_first_of("=");
				if (pos == std::string::npos)
				{

				}
				else
				{
					std::string pre(beg, beg + pos);
					while (!pre.empty() && pre[pre.size()-1] == ' ') pre.erase(pre.end()-1);

					std::string post(beg + pos + 1, end);
					while (!post.empty() && post[0] == ' ') post.erase(post.begin());

					d->s[s->section][pre] = post;
				}
			}

			void parse_it(data *d, const char *txt, size_t sz)
			{
				pstate s;

				const char *lstart = 0;
				for (size_t i=0; i!=sz; i++)
				{
					if (txt[i] == 0xD || txt[i] == 0xA)
					{
						if (lstart)
						{
							parse_row(d, &s, lstart, (&txt[i]) - lstart);
							lstart = 0;
						}
					}
					else if (!lstart) {
						lstart = &txt[i];
					}
				}

				if (lstart) {
					parse_row(d, &s, lstart, txt+sz-lstart);
				}
			}
		}

		data* load(const char *filename)
		{
			data *d = new data();

			std::ifstream f(filename);
			std::string row;
			pstate s;
			while (getline(f, row))
			{
				parse_row(d, &s, row.c_str(), row.size());
			}
			return d;
		}

		data* merge(data *first, data *second)
		{
			data *d = new data();

			for (int f=0; f<2; f++)
			{
				data *r = !f ? first : second;

				SMap::const_iterator i = r->s.begin();
				while (i != r->s.end())
				{
					CMap::const_iterator j = i->second.begin();
					while (j != i->second.end())
					{
						d->s[i->first][j->first] = j->second;
						j++;
					}
					++i;
				}
			}

			return d;
		}

		void free(data *d)
		{
			if (d) {
				delete d;
			}
		}

		int get_int(data *d, const char *section, const char *key, int def)
		{
			const char *s = get_string(d, section, key, 0);
			if (!s) {
				return def;
			}
			else{
				return atoi(s);
			}
		}

		float get_float(data *d, const char *section, const char *key, float def)
		{
			const char *s = get_string(d, section, key, 0);
			if (!s) {
				return def;
			}
			else{
				return (float)atof(s);
			}
		}

		const char* get_string(data *d, const char *section, const char *key, const char *def)
		{
			SMap::const_iterator i = d->s.find(section);
			if (i == d->s.end()) {return def; }
			CMap::const_iterator j = i->second.find(key);
			if (j == i->second.end()) {return def; }

			return j->second.c_str();
		}

	}

}