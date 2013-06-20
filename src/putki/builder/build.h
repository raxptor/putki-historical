#ifndef __putki_databuilder_lib__build__
#define __putki_databuilder_lib__build__

#include <putki/runtime.h>

namespace putki
{
	namespace package { struct data; }
	namespace db { struct data; }
	namespace builder { struct data; }
	
	namespace build
	{
		struct packaging_config;
	
		void full_build(builder::data *builder);

		void post_build_ptr_update(db::data *input, db::data *output);
		
		// can be called from user functions.
		void commit_package(putki::package::data *package, packaging_config *packaging, const char *out_path);
	}
}

#endif
