#include "builder.h"

#include <putki/builder/db.h>

#include <map>
#include <string>
#include <vector>

namespace putki
{
	namespace builder
	{
		struct builder_entry
		{
			int obj_phase_mask;
			handler_i *handler;
		};
		
		struct type_entry
		{
			std::vector<builder_entry> handlers;
		};
		
		typedef std::map<std::string, type_entry> BuildersMap;
		
		struct data
		{
			BuildersMap handlers;
			runtime::descptr runtime;
			std::string obj_path, res_path, out_path, tmp_path;
		};

		namespace
		{
			builder_setup_fn s_init_fn = 0;
			packaging_fn s_packaging_fn = 0;
		}

		void set_builder_configurator(builder_setup_fn conf)
		{
			s_init_fn = conf;
		}

		void set_packager(packaging_fn fn)
		{
			s_packaging_fn = fn;
		}

		void invoke_packager(db::data *out, build::packaging_config *pconf)
		{
			if (s_packaging_fn)
				s_packaging_fn(out, pconf);
		}
		
		data* create(runtime::descptr rt, const char *path)
		{
			data *d = new data();
			d->runtime = rt;

			d->obj_path = d->res_path = d->out_path = d->tmp_path = path;

			d->obj_path.append("/data/obj");
			d->res_path.append("/data/res");

			d->out_path.append("/out/");
			d->out_path.append(runtime::desc_str(rt));
			d->out_path.append("");

			d->tmp_path.append("/out/");
			d->tmp_path.append(runtime::desc_str(rt));
			d->tmp_path.append("/.tmp");

			// app specific configurators
			if (s_init_fn)
				s_init_fn(d);

			return d;
		}

		const char *obj_path(data *d)
		{
			return d->obj_path.c_str();
		}

		const char *res_path(data *d)
		{
			return d->res_path.c_str();
		}

		const char *out_path(data *d)
		{
			return d->out_path.c_str();
		}
		
		const char *tmp_path(data *d)
		{
			return d->tmp_path.c_str();
		}

		runtime::descptr runtime(builder::data *data)
		{
			return data->runtime;
		}

		void free(data *builder)
		{
			delete builder;
		}
		
		void add_data_builder(builder::data *builder, type_t type, int obj_phase_mask, handler_i *handler)
		{
			BuildersMap::iterator i = builder->handlers.find(type);
			if (i == builder->handlers.end())
			{
				type_entry t;
				builder->handlers[type] = t;
				add_data_builder(builder, type, obj_phase_mask, handler);
				return;
			}
			
			builder_entry b;
			b.obj_phase_mask = obj_phase_mask;
			b.handler = handler;
			i->second.handlers.push_back(b);
		}
		
		void build_source_object(data *builder, int phase, db::data *input, const char *path, db::data *output)
		{
			type_handler_i *th;
			instance_t obj;
			if (!db::fetch(input, path, &th, &obj))
				return;

			obj = th->clone(obj);
	
			bool handled = false;
			BuildersMap::iterator i = builder->handlers.find(th->name());
			if (i != builder->handlers.end())
			{
				for (std::vector<builder_entry>::size_type j=0;j!=i->second.handlers.size();j++)
				{
					const builder_entry *e = &i->second.handlers[j];
					if (e->obj_phase_mask & phase)
					{
						handled |= e->handler->handle(builder, input, path, output, phase);
					}
				}
			}
			
			if (!handled)
				db::insert(output, path, th, obj);
		}
				
		void build_references(data *builder, db::data *input, db::data *output, type_handler_i *type, int obj_phase, instance_t obj)
		{
		
		
		}
		
		void build_source_object(data *builder, db::data *input, const char *path, db::data *output)
		{
			build_source_object(builder, SOURCE_OBJECT, input, path, output);
		}
		
	}

}
