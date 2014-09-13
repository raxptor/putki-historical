#include "builder.h"

#include <putki/builder/db.h>
#include <putki/builder/build-db.h>
#include <putki/builder/resource.h>
#include <putki/builder/source.h>

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
			std::string config;
			std::string obj_path, res_path, out_path, tmp_path, built_obj_path;
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
			if (s_packaging_fn) {
				s_packaging_fn(out, pconf);
			}
		}

		data* create(runtime::descptr rt, const char *path, bool reset_build_db, const char *build_config)
		{
			data *d = new data();
			d->runtime = rt;
			d->config = build_config;

			d->obj_path = d->res_path = d->out_path = d->tmp_path = d->built_obj_path = path;

			d->obj_path.append("/data/objs");
			d->res_path.append("/data/res");

			d->out_path.append("/out/");
			d->out_path.append(runtime::desc_str(rt));
			d->out_path.append("");

			d->tmp_path.append("/out/");
			d->tmp_path.append(runtime::desc_str(rt));
			d->tmp_path.append("/.tmp");

			d->built_obj_path.append("/out/");
			d->built_obj_path.append(runtime::desc_str(rt));
			d->built_obj_path.append("/.dbg");

			// app specific configurators
			if (s_init_fn) {
				s_init_fn(d);
			}

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
		
		const char *config(builder::data *d)
		{
			return d->config.c_str();
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

		const char *built_obj_path(data *d)
		{
			return d->built_obj_path.c_str();
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

		// returns either 0 (loaded from cache)
		// or a reason to rebuild.
		const char* fetch_cached_build(data *builder, build_db::record * newrecord, builder::handler_i *handler, db::data *input, const char *path, instance_t obj, type_handler_i *th, db::data *output)
		{
			// Time to hunt for cached object.
			build_db::deplist *dlist = build_db::inputdeps_get(builder::get_build_db(builder), path, false);
			if (dlist)
			{
				int matches = 0;

				for (int i=0;; i++)
				{
					const char *entrypath = build_db::deplist_path(dlist, i);
					if (!entrypath) {
						break;
					}

					const char *signature = build_db::deplist_signature(dlist, i);

					if (!build_db::deplist_is_external_resource(dlist, i))
					{
						const char *builder = build_db::deplist_builder(dlist, i);
						bool sigmatch = !strcmp(db::signature(input, entrypath), signature);

						// std::cout << path << " i: " << entrypath << " old:" << signature << " new " << db::signature(input, entrypath) << std::endl;

						// only care for builder match when the input is going to be built with this builder
						bool buildermatch = strcmp(path, entrypath) || !strcmp(builder, handler->version());
						if (sigmatch && buildermatch) {
							matches++;
						}
						else
						{
							if (!buildermatch) {
								return "builder version is different";
							}
							else{
								std::cout << " -> Detected modification in [" << entrypath << "]" << std::endl;
							}

							return "input or builder has been modified";
						}
					}
					else
					{
						if (strcmp(resource::signature(builder, entrypath).c_str(), signature)) {
							return "external source data has been modified";
						}
						else{
							matches++;
						}
					}
				}

				if (matches)
				{
					// replace the build record from the cache.
					build_db::copy_existing(builder::get_build_db(builder), newrecord, path);

					for (int j=0;; j++)
					{
						const char *path = build_db::enum_outputs(newrecord, j);
						if (!path) {
							break;
						}

						// load the file and resolve unresolved pointers to the input database (which is how they would be
						// right after having been built anyway!)

						// (we check first so we don't re-load any objects which might already have been pulled in
						// it would be bad as it would create new pointer to the object and invalidate the previous one.
						// oh the fun of single file loading.
						if (!db::fetch(output, path, &th, &obj)) {
							load_file_into_db(builder::built_obj_path(builder), path, output, true, input);
						}

						// todo, verify signature here too maybe?
					}

					// Now we hope for the best since these files came from disk and should be OK unless the user
					// has touched them.
					return 0;
				}
				else
				{
					return "resource has not been built";
				}

				build_db::deplist_free(dlist);
			}

			return "no previous build records";
		}

		// return true if was built, false if cached.
		bool build_source_object(data *builder, build_db::record * record, int phase, db::data *input, const char *path, instance_t obj, type_handler_i *th, db::data *output)
		{
			bool handled = false;
			BuildersMap::iterator i = builder->handlers.find(th->name());
			if (i != builder->handlers.end()) {
				for (std::vector<builder_entry>::size_type j=0; j!=i->second.handlers.size(); j++)
				{
					const builder_entry *e = &i->second.handlers[j];
					if (e->obj_phase_mask & phase)
					{
						const char *reason = fetch_cached_build(builder, record, e->handler, input, path, obj, th, output);
						if (reason)
						{
							std::cout << " => Building [" << path << "] because " << reason << std::endl;
							e->handler->handle(builder, record, input, path, obj, output, phase);
						}
						else
						{
							// std::cout << " => Picked up cached result for [" << path << "]" << std::endl;

							// build record has been rewritten from cache, can't go modify it more now.
							return false;
						}

						// TODO: Clean up return values. Now always assume they are handled
						//       if we actually call in here.
						handled = true;

						build_db::set_builder(record, e->handler->version());
						break;
					}
				}
			}

			if (!handled) {
				// std::cout << " => Copied [" << path << "] into output (no registered builder)" << std::endl;
				build_db::set_builder(record, "pukti-generic");
			}

			type_handler_i *_th;
			instance_t _obj;
			if (db::fetch(output, path, &_th, &_obj) && _th == th && _obj == obj)
			{
				// Not sure if anything needs to be done, or if this check is at all necessary
				// When builders produce extra outputs, and they are built,
				// they will be built into the same database as where they came from, so will always generate this case.
				// But it means it doesn't need to be added.
				//
				// std::cout << "==> Builder already added object! [" << path << "]" << std::endl;
			}
			else
			{
				// always here output.
				db::insert(output, path, th, obj);
			}

			build_db::add_output(record, path, "putki-generic");
			return true;
		}

		void build_source_object(data *builder, db::data *input, const char *path, db::data *output)
		{
			type_handler_i *th;
			instance_t obj;
			if (!db::fetch(input, path, &th, &obj)) {
				return;
			}

			// first try to create a new one by cloning what's already in there.
			build_db::record * br = build_db::create_record(path, db::signature(input, path));
			build_db::add_input_dependency(br, path);

			// since we are reading from the actual input here, clone the object so we can conveniently modify the contents.
			// this also means a pointer update needs to be done after this step.
			instance_t clone = th->clone(obj);

			putki::db::data *tmp_output = putki::db::create();

			if (build_source_object(builder, br, PHASE_INDIVIDUAL, input, path, clone, th, tmp_output))
			{
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

							// push up sources dependencies that are in the input, this is convenience for not having to add
							// manually all the way up.
							build_db::deplist *dlist = build_db::inputdeps_get(builder::get_build_db(builder), cr_path.c_str(), true);
							if (dlist)
							{
								// Note: this dlist is a dep list wih paths only, don't try reading signatures or such.
								for (int i=0;; i++)
								{
									const char *entrypath = build_db::deplist_path(dlist, i);
									if (!entrypath) {
										break;
									}
									if (build_db::deplist_is_external_resource(dlist, i)) {
										continue;
									}

									// Add if exists in input domain. THis is to avoid adding tmp paths
									// to the input list.
									if (db::fetch(input, entrypath, &th, &obj)) {
										build_db::add_input_dependency(br, entrypath);
									}
								}
								build_db::deplist_free(dlist);
							}
						}
						else
						{
							std::cout << " **** BUILD OUTPUT REPORTED ADDED OBJECT WHICH WAS NOT FOUND " << std::endl;
						}

						outpos++;
					}
				}
			} // end if was built.

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
