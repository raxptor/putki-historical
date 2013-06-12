#include "log.h"

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <cstdio>
#endif

namespace claw
{
	void syslog(const char *wut)
	{
#if defined(_WIN32)
		OutputDebugStringA(wut);
		OutputDebugStringA("\n");
#else
		printf("%s\n", wut);
#endif
	}

	void log(const char *cstr)
	{
		// What hooks to use here?
		claw::syslog(cstr);
	}

	void error(const char *cstr)
	{
		claw::syslog(cstr);
#if defined(_WIN32)
		MessageBoxA(0, cstr, "ERORR!", MB_ICONERROR | MB_OK);
		ExitProcess(-1);
#else
		exit(-1);
#endif
	}
}
