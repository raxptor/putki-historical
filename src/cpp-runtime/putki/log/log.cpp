#include <iostream>
#include "log.h"

namespace putki
{
	namespace
	{
		LogType s_level = LOG_WARNING;
	}

	void set_loglevel(LogType filter)
	{
		s_level = filter;
	}

	void log(LogType type, const char *msg)
	{
		if (type < s_level)
			return;
		
		switch (type)
		{
			case LOG_DEBUG:
				std::cout << "debug: " << msg << std::endl;
				break;
			case LOG_WARNING:
				std::cout << "warn:  " << msg << std::endl;
				break;
			case LOG_ERROR:
				std::cerr << "err:   " << msg << std::endl;
				break;
			default:
				std::cout << "unknown: " << msg << std::endl;
				break;
		}
	}
}
