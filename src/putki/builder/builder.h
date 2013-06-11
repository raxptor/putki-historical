#ifndef __PUTKI_BUILDER_H__
#define __PUTKI_BUILDER_H__

#include <putki/builder/typereg.h>
#include <putki/runtime.h>

namespace putki
{
	namespace db { struct data; }
	
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

		typedef void (*builder_setup_fn)(builder::data *builder);
	
		data* create(putki::runtime rt);
		void free(data *builder);
		
		putki::runtime runtime(builder::data *data);
		void add_data_builder(builder::data *builder, type_t type, int obj_phase, handler_i *handler);

		void build_references(data *builder, db::data *input, db::data *output, type_handler_i *type, int obj_phase, instance_t obj);

		void build_source_object(data *builder, db::data *input, const char *path, db::data *output);

		void set_builder_configurator(builder_setup_fn fn);
	}
	
}

#endif
