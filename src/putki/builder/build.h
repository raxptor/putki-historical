#ifndef __putki_databuilder_lib__build__
#define __putki_databuilder_lib__build__

#include <putki/runtime.h>
#include <putki/builder/builder.h>

namespace putki
{
	namespace package { struct data; }
	
	namespace build
	{
		struct packaging_config;
	
		void full_build(putki::builder::data *builder, const char *input_path, const char *output_path, const char *package_path);
		
		// can be called from user functions.
		void commit_package(putki::package::data *package, packaging_config *packaging, const char *out_path);
	}
}

#endif