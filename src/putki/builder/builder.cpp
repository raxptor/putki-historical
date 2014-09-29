#include "builder.h"

#include <putki/builder/db.h>
#include <putki/builder/build-db.h>
#include <putki/builder/resource.h>
#include <putki/builder/source.h>
#include <putki/builder/inputset.h>
#include <putki/builder/write.h>
#include <putki/builder/log.h>
#include <putki/builder/tool.h>
#include <putki/sys/files.h>
#include <putki/sys/thread.h>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

namespace putki
{
	namespace builder
	{
		struct builder_entry
		{
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
			unsigned int num_threads;
			std::string obj_path, res_path, out_path, tmpobj_path, tmp_path, built_obj_path;
			build_db::data *build_db;
			inputset::data *input_set;
			inputset::data *tmp_input_set;
			deferred_loader *tmp_loader;
			deferred_loader *output_loader;
			bool liveupdates;
			
			// fix this
			db::data *grand_input;
		};

		struct prebuild_info
		{
			std::vector<std::string> require_outputs;
		};

		struct work_item
		{
			db::data *input;
			std::string path, parent_path;
			bool from_cache;
			prebuild_info prebuild;
		};

		struct build_context
		{
			builder::data *builder;
			db::data *input;
			db::data *output;
			db::data *tmp;

			sys::mutex mtx_items, mtx_output, mtx_tmp;
			sys::condition cnd_items;
			unsigned int item_pos, items_finished;
			std::vector<work_item*> items;
			std::vector<sys::thread*> threads;
			std::set<std::string> added;
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
			{
				s_packaging_fn(out, pconf);
			}
		}

		void prebuild_add_output_dep(prebuild_info *info, const char *path)
		{
			info->require_outputs.push_back(path);
		}

		data* create(runtime::descptr rt, const char *path, bool reset_build_db, const char *build_config, int numthreads)
		{
			data *d = new data();
			d->runtime = rt;
			d->config = build_config;
			d->num_threads = numthreads ? numthreads : 4;
			d->liveupdates = false;

			d->obj_path = d->res_path = d->out_path = d->tmp_path = d->tmpobj_path = d->built_obj_path = path;

			std::string desc_path = runtime::desc_str(rt);
			if (build_config)
			{
				desc_path.append("-");
				desc_path.append(build_config);
			}

			for (unsigned int i=0;i<desc_path.size();i++)
				desc_path[i] = ::tolower(desc_path[i]);

			d->obj_path.append("/data/objs");
			d->res_path.append("/data/res");

			d->out_path.append("/out/");
			d->out_path.append(desc_path);
			d->out_path.append("");

			d->tmpobj_path.append("/out/");
			d->tmpobj_path.append(desc_path);
			d->tmpobj_path.append("/.obj_tmp");

			d->tmp_path.append("/out/");
			d->tmp_path.append(desc_path);
			d->tmp_path.append("/.res_tmp");

			d->built_obj_path.append("/out/");
			d->built_obj_path.append(desc_path);
			d->built_obj_path.append("/.built");

			// app specific configurators
			if (s_init_fn)
			{
				s_init_fn(d);
			}

			std::string build_db_path = path;
			build_db_path.append("/out/");
			build_db_path.append(desc_path);
			build_db_path.append(".build-db");

			std::string input_db_path = path;
			input_db_path.append("/out/.input-db");
			
			std::string tmp_db_path = path;
			tmp_db_path.append("/out/");
			tmp_db_path.append(desc_path);
			tmp_db_path.append("/.input-db");

			d->build_db = build_db::create(build_db_path.c_str(), !reset_build_db);
			d->input_set = inputset::open(d->obj_path.c_str(), d->res_path.c_str(), input_db_path.c_str());
			d->tmp_input_set = inputset::open(d->tmpobj_path.c_str(), d->tmp_path.c_str(), tmp_db_path.c_str());

			d->grand_input = 0;
			return d;
		}

		void free(data *builder)
		{
			// keep it all in memory!
			if (!builder->liveupdates)
			{
				inputset::write(builder->input_set);
				inputset::write(builder->tmp_input_set);
			}

			build_db::release(builder->build_db);
			inputset::release(builder->input_set);
			inputset::release(builder->tmp_input_set);
			loader_decref(builder->output_loader);
			loader_decref(builder->tmp_loader);

			delete builder;
		}

		void enable_liveupdate_builds(builder::data *data)
		{
			data->liveupdates = true;
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

		void add_data_builder(builder::data *builder, type_t type, handler_i *handler)
		{
			BuildersMap::iterator i = builder->handlers.find(type);
			if (i == builder->handlers.end())
			{
				type_entry t;
				builder->handlers[type] = t;
				add_data_builder(builder, type, handler);
				return;
			}

			builder_entry b;
			b.handler = handler;
			i->second.handlers.push_back(b);
		}

		// remove this when cleaned up
		struct destroy_deplist
		{
			destroy_deplist(build_db::deplist *dl) : _dl(dl) { }
			~destroy_deplist() { build_db::deplist_free(_dl); }
			build_db::deplist *_dl;
		};

		// This adds a newly created input object from the handler. It is an output from the handler's point of view, but really an input.
		// Maybe we could support adding final objects too.
		void add_handler_output(build_context *ctx, build_db::record *record, const char *path, type_handler_i *type, instance_t obj, const char *handler_version)
		{
			// add input object to tmp
			db::insert(ctx->tmp, path, type, obj);
			putki::build_db::add_output(record, path, handler_version);
		}

		void touched_temp_resource(data *builder, const char *path)
		{
			inputset::touched_resource(builder->tmp_input_set, path);
		}

		// returns either 0 (loaded from cache)
		// or a reason to rebuild.
		const char* fetch_cached_build(build_context *context, data *builder, build_db::record * newrecord, const char *handler_name, db::data *input, const char *path, type_handler_i *th)
		{
			// Time to hunt for cached object.
			build_db::record *record = build_db::find(builder::get_build_db(builder), path);
			if (!record)
				return "no build record found";
				
			const char *old_builder = build_db::get_builder(record);
			if (strcmp(old_builder, handler_name))
			{
				RECORD_DEBUG(newrecord, "Old builder=" << old_builder << " current=" << handler_name)
				return "builders are diffeent";
			}

			build_db::deplist *dlist = build_db::inputdeps_get(builder::get_build_db(builder), path);
			destroy_deplist destroy(dlist);

			if (!dlist)
			{
				return "no dlist";
			}

			bool gotmatch = false;

			for (int i=0;;i++)
			{
				const char *entrypath = build_db::deplist_path(dlist, i);
				if (!entrypath)
				{
					break;
				}
			
				if (!i)
				{
					RECORD_DEBUG(newrecord, "Examining cache...")
				}

				const char *signature = build_db::deplist_signature(dlist, i);
				if (!build_db::deplist_is_external_resource(dlist, i))
				{
					char inputsig[SIG_BUF_SIZE];

					if (builder->liveupdates)
					{
						strcpy(inputsig, "<broken signature>");
						db::signature(input, entrypath, inputsig);
					}
					else
					{
						if (!inputset::get_object_sig(builder->input_set, entrypath, inputsig))
						{
							if (!inputset::get_object_sig(builder->tmp_input_set, entrypath, inputsig))
							{
								BUILD_ERROR(builder, "Signature missing weirdness [" << entrypath << "]")
								strcpy(inputsig, "bonkers");
							}
						}
					}

					RECORD_DEBUG(newrecord, "=> i: " << entrypath << " old:" << signature << " new " << inputsig)

					// only care for builder match when the input is going to be built with this builder
					if (strcmp(inputsig, signature))
					{
						RECORD_DEBUG(newrecord, "!! Detected modification in [" << entrypath << "]")
						return "input or has been modified";
					}

					gotmatch = true;
				}
				else
				{
					RECORD_DEBUG(newrecord, "=> ext: " << entrypath << " old:" << signature)

					char cursig[SIG_BUF_SIZE];
					if ( (entrypath[0] != '%' && !inputset::get_res_sig(builder->input_set, entrypath, cursig)) ||
					     (entrypath[0] == '%' && !inputset::get_res_sig(builder->tmp_input_set, entrypath+1, cursig) ))
					{
						RECORD_DEBUG(newrecord, "Could not get input sig for " << entrypath)
						return "failed to read signature on existing resource";
					}
					
					if (strcmp(cursig, signature))
						return "external source data has been modified";

					gotmatch = true;
				}
			}

			if (!gotmatch)
				return "there was a record but no matches nor mismatches";

			// Replace the build record from the cache.

			RECORD_DEBUG(newrecord, "Loading from cache...")
			if (!build_db::copy_existing(builder::get_build_db(builder), newrecord, path))
			{
				BUILD_ERROR(builder, "Failed to copy existing FAILED")
			}

			// first pass is outputs, second is pointer (which need to be loaded!)
			for (int j=0;;j++)
			{
				const char *rpath = build_db::enum_outputs(newrecord, j);
				if (!rpath)
				{
					break;
				}
				
				RECORD_DEBUG(newrecord, "	output[" << j << "] is " << rpath)

				// auxes will not be inserted.
				if (db::is_aux_path(rpath))
					continue;
					
				if (!strcmp(path, rpath))
					load_file_deferred(builder->output_loader, context->output, rpath);
				else
					load_file_deferred(builder->tmp_loader, context->tmp, rpath);
			}

			if (!db::exists(context->output, path, true))
			{
				BUILD_ERROR(builder, "ERROR! Wanted to load cached object [" << path << "] but it did not work!")
			}

			// Now we hope for the best since these files came from disk and should be OK unless the user
			// has touched them.
			return 0;
		}

		// gets aux pointers to the input, adding to output.
		struct get_aux_deps : public putki::depwalker_i
		{
			std::vector<std::string> aux_outs;
			db::data *input, *tmp, *output;
			build_db::record *record;
			const char *name;

			bool pointer_pre(putki::instance_t *on, const char *type_name)
			{
				if (!*on)
					return false;

				const char *path = putki::db::pathof_including_unresolved(output, *on);
				if (!path)
					return false;

				if (!putki::db::is_aux_path(path))
					return false;

				aux_outs.push_back(path);
				return true;
			}

			void pointer_post(putki::instance_t *on)
			{
				if (!*on)
					return;

				putki::type_handler_i *th;
				putki::instance_t obj = 0;

				const char *path = putki::db::pathof_including_unresolved(input, *on);
				if (!path && !(path = putki::db::pathof_including_unresolved(tmp, *on)))
				{
					RECORD_ERROR(record, "No path on non-null pointer")
					return;
				}

				if (!putki::db::is_aux_path(path))
					return;

				if (putki::db::fetch(output, path, &th, &obj))
				{
					*on = obj;
					RECORD_DEBUG(record, "Updated pointer to aux [" << path << "]")
				}
				else
				{
					if (!putki::db::fetch(input, path, &th, &obj))
					{
						RECORD_ERROR(record, "Breakdown of common sense on [" << path << "] CRAZY OBJECT!")
						return;
					}

					RECORD_DEBUG(record, "Cloning [" << path << "] to output.")
					obj = th->clone(obj);
					putki::db::insert(output, path, th, obj);
					*on = obj;
				}
			}
		};


		// return true if was built, false if cached.
		bool build_object(build_context *context, build_db::record * record, db::data *input, const char *path, type_handler_i *th)
		{
			bool handled = false;
			instance_t input_obj = 0;
			instance_t output_obj = 0;
			
			BuildersMap::iterator i = context->builder->handlers.find(th->name());
			if (i != context->builder->handlers.end())
			{
				for (std::vector<builder_entry>::size_type j=0;j!=i->second.handlers.size();j++)
				{
					const builder_entry *e = &i->second.handlers[j];
					const char *reason = fetch_cached_build(context, context->builder, record, e->handler->version(), input, path, th);
					if (reason)
					{
						APP_INFO("Building [" << path << "]")
						RECORD_INFO(record, "Building with <" << e->handler->version() << "> because " << reason);

						// now need to build, clone it before the builder gets to it.
						if (!db::fetch(input, path, &th, &input_obj))
							APP_ERROR("Could not fetch but it existed before? " << path);

						verify_obj(input, 0, th, input_obj, REQUIRE_RESOLVED | REQUIRE_HAS_PATHS, true, true);
						output_obj = th->clone(input_obj);
						e->handler->handle(context, context->builder, record, input, path, output_obj);
					}
					else
					{
						// build record has been rewritten from cache, can't go modify it more now.
						RECORD_DEBUG(record, "Using from cache")
						return false;
					}

					// we only support one builder per object
					handled = true;
					build_db::set_builder(record, e->handler->version());
					break;
				}
			}

			const char *default_name = "default";

			if (!handled)
			{
				// Just moving into output
				build_db::set_builder(record, default_name);

				// can only come here if no builder was triggered. need to see if cached (though will be same) is available,
				// only actually to see if it needs to be rewritten
				if (!fetch_cached_build(context, context->builder, record, default_name, input, path, th))
				{
					RECORD_DEBUG(record, "Using from cache")
					return false;
				}
				else
				{
					RECORD_INFO(record, "Building with default handler")
				}
			}

			db::data *outputdb = context->output;

			// if it has been sucked into input (check without triggering delayed load),
			// then a clone must be inserted
			if (output_obj)
			{
				// if we have the ptr already it was processed/cloned
				db::insert(outputdb, path, th, output_obj);
			}
			else
			{
				if (!input_obj && !db::fetch(input, path, &th, &input_obj, true))
				{
					APP_ERROR("Could not fetch")
				}

				output_obj = th->clone(input_obj);
				db::insert(outputdb, path, th, output_obj);
			}

			get_aux_deps ad;
			ad.input = context->input;
			ad.tmp = context->tmp;
			ad.output = context->output;
			ad.record = record;
			ad.name = default_name;
			th->walk_dependencies(output_obj, &ad, false);

			for (unsigned int i=0;i<ad.aux_outs.size();i++)
			{
				build_db::add_output(record, ad.aux_outs[i].c_str(), default_name);
			}

			build_db::add_output(record, path, default_name);
			return true;
		}

		build_context *create_context(builder::data *builder, db::data *input, db::data *tmp, db::data *output)
		{
			build_context *ctx = new build_context();
			ctx->builder = builder;
			ctx->input = input;
			ctx->tmp = tmp;
			ctx->output = output;

			builder->output_loader = create_loader(builder->built_obj_path.c_str());
			loader_add_resolve_src(builder->output_loader, output, builder->built_obj_path.c_str());
			
			// TODO: Should these be here? When would we want to resolve pointers to here...
			//       Not in packaging at least!
			loader_add_resolve_src(builder->output_loader, tmp, builder->tmpobj_path.c_str());
			loader_add_resolve_src(builder->output_loader, input, builder->obj_path.c_str());

			builder->tmp_loader = create_loader(builder->tmpobj_path.c_str());
			loader_add_resolve_src(builder->tmp_loader, tmp, builder->tmpobj_path.c_str());
			loader_add_resolve_src(builder->tmp_loader, input, builder->obj_path.c_str());

			return ctx;
		}

		void context_add_to_build(build_context *context, const char *path)
		{
			sys::scoped_maybe_lock lk0(&context->mtx_items);
			if (!context->added.count(path))
			{
				work_item *wi = new work_item();
				wi->input = context->input;
				wi->path = path;
				context->items.push_back(wi);
				context->cnd_items.broadcast();
				context->added.insert(path);
			}
		}

		void context_add_build_record_pointers(build_context *context, const char *path)
		{
			build_db::record *r = build_db::find(context->builder->build_db, path);
			if (!r)
				APP_ERROR("Build db broken")

			std::vector<std::string> ptrs, final;

			for (unsigned long i=0;;i++)
			{
				const char *path = build_db::get_pointer(r, i);
				if (!path) break;

				char buf[SIG_BUF_SIZE];
				if (inputset::get_object_sig(context->builder->input_set, path, buf))
					ptrs.push_back(path);
			}
			
			sys::scoped_maybe_lock lk0(&context->mtx_items);
			for (unsigned int i=0;i!=ptrs.size();i++)
			{
				if (!context->added.count(ptrs[i]))
					final.push_back(ptrs[i]);
			}
			lk0.unlock();
			
			for (unsigned int i=0;i!=final.size();i++)
				context_add_to_build(context, final[i].c_str());
		}

		void context_process_record(build_context *context, work_item *item)
		{
			BUILD_DEBUG(context->builder, "Record: " << item->path)
			if (!db::exists(item->input, item->path.c_str(), true))
			{
				BUILD_ERROR(context->builder, "db::exists check failed on " << item->path << " " << item->input);
				return;
			}

			// We use db::signature
			char sig[SIG_BUF_SIZE];
			type_handler_i *th = 0;
			const char *type_name = 0;
			
			if (db::is_aux_path(item->path.c_str()))
			{
				strcpy(sig, "aux-no-sig");
			}
			else
			{
				type_name = inputset::get_object_type(context->builder->input_set, item->path.c_str());
				if (!inputset::get_object_sig(context->builder->input_set, item->path.c_str(), sig))
				{
					type_name = inputset::get_object_type(context->builder->tmp_input_set, item->path.c_str());
					if (!inputset::get_object_sig(context->builder->tmp_input_set, item->path.c_str(), sig))
					{
						BUILD_WARNING(context->builder, "No signature on " << item->path);
						strcpy(sig, "tmp-obj-sig");
					}
				}
			}
			
			if (!type_name)
			{
				// Fall back for inserted object during live update
				if (context->builder->liveupdates)
				{
					// recover by getting th
					instance_t tmp;
					db::fetch(item->input, item->path.c_str(), &th, &tmp);
					type_name = "live-update-insertede-object";
				}
				else
				{
					BUILD_ERROR(context->builder, "I cannot build because " << item->path << " has unknown type!");
				}
			}

			if (!th) th = typereg_get_handler(type_name);
			if (!th) BUILD_ERROR(context->builder, "No type handler for [" << type_name << "]");

			build_db::record *record = build_db::create_record(item->path.c_str(), sig);
			build_db::add_input_dependency(record, item->path.c_str());
			build_db::set_parent(record, item->parent_path.c_str());

			std::vector<work_item *> sub_items;

			bool from_cache = item->from_cache = !build_object(context, record, item->input, item->path.c_str(), th);
			{
				// create new build records for the sub outputs
				unsigned int outpos = 0;

				while (const char *cr_path_ptr = build_db::enum_outputs(record, outpos))
				{
					// ignore what we just built.
					if (!strcmp(cr_path_ptr, item->path.c_str()))
					{
						outpos++;
						continue;
					}

					if (db::is_aux_path(cr_path_ptr))
					{
						outpos++;
						continue;
					}
					
					// if from cache, data is already there. aux objs are not stored separately
					if (!from_cache && !db::is_aux_path(cr_path_ptr))
					{
						RECORD_INFO(record, "Build created [" << cr_path_ptr << "]")

						// Write out temp object cache
						type_handler_i *_th;
						instance_t _obj;
						if (db::fetch(context->tmp, cr_path_ptr, &_th, &_obj))
						{
							char fn[1024], buffer[128];
							BUILD_DEBUG(context->builder, "Writing output to tmp [" << cr_path_ptr << "] and recording signature " << db::signature(context->tmp, cr_path_ptr, buffer))
							if (write::write_object_to_fs(context->builder->tmpobj_path.c_str(), cr_path_ptr, context->tmp, _th, _obj, fn))
							{
								inputset::force_obj(context->builder->tmp_input_set, cr_path_ptr, db::signature(context->tmp, cr_path_ptr, buffer), _th->name());
							}
							else
							{
								APP_ERROR("Failed to write " << cr_path_ptr)
							}
						}
						else
						{
							BUILD_ERROR(context->builder, "Could not read output " << cr_path_ptr)
						}
					}

					work_item *wi = new work_item();
					wi->path = cr_path_ptr;
					wi->parent_path = item->path;
					wi->input = context->tmp;
					sub_items.push_back(wi);
					outpos++;
				}
			}

			APP_DEBUG("Post-processing item")

			build_db::commit_record(context->builder->build_db, record);

			if (!context->builder->liveupdates)
			{
				if (!from_cache)
					build_db::insert_metadata(builder::get_build_db(context->builder), context->output, item->path.c_str());

				context_add_build_record_pointers(context, item->path.c_str());
			}

			flush_log(record);

			context->mtx_items.lock();
			for (unsigned int i=0;i<sub_items.size();i++)
				context->items.push_back(sub_items[i]);
			context->cnd_items.broadcast();
			context->mtx_items.unlock();
		}

		void context_finalize(build_context *context)
		{
			APP_INFO("Finalizing build context with " << context->items.size() << " records.")
			std::random_shuffle(context->items.begin(), context->items.end());
		}
		
		struct buildthread
		{
			build_context *context;
			int id;
		};
		
		void* build_thread(void *userptr)
		{
			buildthread *bt = (buildthread *)userptr;
			build_context *context = bt->context;
			int id = bt->id;
			
			bool has_built = false;
			
			while (true)
			{
				// get an item
				work_item *item;
				context->mtx_items.lock();
				
				if (has_built)
				{
					context->items_finished++;
					context->cnd_items.broadcast();
				}
				
				while (true)
				{
					if (context->item_pos < context->items.size())
					{
						item = context->items[context->item_pos++];
						APP_DEBUG("Thread " << id << " picked item " << item->path)
						context->mtx_items.unlock();
						break;
					}
					if (context->items_finished == context->item_pos)
					{
						context->mtx_items.unlock();
						delete bt;
						return 0;
					}
					context->cnd_items.wait(&context->mtx_items);
				}
				
				context_process_record(context, item);
				has_built = true;
			}
		}

		void context_build(build_context *context)
		{
			context->builder->grand_input = context->input;
			context->item_pos = context->items_finished = 0;

			APP_INFO("Starting build with " << context->builder->num_threads << " threads..")
			
			for (int i=0;i<(context->builder->num_threads-1);i++)
			{
				buildthread *bt = new buildthread();
				bt->id = i;
				bt->context = context;
				context->threads.push_back(sys::thread_create(build_thread, bt));
			}

			// join the build self.
			buildthread *self = new buildthread();
			self->context = context;
			self->id = -1;
			build_thread(self);
			
			for (int i=0;i!=context->threads.size();i++)
			{
				sys::thread_join(context->threads[i]);
				APP_DEBUG("Thread " << i << " completed")
				delete context->threads[i];
			}
			
			APP_INFO("Finished build, total of " << context->items.size() << " build records")
		}

		void build_source_object(data *builder, db::data *input, db::data *tmp, db::data *output, const char *path)
		{
			work_item *wi = new work_item();
			
			if (db::exists(input, path))
			{
				wi->input = input;
			}
			else if (db::exists(tmp, path))
			{
				wi->input = tmp;
			}
			else
			{
				APP_WARNING("Tried to build object not in input or tmp! [" << path << "]")
				delete wi;
				return;
			}
			
			wi->path = path;
			
			build_context *ctx = create_context(builder, input, tmp, output);
			ctx->items.push_back(wi);
			
			context_finalize(ctx);
			context_build(ctx);
			build::post_build_ptr_update(input, output);
			context_destroy(ctx);
		}

		const char* context_get_built_object(build_context *context, unsigned int i)
		{
			if (i < context->items.size())
			{
				return context->items[i]->path.c_str();
			}
			return 0;
		}

		bool context_was_read_from_cache(build_context *context, unsigned int i)
		{
			if (i < context->items.size())
			{
				return context->items[i]->from_cache;
			}
			return false;
		}

		void context_destroy(build_context *context)
		{
			for (unsigned int i=0;i<context->items.size();i++)
				delete context->items[i];
			delete context;
		}
		void write_build_db(builder::data *d)
		{
			build_db::store(d->build_db);
		}

		void record_log(data *builder, LogType type, const char *text)
		{
			putki::print_log("BUILD", type, text);
		}
	}

}
