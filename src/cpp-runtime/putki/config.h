#ifndef __PUTKI_RT_CONFIG__
#define __PUTKI_RT_CONFIG__

namespace putki
{
	// which config to use to load paths. is 'Default' unless
	// modified. this affects paths from loading packages
	void set_build_config(const char *config);
	
	const char *get_build_config();
	
	// package path prefix, defaults to 'out/'
	void set_package_path_prefix(const char *path_prefix);
	
	// buffer must be at least 128 bytes
	void format_package_path(const char *name, char *outbuf);
}

#endif
