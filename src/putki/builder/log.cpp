#include <iostream>
#include <stdint.h>
#include <sstream>

#include <putki/builder/log.h>
#include <putki/sys/thread.h>
#include <putki/sys/sstream.h>

#if defined(_WIN32)
bool use_ansi_color = false;
#else
#include <unistd.h>
bool use_ansi_color = isatty(1);
#endif

namespace putki
{
	namespace
	{
		LogType loglevel = LOG_INFO;
		sys::mutex mtx;
	}

	void set_loglevel(LogType level)
	{
		loglevel = level;
	}
	
	void set_use_ansi_color(bool enabled)
	{
		use_ansi_color = enabled;
	}

	bool check_filter(LogType level)
	{
		return level >= loglevel;
	}
	
	void print_log(const char *indent, LogType level, const char *message)
	{
		print_log_multi(indent, &level, &message, 1);
	}

	void print_log_multi(const char *indent, LogType *levels, const char **messages, unsigned int count)
	{
		sstream buf;

		for (unsigned int i=0;i!=count;i++)
		{
			if (use_ansi_color)
			{
				buf << "\033[34m";
			}

			buf << indent;

			if (use_ansi_color)
			{
				buf << "\033[0m";
			}

			buf << " => ";
			
			if (use_ansi_color)
			{
				switch (levels[i])
				{
					case LOG_INFO:
						buf << "\033[0m";
						break;
					case LOG_ERROR:
						buf << "\033[31m";
						break;
					case LOG_WARNING:
						buf << "\033[35m";
						break;
					default:
					case LOG_DEBUG:
						buf << "\033[32m";
						break;
				}
			}
			
			buf << messages[i];
			
			if (use_ansi_color)
			{
				buf << "\033[0m";
			}
			
			if (levels[i] == LOG_ERROR)
			{
				// flush.
				mtx.lock();
				std::cout.write(buf.c_str(), buf.size());
				std::cout << std::endl;
				std::cout.flush();
				mtx.unlock();
				// crash
				int *p = (int *) 0x23414;
				*p = 234124;
			}
			
			buf << "\n";
		}
		
		mtx.lock();
		std::cout.write(buf.c_str(), buf.size());
		std::cout.flush();
		mtx.unlock();
	}
}
