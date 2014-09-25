#ifndef __PUTKI_BUILDER_H__
#define __PUTKI_BUILDER_H__

#include <putki/builder/typereg.h>
#include <putki/builder/build.h>
#include <putki/builder/log.h>
#include <putki/runtime.h>

namespace putki
{
	namespace db { struct data; }
	namespace resource { struct data; }
	namespace build_db { struct record; struct data; }

	namespace builder
	{
		struct data;
		struct prebuild_info;
		struct record_data;
		
		// num_threads=0 => auto
		data* create(runtime::descptr rt, const char *basepath, bool reset_build_db, const char *build_config, int num_threads=0);
		void free(data *builder);
		
		// runtime
		runtime::descptr runtime(builder::data *data);
		const char *config(builder::data *data);
		
		// live update functionality
		void build_source_object(data *builder, db::data *input, db::data *tmp, db::data *output, const char *path);
		void enable_liveupdate_builds(builder::data *data);
	
		// new api
		struct build_context;
		build_context *create_context(data *builder, db::data *input, db::data *tmp, db::data *output);
		void context_add_to_build(build_context *context, const char *path);
		void context_finalize(build_context *context);
		void context_build(build_context *context);
		void context_destroy(build_context *context);

		// enumerating items in the build list after _build
		const char* context_get_built_object(build_context *context, unsigned int i);
		bool context_was_read_from_cache(build_context *context, unsigned int i);

		struct handler_i
		{
			virtual const char *version() {
				return "unknown-builder";
			};

			virtual void deps(build_context *ctx, prebuild_info *info) { }
			virtual bool handle(build_context *ctx, data *builder, build_db::record *record, db::data *input, const char *path, instance_t obj) = 0;

			template <typename T>
			inline void add_output(build_context *ctx, build_db::record *record, const char *path, T *obj)
			{
				add_handler_output(ctx, record, path, T::th(), obj, version());
			}
		};
		
		void add_data_builder(builder::data *builder, type_t type, handler_i *handler);
		void add_handler_output(build_context *ctx, build_db::record *record, const char *path, type_handler_i *type, instance_t obj, const char *handler_version);
		void touched_temp_resource(data *builder, const char *path);

		void record_log(data *builder, LogType, const char *text);
		
		// App specific callbacks for setting up & packaging.
		typedef void (*builder_setup_fn)(builder::data *builder);
		typedef void (*packaging_fn)(putki::db::data *out, putki::build::packaging_config *pconf);

		void set_builder_configurator(builder_setup_fn fn);
		void set_packager(packaging_fn fn);
		void invoke_packager(putki::db::data *out, putki::build::packaging_config *pconf);

		void write_build_db(builder::data *);
		build_db::data *get_build_db(builder::data *);

		const char *obj_path(data *d);
		const char *res_path(data *d);
		const char *tmp_path(data *d);
		const char *out_path(data *d);
		const char *built_obj_path(data *d);
	}
}

#endif
