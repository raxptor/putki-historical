#include "builder.h"

#include <putki/builder/db.h>
#include <putki/builder/build-db.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>

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
			std::string obj_path, res_path, out_path, tmp_path, dbg_path;
			build_db::data *build_db;
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
		
		data* create(runtime::descptr rt, const char *path, bool reset_build_db)
		{
			data *d = new data();
			d->runtime = rt;

			d->obj_path = d->res_path = d->out_path = d->tmp_path = d->dbg_path = path;

			d->obj_path.append("/data/objs");
			d->res_path.append("/data/res");

			d->out_path.append("/out/");
			d->out_path.append(runtime::desc_str(rt));
			d->out_path.append("");

			d->tmp_path.append("/out/");
			d->tmp_path.append(runtime::desc_str(rt));
			d->tmp_path.append("/.tmp");

			d->dbg_path.append("/out/");
			d->dbg_path.append(runtime::desc_str(rt));
			d->dbg_path.append("/.dbg");

			// app specific configurators
			if (s_init_fn)
				s_init_fn(d);

			std::string build_db_path = path;
			build_db_path.append("/out/");
			build_db_path.append(runtime::desc_str(rt));
			build_db_path.append(".build-db");
			d->build_db = build_db::create(build_db_path.c_str(), !reset_build_db);
			return d;
		}
		
		build_db::data *get_build_db(builder::data *d)
		{
			return d->build_db;
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

		const char *dbg_path(data *d)
		{
			return d->dbg_path.c_str();
		}

		runtime::descptr runtime(builder::data *data)
		{
			return data->runtime;
		}

		void free(data *builder)
		{
			build_db::release(builder->build_db);
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
		
		void build_source_object(data *builder, build_db::record * record, int phase, db::data *input, const char *path, instance_t obj, type_handler_i *th, db::data *output)
		{
			bool handled = false;
			BuildersMap::iterator i = builder->handlers.find(th->name());
			if (i != builder->handlers.end())
			{
				for (std::vector<builder_entry>::size_type j=0;j!=i->second.handlers.size();j++)
				{
					const builder_entry *e = &i->second.handlers[j];
					if (e->obj_phase_mask & phase)
					{
						e->handler->handle(builder, record, input, path, obj, output, phase);
						
						// TODO: Clean up return values. Now always assume they are handled
						//       if we actually call in here.
						handled = true;
						
						build_db::set_builder(record, e->handler->version());
					}
				}
			}
			
			if (!handled)
			{
				build_db::set_builder(record, "pukti-generic");
				db::insert(output, path, th, obj);
			}
			
			// always adds its own output.
			build_db::add_output(record, path, "putki-generic");
		}

		void build_source_object(data *builder, db::data *input, const char *path, db::data *output)
		{
			type_handler_i *th;
			instance_t obj;
			if (!db::fetch(input, path, &th, &obj))
				return;
				
			build_db::record * br = build_db::create_record(path, db::signature(input, path));

			build_db::add_input_dependency(br, path);

			putki::db::data *tmp_output = putki::db::create();
			
			// since we are reading from the actual input here, clone the object so we can conveniently modify the contents.
			// this also means a pointer update needs to be done after this step.
			instance_t clone = th->clone(obj);
			
			build_source_object(builder, br, PHASE_INDIVIDUAL, input, path, clone, th, tmp_output);

			// sequentially build all outputs as well.
			unsigned int outpos = 0;

			while (const char *cr_path_ptr = build_db::enum_outputs(br, outpos))
			{
				// ignore what we just built.
				if (!strcmp(cr_path_ptr, path))
				{
					outpos++;
					continue;
				}

				std::string cr_path = cr_path_ptr;
				std::cout << "      ==> Building subly created input [" << outpos << "] which is [" << cr_path << "]" << std::endl;

				// => This build step uses input as input still, and a tmp db for output.
				//
				//    Since these objects were generated by the previous build we do not need to clone them; they will not ruin any original input data
				// 
				{
					// note these are in the output
					type_handler_i *th;
					instance_t obj;

					if (db::fetch(tmp_output, cr_path.c_str(), &th, &obj))
					{
						build_db::record *suboutput = build_db::create_record(cr_path.c_str(), db::signature(tmp_output, cr_path.c_str()));
						build_db::copy_input_dependencies(suboutput, br);

						build_source_object(builder, suboutput, PHASE_INDIVIDUAL, input, cr_path.c_str(), obj, th, tmp_output);

						build_db::append_extra_outputs(br, suboutput);
						build_db::commit_record(builder->build_db, suboutput);
					}
					else
					{
						std::cout << " **** BUILD OUTPUT REPORTED ADDED OBJECT WHICH WAS NOT FOUND " << std::endl;
					}

					outpos++;
				}
			}

			build_db::commit_record(builder->build_db, br);

			// merge into the real output.
			build::post_build_merge_database(tmp_output, output);
			db::free(tmp_output);
		}

		struct global_pass_builder : public db::enum_i
		{
			data *builder;
			db::data *input, *output;
			int phase;

			virtual void record(const char *path, type_handler_i *th, instance_t i) 
			{
				/*
				buildrecord::data br;
				build_source_object(builder, &br, phase, input, path, i, th, output);
				*/
			}
		};

		void build_global_pass(data *builder, db::data *input, db::data *output)
		{
			global_pass_builder gb;
			gb.builder = builder;
			gb.input = input;
			gb.output = output;
			gb.phase = PHASE_GLOBAL;
			std::cout << "==> Doing global build pass." << std::endl;
			db::read_all(input, &gb);
			std::cout << "==> Global build pass done." << std::endl;
		}
		
		void write_build_db(builder::data *d)
		{
			build_db::store(d->build_db);
		}
		
		void build_error(data *builder, const char *str)
		{
			std::cout << "!!! BUILD ERROR: " << str << std::endl;
		}
		
	}

}
