#ifndef __putki_databuilder_lib__build__
#define __putki_databuilder_lib__build__

#include <putki/runtime.h>
#include <putki/builder/typereg.h>

namespace putki
{
	namespace package { struct data; }
	namespace db { struct data; }
	namespace builder { struct data; }

	namespace build
	{
		struct packaging_config;

		void full_build(builder::data *builder, bool make_patch);
		void single_build(builder::data *builder, const char *path);
		
		// make sure it is all resolved
		void resolve_object(db::data *source, const char *path);

		void post_build_ptr_update(db::data *input, db::data *output);
		void post_build_merge_database(putki::db::data *source, db::data *target);

		// can be called from user functions.
		void commit_package(putki::package::data *package, packaging_config *packaging, const char *out_path);
	}
}

#endif
