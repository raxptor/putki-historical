#ifndef __PUTKISYS_SSTREAM_H__
#define __PUTKISYS_SSTREAM_H__

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <string>

namespace putki
{

	// backwards writing
	template<int digits, typename T>
	struct format_dec_digit
	{
		static inline void run(char *out, T val)
		{
			*out = '0' + (val % 10);
			format_dec_digit<digits-1, T>::run(out-1, val / 10);
		}
	};

	template<typename T>
	struct format_dec_digit<1, T>
	{
		static inline void run(char *out, T val)
		{
			*out = '0' + (val % 10);
		}
	};

	template<class T>
	char* format_dec(char *out, T val)
	{
		if (val < 0)
		{
			*out++ = '-';
			return format_dec(out, 0-val);
		}
		
		if (!val)
		{
			*out++ = '0';
			return out;
		}

		if (val < 10)
		{
			*out++ = (char)('0' + val);
			return out;
		}
		else if (val < 100)
		{
			format_dec_digit<2, T>::run(out+1, val);
			return out + 2;
		}
		else if (val < 1000)
		{
			format_dec_digit<3, T>::run(out+2, val);
			return out + 3;
		}
		else if (val < 10000)
		{
			format_dec_digit<4, T>::run(out+3, val);
			return out + 4;
		}

		// generic impl
		
		char tmp[64];
		int dig = 0;
		while (val > 0)
		{
			T digit = val % 10;
			tmp[dig++] = (char)('0' + digit);
			val = val / 10;
		}
		
		for (int i=0;i<dig;i++)
			*out++ = tmp[dig-i-1];
		
		return out;
	}

	template<typename T>
	inline int hexdigit(T val, int digit)
	{
		return (val >> (sizeof(T)*2-digit-1) * 4) & 0xf;
	}
	
	template<typename T>
	char* format_hex(char *out, T val)
	{
		static const char sym[] = "0123456789abcdef";

		(*out++) = '0';
		(*out++) = 'x';

		if (val == 0)
		{
			(*out++) = '0';
			return out;
		}

		const int sym_out = sizeof(T) * 2;

		int i = 0;
		while (i != sym_out)
		{
			if (!hexdigit(val, i))
				++i;
			else
				break;
		}

		int j = 0;
		while (i != sym_out)
		{
			out[j] = sym[hexdigit(val, i)];
			++j; ++i;
		}
		
		return out + j;
	}

	struct sstream
	{
		char _static[256];
		unsigned int _len; // buffer length
		char *_buf, *_writeptr;
		
		sstream()
		{
			_buf = _static;
			_writeptr = _buf;
			_len = sizeof(_static);
		}
		
		~sstream()
		{
			if (_buf != _static)
			{
				free(_buf);
			}
		}
		
		inline void need_x_more(unsigned int more, unsigned int allocmult=4, unsigned int allocadd=256)
		{
			const unsigned int need = (_writeptr - _buf) + more;
			if (need > _len)
			{
				_len = _len * allocmult;

				// if multiple wasn't enough
				if (_len < need)
					_len = need;
					
				// always add extra on what is needed
				_len += allocadd;
				
				// copy
				char *n = (char*) malloc(_len);
				memcpy(n, _buf, _writeptr - _buf);
				if (_buf != _static)
					free(_buf);
				_writeptr = n + (_writeptr - _buf);
				_buf = n;
			}
		}
		
		size_t size()
		{
			return _writeptr - _buf;
		}
		
		void clear()
		{
			_writeptr = _buf;
		}
		
		const char *c_str()
		{
			// needs terminator.
			need_x_more(1, 0, 256);
			*_writeptr = 0;
			return _buf;
		}
		
		// for str().c_str() compatibility with std::stringstream
		sstream & str()
		{
			return *this;
		}

		template<typename T>
		inline sstream & hex(T val)
		{
			need_x_more(32);
			_writeptr = format_hex<T>(_writeptr, val);
			return *this;
		}
		
		inline sstream & operator<<(char c)
		{
			need_x_more(1);
			*_writeptr++ = c;
			return *this;
		}
		
		inline sstream & operator<<(const char *txt)
		{
			const unsigned int len = strlen(txt);
			need_x_more(len);
			memcpy(_writeptr, txt, len);
			_writeptr += len;
			return *this;
		}

		inline sstream & operator<<(void *ptr)
		{
			if (!ptr)
			{
				return *this << "<nullptr>";
			}
			else
			{
				need_x_more(24);
				_writeptr = format_hex<ptrdiff_t>(_writeptr, (ptrdiff_t)ptr);
				return *this;
			}
		}

		inline sstream & operator<<(std::string const & str)
		{
			*this << str.c_str();
			return *this;
		} 
		
		inline sstream & operator<<(int val)
		{
			need_x_more(32);
			_writeptr = format_dec<int>(_writeptr, val);
			return *this;
		}

		inline sstream & operator<<(unsigned int val)
		{
			need_x_more(32);
			_writeptr = format_dec<int>(_writeptr, val);
			return *this;
		}

		inline sstream & operator<<(unsigned long val)
		{
			need_x_more(32);
			_writeptr = format_dec<unsigned long>(_writeptr, val);
			return *this;
		}

		inline sstream & operator<<(long val)
		{
			need_x_more(32);
			_writeptr = format_dec<long>(_writeptr, val);
			return *this;
		}

		inline sstream & operator<<(long long val)
		{
			need_x_more(32);
			_writeptr = format_dec<long long>(_writeptr, val);
			return *this;
		}

		inline sstream & operator<<(unsigned long long val)
		{
			need_x_more(32);
			_writeptr = format_dec<unsigned long long>(_writeptr, val);
			return *this;
		}

		inline sstream & operator<<(float val)
		{
			char tmp[64];
			sprintf(tmp, "%.10f", val);
			return *this << tmp;;
		}
		
		private:
		
			sstream(const sstream & src)
			{
			}
			
			sstream& operator=(const sstream &src)
			{
				return *this;
			}
	};
}

#endif
