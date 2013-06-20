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
			PHASE_1 = 1,
			PHASE_2 = 2,
			PHASE_3 = 4
		};
		
		enum
		{
			SOURCE_OBJECT = 1,
			OUTPUT_OBJECT = 2
		};
	
		struct data;

		struct handler_i
		{
			virtual bool handle(data *builder, db::data *input, const char *path, db::data *output, int obj_phase) = 0;
		};
	
		data* create(runtime::descptr rt, const char *basepath);
		void free(data *builder);
		
		runtime::descptr runtime(builder::data *data);
		void add_data_builder(builder::data *builder, type_t type, int obj_phase, handler_i *handler);

		void build_references(data *builder, db::data *input, db::data *output, type_handler_i *type, int obj_phase, instance_t obj);

		void build_source_object(data *builder, db::data *input, const char *path, db::data *output);

		// App specific callbacks for setting up & packaging.
		typedef void (*builder_setup_fn)(builder::data *builder);
		typedef void (*packaging_fn)(putki::db::data *out, putki::build::packaging_config *pconf);

		void set_builder_configurator(builder_setup_fn fn);
		void set_packager(packaging_fn fn);
		void invoke_packager(putki::db::data *out, putki::build::packaging_config *pconf);

		const char *obj_path(data *d);
		const char *res_path(data *d);
		const char *tmp_path(data *d);
		const char *out_path(data *d);
	}
	
}

#endif
