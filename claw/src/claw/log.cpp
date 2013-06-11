#include "log.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace claw
{
	void syslog(const char *wut)
	{
		OutputDebugStringA(wut);
		OutputDebugStringA("\n");
	}

	void log(const char *cstr)
	{
		// What hooks to use here?
		claw::syslog(cstr);
	}

	void error(const char *cstr)
	{
		claw::syslog(cstr);
		MessageBoxA(0, cstr, "ERORR!", MB_ICONERROR | MB_OK);
		ExitProcess(-1);
	}
}
