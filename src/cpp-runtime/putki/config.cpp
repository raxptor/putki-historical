#include "config.h"

#include <putki/runtime.h>
#include <ctype.h>
#include <cstring>
#include <cstdio>

namespace putki
{
	namespace 
	{
		const char *build_config = "Default";
		const char *package_path = "out";
	}
	
	// which config to use to load paths. is 'Default' unless
	// modified. this affects paths from loading packages
	void set_build_config(const char *config)
	{
		build_config = config;
	}
	
	const char *get_build_config()
	{
		return build_config;
	}
	
	// package path prefix, defaults to 'out/'
	void set_package_path_prefix(const char *path_prefix)
	{
		package_path = path_prefix;
	}
	
	// buffer must be at least 128 bytes
	void format_package_path(const char *name, char *outbuf)
	{
		char tmp[128];
		sprintf(tmp, "%s-%s", runtime::desc_str(), build_config);
		for (int i=0;i<127;i++)
			tmp[i] = ::tolower(tmp[i]);
		
		sprintf(outbuf, "%s/%s/packages/%s", package_path, tmp, name);
	}
}

