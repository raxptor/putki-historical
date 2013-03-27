#pragma once

#include "pkgmgr.h"

namespace outki
{
	namespace pkgloader
	{
		pkgmgr::loaded_package * from_file(const char *file);
	}
}
