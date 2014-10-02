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
		const char *output_path = "out";
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
	
	// output path prefix, defaults to 'out/'
	void set_output_path_prefix(const char *path_prefix)
	{
		output_path = path_prefix;
	}
	
	// buffer must be at least 128 bytes
	const char* format_package_path(const char *name, char *outbuf)
	{
#ifdef PUTKI_NO_RT_PATH_PREFIX
		sprintf(outbuf, "packages/%s", name);
		return outbuf;
#else
		char tmp[128];
		sprintf(tmp, "%s-%s", runtime::desc_str(), build_config);
		for (int i=0;i<127;i++)
			tmp[i] = ::tolower(tmp[i]);
		
		sprintf(outbuf, "%s/%s/packages/%s", output_path, tmp, name);
		return outbuf;
#endif
	}

	const char* format_file_path(const char *name, char *outbuf)
	{
#ifdef PUTKI_NO_RT_PATH_PREFIX
		strcpy(outbuf, name);
		return outbuf;
#else
		char tmp[128];
		sprintf(tmp, "%s-%s", runtime::desc_str(), build_config);
		for (int i=0;i<127;i++)
			tmp[i] = ::tolower(tmp[i]);

		sprintf(outbuf, "%s/%s/%s", output_path, tmp, name);
		return outbuf;
#endif
	}

}

