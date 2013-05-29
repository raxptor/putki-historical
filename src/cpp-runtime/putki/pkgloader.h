#pragma once

#include "pkgmgr.h"

namespace putki
{
	namespace pkgloader
	{
		pkgmgr::loaded_package * from_file(const char *file);
	}
}
