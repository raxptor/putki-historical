#include "staticdata.h"

#include <putki/pkgmgr.h>
#include <putki/pkgloader.h>

namespace game
{
	namespace
	{
		putki::pkgmgr::loaded_package *s_pkg = 0;
	}

	void load_static_package()
	{
		s_pkg = putki::pkgloader::from_file("out/win32/packages/static.pkg");
	}

	outki::globalsettings *get_global_settings()
	{
		return (outki::globalsettings *)putki::pkgmgr::resolve(s_pkg, "globalsettings");
	}

}
