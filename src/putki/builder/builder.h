#ifndef __PUTKI_BUILDER_H__
#define __PUTKI_BUILDER_H__

#include <putki/builder/typereg.h>
#include <putki/builder/build.h>
#include <putki/runtime.h>

namespace putki
{
	namespace db { struct data; }
	namespace resource { struct data; }
	
	namespace builder
	{
		enum
		{
			PHASE_INDIVIDUAL = 1,
			PHASE_GLOBAL     = 2,   // build for things that need global scans.
			PHASE_FINAL      = 4    // build steps fro things that could be created from global scan.
		};
		
		struct data;

		struct handler_i
		{
			virtual bool handle(data *builder, db::data *input, const char *path, instance_t obj, db::data *output, int obj_phase) = 0;
		};
	
		data* create(runtime::descptr rt, const char *basepath);
		void free(data *builder);
		
		runtime::descptr runtime(builder::data *data);
		void add_data_builder(builder::data *builder, type_t type, int obj_phase, handler_i *handler);

		void build_references(data *builder, db::data *input, db::data *output, type_handler_i *type, int obj_phase, instance_t obj);

		void build_source_object(data *builder, db::data *input, const char *path, db::data *output);
		void build_global_pass(data *builder, db::data *input, db::data *output);
		void build_final_pass(data *builder, db::data *input, db::data *output);

		// App specific callbacks for setting up & packaging.
		typedef void (*builder_setup_fn)(builder::data *builder);
		typedef void (*packaging_fn)(putki::db::data *out, putki::build::packaging_config *pconf);

		void set_builder_configurator(builder_setup_fn fn);
		void set_packager(packaging_fn fn);
		void invoke_packager(putki::db::data *out, putki::build::packaging_config *pconf);

		void build_error(data *builder, const char *str);

		const char *obj_path(data *d);
		const char *res_path(data *d);
		const char *tmp_path(data *d);
		const char *out_path(data *d);
		const char *dbg_path(data *d);
	}
	
}

#endif
