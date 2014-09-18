//
//  build.cpp
//  putki-databuilder-lib
//
//  Created by Dan Nilsson on 5/28/13.
//
//

#include "build.h"

#include <putki/builder/db.h>
#include <putki/builder/typereg.h>
#include <putki/builder/builder.h>
#include <putki/builder/source.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/write.h>
#include <putki/builder/build-db.h>

#include <putki/sys/files.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace
{
	const unsigned long xbufSize = 32*1024*1024;
	char xbuf[xbufSize];

	struct domain_switch : public putki::db::enum_i
	{
		putki::db::data *input, *output;

		struct depwalker : public putki::depwalker_i
		{
			domain_switch *parent;
			putki::instance_t source;

			bool pointer_pre(putki::instance_t *on)
			{
				return true;
			}

			void pointer_post(putki::instance_t *on)
			{
				// ignore nulls.
				if (!*on) {
					return;
				}

				putki::type_handler_i *th;
				putki::instance_t obj = 0;

				const char *path = putki::db::pathof_including_unresolved(parent->input, *on);
				if (!path)
				{
					// this would mean the object exists neither in the input nor output domain.
					if (!putki::db::pathof_including_unresolved(parent->output, *on)) {
						std::cout << "!!! A wild object appears! [" << *on << "]" << std::endl;
					}
					return;
				}

				if (!putki::db::fetch(parent->output, path, &th, &obj))
				{
					obj = putki::db::create_unresolved_pointer(parent->output, path);
					std::cout << "=> Adding unresolved pointer for output for path [" << path << "]" << std::endl;
					std::cout << "  object was " << obj << std::endl;
				}

				// std::cout << " - Updating reference in [" << putki::db::pathof(parent->output, source) << "] to [" << path << "] in output" << std::endl;
				// rewrite to output domain
				*on = obj;
				return;
			}
		};

		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			// ignore deferred objects.
			if (obj && th)
			{
				depwalker dp;
				dp.parent = this;
				dp.source = obj;
				th->walk_dependencies(obj, &dp, false);
			}
		}
	};

	struct db_record_inserter : public putki::db::enum_i
	{
		putki::db::data *input;
		putki::db::data *output;

		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			// forward all objects directly to the output db. we can't clone them here or all
			// pointers to them will become wild.
			putki::db::copy_obj(input, output, path);
		}
	};
	
	// TODO TODO - Read this from external manifest instead / subsystem which can track
	//             a database of input files, particularly along with their aux refs.
	struct build_source_file : public putki::db::enum_i
	{
		putki::builder::build_context *context;
		putki::db::data *input;
		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			putki::builder::context_add_to_build(context, path);
		}
	};

	struct read_output : public putki::db::enum_i
	{
		std::string output_path;
		putki::runtime::descptr output_runtime;

		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			std::string outpath = output_path + path;
			char *ptr = th->write_into_buffer(output_runtime, obj, xbuf, xbuf + xbufSize);
			if (ptr)
			{
				std::cout << "     => Writing " << outpath << std::endl;
				putki::sys::mk_dir_for_path(outpath.c_str());
				std::ofstream out(outpath.c_str(), std::ios::binary);
				out.write(xbuf, ptr - xbuf);
			}
		}
	};

	struct write_cache_json : public putki::db::enum_i
	{
		putki::builder::data *builder;
		putki::db::data *db;
		std::string path_base;

		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			if (putki::db::is_aux_path(path)) {
				return;
			}

			// forward all objects directly to the output db. we can't clone them here or all
			// pointers to them will become wild.
			std::string out_path = path_base;
			out_path.append("/");
			out_path.append(path);
			out_path.append(".json");

			std::stringstream tmp;
			putki::write::write_object_into_stream(tmp, db, th, obj);

			putki::sys::mk_dir_for_path(out_path.c_str());
			std::ofstream f(out_path.c_str());
			f << tmp.str();
		}
	};

}

namespace putki
{
	namespace build
	{
		struct packaging_config
		{
			std::string package_path;
			runtime::descptr rt;
			build_db::data *bdb;
		};

		void post_build_ptr_update(db::data *input, db::data *output)
		{
			// Move all references from objects in the output
			domain_switch dsw;
			dsw.input = input;
			dsw.output = output;
			db::read_all_no_fetch(output, &dsw);
		}

		// merges objects from source into target.
		void post_build_merge_database(putki::db::data *source, db::data *target)
		{
			db_record_inserter ri;
			ri.input = source;
			ri.output = target;
			putki::db::read_all_no_fetch(source, &ri);
		}

		void do_build(putki::builder::data *builder, const char *single_asset)
		{
			if (single_asset) {
				std::cout << "=> Starting isolated build of " << single_asset << std::endl;
			}
			else{
				std::cout << "=> Starting full build" << std::endl;
			}

			db::data *input = putki::db::create();
			load_tree_into_db(builder::obj_path(builder), input);

			std::cout << "=> Loaded DB, building source files" << std::endl;

			db::data *output = putki::db::create();
			builder::build_context *ctx = builder::create_context(builder, input, output);

			// insert all source files into the build context's records.
			build_source_file bsf;
			bsf.context = ctx;
			bsf.input = input;
			if (single_asset)
			{
				type_handler_i *th;
				instance_t obj;
				if (db::fetch(input, single_asset, &th, &obj)) 
				{
					bsf.record(single_asset, th, obj);
				}
				else
				{
					std::cout << "Unable to resolve [" << single_asset << "]!" << std::endl;
				}
			}
			else
			{
				db::read_all_no_fetch(input, &bsf);
			}

			builder::context_finalize(ctx);
			builder::context_build(ctx);

			post_build_ptr_update(input, output);

			// GLOBAL PASS
			{
				db::data *global_out = db::create();
				builder::build_global_pass(builder, output, global_out);
				build::post_build_merge_database(global_out, output);
				db::free(global_out);
			}

			post_build_ptr_update(input, output);

			//
			std::cout << "=> Updating metadata on built items" << std::endl;
			for (unsigned int i=0;;i++)
			{
				const char *path = context_get_built_object(ctx, i);
				if (!path)
					break;
				if (context_was_read_from_cache(ctx, i))
					break;
				build_db::insert_metadata(builder::get_build_db(builder), output, path);
			}


			std::cout << "=> Writing final .json objects for cache." << std::endl;
			write_cache_json js;
			js.path_base = builder::built_obj_path(builder);
			js.db = output;
			js.builder = builder;
			for (unsigned int i=0;;i++)
			{
				const char *path = context_get_built_object(ctx, i);
				if (!path)
					break;
					
				if (context_was_read_from_cache(ctx, i))
					continue;
					
				type_handler_i *th;
				instance_t obj;
				if (db::fetch(output, path, &th, &obj)) 
				{
					std::cout << "  writing [" << path << "]" << std::endl;
					js.record(path, th, obj);
				}
				else
				{
					std::cout << "Unable to resolve [" << path << "]!" << std::endl;
				}
			}
			
			builder::context_destroy(ctx);

			if (single_asset)
			{
				std::cout << "=> Not packaging data in single mode." << std::endl;
				return;
			}

			std::cout << "=> Packaging data." << std::endl;

			char pkg_path[1024];
			sprintf(pkg_path, "%s/packages/", builder::out_path(builder));

			packaging_config pconf;
			pconf.package_path = pkg_path;
			pconf.rt = builder::runtime(builder);
			pconf.bdb = builder::get_build_db(builder);
			putki::builder::invoke_packager(output, &pconf);

			// there should be no objects outside these database now.
			db::free_and_destroy_objs(input);
			db::free_and_destroy_objs(output);
		}

		void full_build(putki::builder::data *builder)
		{
			do_build(builder, 0);
		}

		void single_build(putki::builder::data *builder, const char *single_asset)
		{
			do_build(builder, single_asset);
		}

		void commit_package(putki::package::data *package, packaging_config *packaging, const char *out_path)
		{
			std::string final_path = packaging->package_path + out_path;
			std::cout << "Saving package to [" << final_path << "] ..." << std::endl;

			long bytes_written = putki::package::write(package, packaging->rt, xbuf, xbufSize);
			std::cout << "Wrote " << bytes_written << " bytes" << std::endl;

			putki::package::debug(package, packaging->bdb);

			putki::sys::mk_dir_for_path(final_path.c_str());

			std::ofstream pkg(final_path.c_str(), std::ios::binary);
			pkg.write(xbuf, bytes_written);

			putki::package::free(package);
		}
	}
}

/*
   // API
   // Impl.
   void do_build_steps()
   {

   //	std::cout << "Writing output records..." << std::endl;
   //	read_output o;
   //	putki::db::read_all(output, &o);

   //	std::cout << "Building packages..." << std::endl;
   /	app_build_packages(output);


   //	std::cout << "Build done!" << std::endl;
   //}
 */
