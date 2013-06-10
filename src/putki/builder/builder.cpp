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
			putki::runtime runtime;
		};
		
		data* create(putki::runtime rt)
		{
			data *d = new data();
			d->runtime = rt;
			return d;
		}
		
		putki::runtime runtime(builder::data *data)
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