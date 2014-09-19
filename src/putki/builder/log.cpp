#include <iostream>
#include <unistd.h>
#include <sstream>

#include <putki/builder/log.h>
#include <putki/sys/thread.h>

namespace putki
{
	namespace
	{
		bool tty_out = isatty(1);
		sys::mutex mtx;
	}

	void print_log(LogType level, const char *indent, const char *message)
	{
		std::stringstream buf;

		if (tty_out)
			buf << "\033[34m";

		buf << indent;

		if (tty_out)
			buf << "\033[0m";

		buf << " => ";
		
		if (tty_out)
		{
			switch (level)
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
		buf << message;
		if (tty_out)
		{
			buf << "\033[0m";
		}
		
		mtx.lock();
		std::cout << buf.str() << std::endl;
		mtx.unlock();

		if (level == LOG_ERROR)
		{
			int *q = (int*)0x1234;
			*q = 0x4321;
		}

	}

	void print_blob(const char *message)
	{
		std::cout << "blob:" << message << std::endl;
	}

	bool check_filter(LogType level)
	{
//		if (level == LOG_DEBUG)
//			return false;
		return true;
	}
}
