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
#include <putki/builder/log.h>

#include <putki/sys/files.h>
#include <putki/sys/thread.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>

namespace
{
	const unsigned long xbufSize = 32*1024*1024;
	char xbuf[xbufSize];


	struct domain_switch : public putki::db::enum_i
	{
		putki::db::data *input, *output;
		bool create_unresolved;
	
		domain_switch()
		{
			create_unresolved = true;
		}

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
					if (!putki::db::pathof_including_unresolved(parent->output, *on)) 
					{
						APP_ERROR("!!! A wild object appears! [" << *on << "]")
					}
					return;
				}

				if (!putki::db::fetch(parent->output, path, &th, &obj))
				{
					if (parent->create_unresolved)
					{
						*on = putki::db::create_unresolved_pointer(parent->output, path);
						return;
					}
					else
					{
						return;
					}
				}

				if (*on != obj)
				{
					*on = obj;
				}

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
		putki::db::data *trash;

		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			// if we happen to overwrite an object (when merging outputs into parent records with tmp objects),
			// we would like to know that it must be trashed.
			// APP_DEBUG("Copying " << path)
			if (trash)
			{
				putki::type_handler_i *_th;
				putki::instance_t _obj = 0;

				if (putki::db::fetch(output, path, &_th, &_obj, false))
				{
					// if we are overwriting with a deferred object, current object will be lost
					// if we are overwriting with a different object, current object will be lost
					if (!obj || obj != _obj)
					{
						APP_DEBUG("Trashing object " << path << " src=" << obj << " replacing=" << _obj)
						putki::db::insert(trash, path, _th, _obj);
					}
				}
			}

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

	struct write_cache_json : public putki::db::enum_i
	{
		putki::builder::data *builder;
		putki::db::data *db;
		std::string path_base;
		int written;

		write_cache_json() 
		{
			written = 0;
		}

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
			written++;
		}
	};

}

namespace putki
{
	namespace build
	{
		struct pkg_conf
		{
			package::data *pkg;
			std::string path;
		};

		struct packaging_config
		{
			std::string package_path;
			runtime::descptr rt;
			build_db::data *bdb;
			builder::build_context *context;
			std::vector<pkg_conf> packages;
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
		void post_build_merge_database(putki::db::data *source, db::data *target, db::data *trash)
		{
			db_record_inserter ri;
			ri.input = source;
			ri.output = target;
			ri.trash = trash;
			putki::db::read_all_no_fetch(source, &ri);

			domain_switch dsw;
			dsw.input = source;
			dsw.output = target;
			dsw.create_unresolved = true;
			db::read_all_no_fetch(source, &dsw);
		}

		void commit_package(putki::package::data *package, packaging_config *packaging, const char *out_path)
		{
			pkg_conf pk;
			pk.pkg = package;
			pk.path = out_path;
			packaging->packages.push_back(pk);
			APP_DEBUG("Registered package " << out_path);
		}

		void write_package(pkg_conf *pk, packaging_config *packaging)
		{
			 std::string final_path = packaging->package_path + pk->path;
			 APP_DEBUG("Saving package to [" << final_path << "]...")

			 long bytes_written = putki::package::write(pk->pkg, packaging->rt, xbuf, xbufSize);
			 APP_INFO("Wrote " << final_path << " (" << bytes_written << " bytes")

			 putki::package::debug(pk->pkg, packaging->bdb);

			 putki::sys::mk_dir_for_path(final_path.c_str());

			 std::ofstream pkg(final_path.c_str(), std::ios::binary);
			 pkg.write(xbuf, bytes_written);
		}

		void do_build(putki::builder::data *builder, const char *single_asset)
		{
			sys::mutex in_db_mtx, out_db_mtx;
			db::data *input = putki::db::create(0, &in_db_mtx);
			db::data *output = putki::db::create(0, &out_db_mtx);
			
			load_tree_into_db(builder::obj_path(builder), input);

			builder::build_context *ctx = builder::create_context(builder, input, output);

			APP_INFO("Application packager...")

			char pkg_path[1024];
			sprintf(pkg_path, "%s/packages/", builder::out_path(builder));
			packaging_config pconf;
			pconf.package_path = pkg_path;
			pconf.rt = builder::runtime(builder);
			pconf.bdb = builder::get_build_db(builder);
			pconf.context = ctx;
			putki::builder::invoke_packager(output, &pconf);

			// Required assets
			std::set<std::string> req;
			for (unsigned int i=0;i!=pconf.packages.size();i++)
			{
				for (unsigned int j=0;;j++)
				{
					const char *path = package::get_needed_asset(pconf.packages[i].pkg, j);
					if (path)
						req.insert(path);
					else
						break;
				}
			}

			std::set<std::string>::iterator j = req.begin();
			while (j != req.end())
			{
				context_add_to_build(ctx, (*j++).c_str());
			}

			builder::context_finalize(ctx);
			builder::context_build(ctx);

			post_build_ptr_update(input, output);

			/*
			// GLOBAL PASS
			{
				db::data *global_out = db::create();
				builder::build_global_pass(builder, output, global_out);
				build::post_build_merge_database(global_out, output, junk);
				db::free(global_out);
			}
			post_build_ptr_update(input, output);
			*/

			write_cache_json js;
			js.path_base = builder::built_obj_path(builder);
			js.db = output;
			js.builder = builder;
			for (unsigned int i=0;;i++)
			{
				const char *path = context_get_built_object(ctx, i);
				if (!path)
					break;

				type_handler_i *th;
				instance_t obj;
				if (db::fetch(output, path, &th, &obj, false))
				{
					js.record(path, th, obj);
				}
			}

			if (js.written > 0)
			{
				APP_INFO("Wrote " << js.written << " JSON objects to output")
			}

			APP_INFO("Done building. Writing packages")

			for (unsigned int i=0;i!=pconf.packages.size();i++)
			{
				write_package(&pconf.packages[i], &pconf);
				putki::package::free(pconf.packages[i].pkg);
			}

			// there should be no objects outside these database now.
			db::free_and_destroy_objs(input);
			db::free_and_destroy_objs(output);
			builder::context_destroy(ctx);
		}

		void full_build(putki::builder::data *builder)
		{
			do_build(builder, 0);
		}

		void single_build(putki::builder::data *builder, const char *single_asset)
		{
			do_build(builder, single_asset);
		}
	}
}

