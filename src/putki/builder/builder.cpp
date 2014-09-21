#include "builder.h"

#include <putki/builder/db.h>
#include <putki/builder/build-db.h>
#include <putki/builder/resource.h>
#include <putki/builder/source.h>
#include <putki/builder/inputset.h>
#include <putki/builder/write.h>
#include <putki/builder/log.h>
#include <putki/sys/files.h>
#include <putki/sys/thread.h>

#include <map>
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
			std::string obj_path, res_path, out_path, tmpobj_path, tmp_path, built_obj_path;
			build_db::data *build_db;
			inputset::data *input_set;
			inputset::data *tmp_input_set;
			deferred_loader *cache_loader;
			deferred_loader *tmp_loader;
			
			// fix this
			db::data *grand_input;
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
			if (s_packaging_fn)
			{
				s_packaging_fn(out, pconf);
			}
		}

		void prebuild_add_output_dep(prebuild_info *info, const char *path)
		{
			info->require_outputs.push_back(path);
		}

		data* create(runtime::descptr rt, const char *path, bool reset_build_db, const char *build_config)
		{
			data *d = new data();
			d->runtime = rt;
			d->config = build_config;

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

			d->cache_loader = create_loader(d->built_obj_path.c_str());
			d->tmp_loader = create_loader(d->tmpobj_path.c_str());
			
			d->grand_input = 0;
			return d;
		}

		void free(data *builder)
		{
			inputset::write(builder->input_set);
			inputset::write(builder->tmp_input_set);

			build_db::release(builder->build_db);
			inputset::release(builder->input_set);
			inputset::release(builder->tmp_input_set);
			loader_decref(builder->cache_loader);
			loader_decref(builder->tmp_loader);

			delete builder;
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

		// remove this when cleaned up
		struct destroy_deplist
		{
			destroy_deplist(build_db::deplist *dl) : _dl(dl) { }
			~destroy_deplist() { build_db::deplist_free(_dl); }
			build_db::deplist *_dl;
		};

		// returns either 0 (loaded from cache)
		// or a reason to rebuild.
		const char* fetch_cached_build(data *builder, build_db::record * newrecord, const char *handler_name, db::data *input, const char *path, instance_t obj, type_handler_i *th, db::data *output)
		{
			// Time to hunt for cached object.
			build_db::deplist *dlist = build_db::inputdeps_get(builder::get_build_db(builder), path, false);
			destroy_deplist destroy(dlist);

			if (dlist)
			{
				int matches = 0;
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
						const char *builder_name = build_db::deplist_builder(dlist, i);
						char inputsig[SIG_BUF_SIZE];
						
						if (!inputset::get_object_sig(builder->input_set, entrypath, inputsig))
						{
							if (!inputset::get_object_sig(builder->tmp_input_set, entrypath, inputsig))
							{
								BUILD_ERROR(builder, "Signature missing weirdness [" << entrypath << "]")
								strcpy(inputsig, "bonkers");
							}
						}

						bool sigmatch = !strcmp(inputsig, signature);
						RECORD_DEBUG(newrecord, "=> i: " << entrypath << " old:" << signature << " new " << inputsig)

						// only care for builder match when the input is going to be built with this builder
						bool buildermatch = strcmp(path, entrypath) || !strcmp(builder_name, handler_name);
						if (sigmatch && buildermatch)
						{
							matches++;
						}
						else
						{
							if (!buildermatch)
							{
								return "builder version is different";
							}
							else
							{
								RECORD_DEBUG(newrecord, "!! Detected modification in [" << entrypath << "]")
							}

							return "input or builder has been modified";
						}
					}
					else
					{
						RECORD_DEBUG(newrecord, "=> ext: " << entrypath << " old:" << signature)
						char cursig[SIG_BUF_SIZE];
						if ( (entrypath[0] != '%' && !inputset::get_res_sig(builder->input_set, entrypath, cursig)) ||
						     (entrypath[0] == '%' && !inputset::get_res_sig(builder->tmp_input_set, entrypath+1, cursig) ))
						{
							RECORD_DEBUG(newrecord, "Could not get input sig for " << entrypath)
							strcpy(cursig, "missing");
						}
						
						if (strcmp(cursig, signature))
						{
							return "external source data has been modified";
						}
						else
						{
							matches++;
						}
					}
				}

				if (matches)
				{
					// Replace the build record from the cache.
					//
					// NOTE: This needs to be an atomic operation as it might pull in dependencies
					//       that are/will be rebuilt... objects that are pointed to but not registered as
					//       dependencies might cause that object to be rebuilt, and then we load a cached object here
					//       which pulls in an 'old' version into the database... no good!
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

						// We want to carefully load these files, and not overwrite any which might have been built
						// this time.
						if (!db::exists(output, rpath))
						{
							// only the main object is pulled from output
							if (!strcmp(path, rpath))
							{
								load_file_deferred(builder->cache_loader, output, rpath, 0);
							}
							else
							{
								load_file_deferred(builder->tmp_loader, output, rpath, builder->grand_input);
							}
						}
						else
						{
							BUILD_ERROR(builder, "WHY AM I INSERTING ALREADY EXISTING OUTPUTS?! [" << path << "]")
						}
					}

					if (!db::exists(output, path))
					{
						BUILD_ERROR(builder, "ERROR! Wanted to load cached object [" << path << "] but it did not work!")
					}

					// Now we hope for the best since these files came from disk and should be OK unless the user
					// has touched them.
					return 0;
				}
				else
				{
					return "resource has not been built";
				}
			}

			return "no previous build records";
		}

		// gets aux pointers to the input, adding to output.
		struct get_aux_deps : public putki::depwalker_i
		{
			std::vector<std::string> objects;
			db::data *input, *output;
			build_db::record *record;
			const char *name;

			bool pointer_pre(putki::instance_t *on)
			{
				return true;
			}

			void pointer_post(putki::instance_t *on)
			{
				if (!*on)
				{
					return;
				}

				const char *path = putki::db::pathof(input, *on);
				if (!path)
				{
					return;
				}

				if (!putki::db::is_aux_path(path))
				{
					return;
				}

				putki::type_handler_i *th;
				putki::instance_t obj = 0;

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
		bool build_source_object(data *builder, build_db::record * record, int phase, db::data *input, const char *path, instance_t obj, type_handler_i *th, db::data *output)
		{
			bool handled = false;
			bool built_by_any = false;
			BuildersMap::iterator i = builder->handlers.find(th->name());
			if (i != builder->handlers.end())
			{
				for (std::vector<builder_entry>::size_type j=0;j!=i->second.handlers.size();j++)
				{
					const builder_entry *e = &i->second.handlers[j];
					if (e->obj_phase_mask & phase)
					{
						const char *reason = fetch_cached_build(builder, record, e->handler->version(), input, path, obj, th, output);
						if (reason)
						{
							APP_INFO("Builing [" << path << "]")
							RECORD_INFO(record, "Building with <" << e->handler->version() << "> because " << reason);

							// now need to build, clone it before the builder gets to it.
							if (input == builder->grand_input)
								obj = th->clone(obj);

							e->handler->handle(builder, record, input, path, obj, output, phase);
							built_by_any = true;
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
			}

			const char *default_name = "default";

			if (!handled)
			{
				// Just moving into output

				build_db::set_builder(record, default_name);

				// can only come here if no builder was triggered. need to see if cached (though will be same) is available,
				// only actually to see if it needs to be rewritten
				if (!fetch_cached_build(builder, record, default_name, input, path, obj, th, output))
				{
					RECORD_DEBUG(record, "Using from cache")
					return false;
				}
				else
				{
					RECORD_INFO(record, "Building with default handler")
				}
			}

			if (!db::exists(output, path))
			{
				// items that go to output must always be clones. if they weren't built,
				// they weren't cloned.
				if (!built_by_any && input == builder->grand_input)
					obj = th->clone(obj);

				db::insert(output, path, th, obj);
			}

			// add aux objects directly pointed by this object.. no children
			// because other objects need to take care of themselves.
			get_aux_deps ad;
			ad.input = input;
			ad.output = output;
			ad.record = record;
			ad.name = default_name;
			th->walk_dependencies(obj, &ad, false);

			build_db::add_output(record, path, default_name);
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
			bool from_cache;
			prebuild_info prebuild;
			sys::mutex data;
		};

		struct build_context
		{
			builder::data *builder;
			db::data *input;
			db::data *output;
			db::data *trash;
			
			sys::mutex mtx_items, mtx_output, mtx_trash;
			sys::condition cnd_items;
			unsigned int item_pos, items_finished;
			std::vector<work_item*> items;
			
			std::vector<sys::thread*> threads;
			sys::mutex everything;
		};

		build_context *create_context(builder::data *builder, db::data *input, db::data *output)
		{
			build_context *ctx = new build_context();
			ctx->builder = builder;
			ctx->input = input;
			ctx->output = output;
			ctx->trash = db::create(0, &ctx->mtx_trash);
			return ctx;
		}

		void context_add_to_build(build_context *context, const char *path)
		{
			sys::scoped_maybe_lock lk0(&context->mtx_items);
			work_item *wi = new work_item();
			wi->input = context->input;
			wi->output = 0;
			wi->path = path;
			wi->num_children = 0;
			wi->parent = 0;
			wi->br = 0;
			wi->commit = false;
			wi->from_cache = false;
			context->items.push_back(wi);
		}

		void post_process_item(build_context *context, work_item *item)
		{
                	sys::scoped_maybe_lock id(&item->data);
			if (item->num_children)
			{
				id.unlock();
				return;
			}

			if (!item->parent)
			{
				RECORD_DEBUG(item->br, "Merging to world output")
				id.unlock();

				flush_log(item->br);

				build::post_build_merge_database(item->output, context->output, context->trash);
				db::free(item->output);
				
				item->commit = true;
			}
			if (item->parent)
			{
				sys::scoped_maybe_lock lk(&item->parent->data);
			
				RECORD_DEBUG(item->br, "Merging to parent set " << item->parent->path)
				flush_log(item->br);				
				
				build::post_build_merge_database(item->output, item->parent->output, context->trash);
				
				db::free(item->output);
				
				item->commit = true;

				if (!--item->parent->num_children)
				{
					lk.unlock();
					post_process_item(context, item->parent);
				}
			}
		}

		void context_process_record(build_context *context, work_item *item)
		{
			type_handler_i *th;
			instance_t obj;

			BUILD_DEBUG(context->builder, "Record: " << item->path)

			if (!db::fetch(item->input, item->path.c_str(), &th, &obj))
			{
				BUILD_ERROR(context->builder, "Fetch failed on " << item->path);
				return;
			}
			
			// We use db::signature
			char sig[SIG_BUF_SIZE];
			if (!inputset::get_object_sig(context->builder->input_set, item->path.c_str(), sig))
			{
				if (!inputset::get_object_sig(context->builder->tmp_input_set, item->path.c_str(), sig))
				{
					BUILD_ERROR(context->builder, "No signature");
					strcpy(sig, "tmp-obj-sig");
				}
			}

			item->br = build_db::create_record(item->path.c_str(), sig);

			build_db::add_input_dependency(item->br, item->path.c_str());

			item->output = putki::db::create(item->input);
			
			std::vector<work_item *> sub_items;

			const bool from_cache = !build_source_object(context->builder, item->br, PHASE_INDIVIDUAL, item->input, item->path.c_str(), obj, th, item->output);
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

					if (!from_cache)
					{
						// this is a tmp obj, store.
						type_handler_i *_th;
						instance_t _obj;
						if (db::fetch(item->output, cr_path_ptr, &_th, &_obj))
						{
							char fn[1024];
							// BUILD_DEBUG(context->builder, "Writing output to tmp [" << cr_path_ptr << "] and recording signature " << db::signature(item->output, cr_path_ptr))
							if (write::write_object_to_fs(context->builder->tmpobj_path.c_str(), cr_path_ptr, item->output, _th, _obj, fn))
							{
								inputset::force_obj(context->builder->tmp_input_set, cr_path_ptr, db::signature(item->output, cr_path_ptr));
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
					wi->input = item->output; // where the object is
					wi->output = 0;
					wi->num_children = 0;
					wi->parent = item;
					wi->br = 0;
					wi->commit = false;
					wi->from_cache = from_cache;
					sub_items.push_back(wi);
									
					std::string cr_path = cr_path_ptr;

					if (!from_cache)
					{
						RECORD_INFO(item->br, "Build created [" << cr_path << "]")
					}

					outpos++;
				}
			}
			
			item->num_children += sub_items.size();
			
			context->mtx_items.lock();
			for (unsigned int i=0;i<sub_items.size();i++)
				context->items.push_back(sub_items[i]);
			context->mtx_items.unlock();

			post_process_item(context, item);
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

			const int threads = 20;
			APP_INFO("Starting build with " << threads << " threads..")
			
			for (int i=0;i<threads;i++)
			{
				buildthread *bt = new buildthread();
				bt->id = i;
				bt->context = context;
				context->threads.push_back(sys::thread_create(build_thread, bt));
			}
			
			for (int i=0;i<threads;i++)
			{
				sys::thread_join(context->threads[i]);
				APP_DEBUG("Thread " << i << " completed")
				delete context->threads[i];
			}
			
			APP_INFO("Finished build, total of " << context->items.size() << " build records")

			// All records are such that the parent will come first, so we go back wards.
			for (int i=context->items.size()-1;i>=0;i--)
			{
				work_item *item = context->items[i];
				if (!item->commit)
				{
					APP_WARNING("item[" << i << "] path=" << item->path << " not flagged for commit?!")
					continue;
				}

				if (item->parent)
				{
					build_db::commit_record(context->builder->build_db, item->br);
				}
				else
				{
					build_db::commit_record(context->builder->build_db, item->br);
				}

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

			APP_DEBUG("Trash size: " << db::size(context->trash))
			db::free_and_destroy_objs(context->trash);
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
			/*
			global_pass_builder gb;
			gb.builder = builder;
			gb.input = input;
			gb.output = output;
			gb.phase = PHASE_GLOBAL;
			std::cout << "Doing global build pass." << std::endl;
			db::read_all_no_fetch(input, &gb);
			std::cout << "Global build pass done." << std::endl;
			*/
		}

		void write_build_db(builder::data *d)
		{
			build_db::store(d->build_db);
		}

		void build_error(data *builder, const char *str)
		{
			std::cout << "!!! BUILD ERROR: " << str << std::endl;
		}

		void record_log(data *builder, LogType type, const char *text)
		{
			putki::print_log("BUILD", type, text);
		}
	}

}
