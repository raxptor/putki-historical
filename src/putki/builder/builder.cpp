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
			std::string obj_path, res_path, out_path, tmp_path, built_obj_path;
			build_db::data *build_db;
		};

		struct prebuild_info
		{
			std::vector<std::string> require_outputs;
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

		void prebuild_add_output_dep(prebuild_info *info, const char *path)
		{
			info->require_outputs.push_back(path);
		}

		data* create(runtime::descptr rt, const char *path, bool reset_build_db)
		{
			data *d = new data();
			d->runtime = rt;

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
			d->built_obj_path.append("/.built");

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
				std::cout << "        => Examining rebuild need for " << path << std::endl;
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
						std::cout << "        i: " << entrypath << " old:" << signature << " new " << db::signature(input, entrypath) << std::endl;
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
								std::cout << "        *** Detected modification in [" << entrypath << "]" << std::endl;
							}

							return "input or builder has been modified";
						}
					}
					else
					{
						std::cout << path << " ext:" << entrypath << std::endl;
						if (strcmp(resource::signature(builder, entrypath).c_str(), signature)) {
							return "external source data has been modified";
						}
						else {
							matches++;
						}
					}
				}

				if (matches)
				{
					// replace the build record from the cache.
					std::cout << "         *** Loading cached object " << path << std::endl;
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
							std::cout << " => Picked up cached result for [" << path << "]" << std::endl;

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
				// Just moving into output
				build_db::set_builder(record, "putki-generic");
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
				std::cout << "==> Builder already added object! [" << path << "] obj=" << obj << std::endl;
			}
			else
			{
				// always here output.
				db::insert(output, path, th, obj);
			}

			build_db::add_output(record, path, "putki-generic");
			return true;
		}

		struct work_item
		{
			db::data *input;
			db::data *output;
			std::string path;
			work_item *parent;
			int num_children;
			build_db::record *br;
			bool commit;
			prebuild_info prebuild;
		};

		struct build_context
		{
			builder::data *builder;
			db::data *input;
			db::data *output;
			std::vector<work_item*> items;
		};

		build_context *create_context(builder::data *builder, db::data *input, db::data *output)
		{
			build_context *ctx = new build_context();
			ctx->builder = builder;
			ctx->input = input;
			ctx->output = output;
			return ctx;
		}

		void context_add_to_build(build_context *context, const char *path)
		{
			work_item *wi = new work_item();
			wi->input = context->input;
			wi->output = 0;
			wi->path = path;
			wi->num_children = 0;
			wi->parent = 0;
			wi->br = 0;
			wi->commit = false;
			context->items.push_back(wi);
		}

		void post_process_item(build_context *context, work_item *item)
		{
			if (item->num_children)
				return;

			if (!item->parent)
			{
				std::cout << "     => inserting [" << item->path << "] into world output [record " << item->path << "]" << std::endl;
				build::post_build_merge_database(item->output, context->output);
				db::free(item->output);
				item->commit = true;
			}
			if (item->parent)
			{
				build::post_build_merge_database(item->output, item->parent->output);
				db::free(item->output);
				item->commit = true;

				if (!--item->parent->num_children)
				{
					post_process_item(context, item->parent);
				}
			}
		}

		struct aux_ptr_add
		{		
			
		};


		void context_process_record(build_context *context, work_item *item)
		{
			type_handler_i *th;
			instance_t obj;

			if (!db::fetch(item->input, item->path.c_str(), &th, &obj))
			{
				std::cerr << "ERROR1 WITH ITEM " << item->path << std::endl;
				return;
			}

			std::cout << "RECORD " << item->path << " object:" << obj << " input:" << item->input << " parent:" << item->parent << std::endl;
			item->br = build_db::create_record(item->path.c_str(), db::signature(item->input, item->path.c_str()));

			build_db::add_input_dependency(item->br, item->path.c_str());
			
			// If we read from the actual grand input, which is read-only, we must clone the object.
			// When building sub-assets, then not as concerned as it will not affect the build of other 
			// objects, and we save pointer updates and confusion
			instance_t clone = obj;
			if (item->input == context->input)
				clone = th->clone(obj);

			item->output = putki::db::create(item->input);

			if (build_source_object(context->builder, item->br, PHASE_INDIVIDUAL, item->input, item->path.c_str(), clone, th, item->output))
			{
				// create new build records for the sub outputs
				unsigned int outpos = 0;

				while (const char *cr_path_ptr = build_db::enum_outputs(item->br, outpos))
				{
					// ignore what we just built.
					if (!strcmp(cr_path_ptr, item->path.c_str()))
					{
						outpos++;
						continue;
					}

					work_item *wi = new work_item();
					wi->path = cr_path_ptr;
					wi->input = item->output; // where the object is
					wi->output = 0;
					wi->num_children = 0;
					wi->parent = item;
					wi->br = 0;
					wi->commit = false;
					context->items.push_back(wi);

					item->num_children++;

					std::string cr_path = cr_path_ptr;
					std::cout << "      ==> Adding subly created input [" << outpos << "] which is [" << cr_path << "]" << std::endl;

					outpos++;
				}
			} // end if was built.

			post_process_item(context, item);
		}

		void context_finalize(build_context *context)
		{
			std::cout << "Finalizing build context with " << context->items.size() << " records." << std::endl;
		}

		void context_build(build_context *context)
		{
			std::cout << "Starting build..." << std::endl;
			for (int i=0;i<context->items.size();i++)
			{
				context_process_record(context, context->items[i]);
			}

			std::cout << "Finished build with " << context->items.size() << " build records to commit" << std::endl;

			// All records are such that the parent will come first, so we go back wards.
			for (int i=context->items.size()-1;i>=0;i--)
			{
				work_item *item = context->items[i];
				if (!item->commit) 
				{
					std::cout << " item[" << i << "] path=" << item->path << " not flagged for commit?!" << std::endl;
					continue;
				}

				// done.
				if (item->parent) 
				{
					// push up results & input deps.	
					build_db::copy_input_dependencies(item->br, item->parent->br);
					build_db::append_extra_outputs(item->parent->br, item->br);
				}

				build_db::commit_record(context->builder->build_db, item->br);
			}
		}

		void build_source_object(data *builder, db::data *input, const char *path, db::data *output)
		{
			build_context *ctx = create_context(builder, input, output);
			context_add_to_build(ctx, path);
			context_finalize(ctx);
			context_build(ctx);
			context_destroy(ctx);
		}

		const char* context_get_built_object(build_context *context, unsigned int i)
		{
			if (i < context->items.size())
				return context->items[i]->path.c_str();
			return 0;
		}

		void context_destroy(build_context *context)
		{
			for (unsigned int i=0;i<context->items.size();i++)
				delete context->items[i];
			delete context;
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
