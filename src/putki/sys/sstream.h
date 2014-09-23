#ifndef __PUTKISYS_SSTREAM_H__
#define __PUTKISYS_SSTREAM_H__

#include <string>
#include <cstdio>

namespace putki
{
	struct sstream
	{
		std::string buf;
		
		std::string const & str() const
		{
			return buf;
		}
		
		inline sstream & operator<<(char c)
		{
			buf.push_back(c);
			return *this;
		}
		
		inline sstream & operator<<(const char *txt)
		{
			buf.append(txt);
			return *this;
		}

		inline sstream & operator<<(void *ptr)
		{
			if (!ptr)
				buf.append("<nullptr>");
			else
				*this << ((long long)ptr);
			return *this;
		}

		inline sstream & operator<<(std::string const & str)
		{
			buf.append(str);
			return *this;
		} 
		
		inline sstream & operator<<(int val)
		{
			char tmp[64];
			sprintf(tmp, "%d", val);
			buf.append(tmp);
			return *this;
		}

		inline sstream & operator<<(unsigned int val)
		{
			char tmp[64];
			sprintf(tmp, "%u", val);
			buf.append(tmp);
			return *this;
		}

		inline sstream & operator<<(unsigned long val)
		{
			char tmp[64];
			sprintf(tmp, "%lu", val);
			buf.append(tmp);
			return *this;
		}

		inline sstream & operator<<(long val)
		{
			char tmp[64];
			sprintf(tmp, "%ld", val);
			buf.append(tmp);
			return *this;
		}

		inline sstream & operator<<(long long val)
		{
			char tmp[64];
			sprintf(tmp, "%ld", (long)val);
			buf.append(tmp);
			return *this;
		}

		inline sstream & operator<<(float val)
		{
			char tmp[64];
			sprintf(tmp, "%.10f", val);
			buf.append(tmp);
			return *this;
		}
	};
}

#endif
