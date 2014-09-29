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
	const unsigned long xbufSize = 512*1024*1024;
	char xbuf[xbufSize];


	struct domain_switch : public putki::db::enum_i
	{
		putki::db::data *input, *output;
		bool create_unresolved;
		bool traverse_children;
	
		domain_switch()
		{
			create_unresolved = true;
			traverse_children = false;
		}

		struct depwalker : public putki::depwalker_i
		{
			domain_switch *parent;
			putki::instance_t source;

			bool pointer_pre(putki::instance_t *on, const char *ptr_type)
			{
				// ignore nulls.
				if (!*on) {
					return false;
				}

				putki::type_handler_i *th;
				putki::instance_t obj = 0;

				const char *path = putki::db::pathof_including_unresolved(parent->input, *on);
				if (!path)
				{
					// this would mean the object exists neither in the input nor output domain.
					if (!putki::db::pathof_including_unresolved(parent->output, *on)) 
					{
						APP_ERROR("!!! A wild object appears! [" << *on << "]. Might be a [" << ptr_type << "]")
					}
					return false;
				}

				if (!putki::db::fetch(parent->output, path, &th, &obj))
				{
					if (parent->create_unresolved)
					{
						*on = putki::db::create_unresolved_pointer(parent->output, path);
						return false;
					}
					else
					{
						APP_WARNING("domain_switch: could not find [" << path << " in neither source nor output")
						return false;
					}
				}

				if (*on != obj)
				{
					*on = obj;
				}

				return true;
			}

			void pointer_post(putki::instance_t *on)
			{
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
				th->walk_dependencies(obj, &dp, traverse_children);
			}
		}
	};

	struct db_record_inserter : public putki::db::enum_i
	{
		putki::db::data *input;
		putki::db::data *output;

		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
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

			putki::sstream tmp;
			putki::write::write_object_into_stream(tmp, db, th, obj);

			putki::sys::mk_dir_for_path(out_path.c_str());
			putki::sys::write_file(out_path.c_str(), tmp.str().c_str(), tmp.str().size());
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
			std::string final_path;
			std::string final_manifest_path;
			std::string ptr_file;
			std::string ptr_file_content;
		};

		struct packaging_config
		{
			std::string package_path;
			runtime::descptr rt;
			build_db::data *bdb;
			builder::build_context *context;
			std::vector<pkg_conf> packages;
			bool make_patch;
		};

		void post_build_ptr_update(db::data *input, db::data *output)
		{
			// Move all references from objects in the output
			domain_switch dsw;
			dsw.input = input;
			dsw.output = output;
			db::read_all_no_fetch(output, &dsw);
		}

		void resolve_object(putki::db::data *source, const char *path)
		{
			type_handler_i *th;
			instance_t obj;
			if (!fetch(source, path, &th, &obj))
			{
				APP_WARNING("resolve[" << path << "] could not find object")
				return;
			}
			domain_switch dsw;
			dsw.input = source;
			dsw.output = source;
			dsw.traverse_children = true;
			dsw.record(path, th, obj);
		}

		// merges objects from source into target.
		void post_build_merge_database(putki::db::data *source, db::data *target)
		{
			db_record_inserter ri;
			ri.input = source;
			ri.output = target;
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
			
			bool make_patch = packaging->make_patch;
		
			// expect old packages to be there.
			pk.ptr_file = packaging->package_path + out_path + ".ptr";
			pk.ptr_file_content = out_path;
			pk.final_path = packaging->package_path + out_path;
			pk.final_manifest_path = packaging->package_path + out_path + ".manifest";

			std::vector<std::string> old_ones;
			if (make_patch)
			{
				for (int i=0;;i++)
				{
					sstream pkg_name;
					pkg_name << out_path;
					if (i > 0)
					{
						pkg_name << ".patch" << i;
					}
	
					std::string manifest_name(pkg_name.c_str());
					manifest_name.append(".manifest");
					
					pk.final_path = packaging->package_path + pkg_name.c_str();
					pk.final_manifest_path = packaging->package_path + manifest_name.c_str();

					std::ifstream f0(pk.final_path.c_str());
					std::ifstream f1(pk.final_manifest_path.c_str());
					
					// check for previous files.
					if (f0.good() && f1.good())
					{
						APP_DEBUG("Previous at " << pkg_name.c_str() << " and " << manifest_name);
						old_ones.push_back(pkg_name.c_str());
					}
					else
					{
						pk.ptr_file_content = pkg_name.c_str();
						break;
					}
				}
				
				if (old_ones.empty())
				{
					APP_DEBUG("Wanted to make patch for " << out_path << " but previous package does not exist!");
					make_patch = false;
				}
			}
			
			if (make_patch)
			{
				for (int i=old_ones.size()-1;i>=0;i--)
				{
					package::add_previous_package(package, packaging->package_path.c_str(), old_ones[i].c_str());
				}
				APP_DEBUG("Registered package " << out_path << " and it will be a patch [" << pk.final_path << "] with " << old_ones.size() << " previous files to use");
			}
			
			
					
			pk.pkg = package;
			pk.path = out_path;
			packaging->packages.push_back(pk);
			APP_DEBUG("Registered package " << out_path);
		}

		void write_package(pkg_conf *pk, packaging_config *packaging)
		{
			APP_DEBUG("Saving package to [" << pk->final_path << "]...")

			sstream mf;
			long bytes_written = putki::package::write(pk->pkg, packaging->rt, xbuf, xbufSize, packaging->bdb, mf);

			APP_INFO("Wrote " << pk->final_path << " (" << bytes_written << ") bytes")

			putki::sys::mk_dir_for_path(pk->final_path.c_str());
			putki::sys::mk_dir_for_path(pk->final_manifest_path.c_str());

			std::ofstream pkg(pk->final_path.c_str(), std::ios::binary);
			pkg.write(xbuf, bytes_written);
			pkg.close();
			
			std::ofstream pkg_mf(pk->final_manifest_path.c_str(), std::ios::binary);
			pkg_mf.write(mf.c_str(), mf.size());
			pkg_mf.close();
			
			std::ofstream ptr(pk->ptr_file.c_str());
			ptr << pk->ptr_file_content;
			ptr.close();
		}

		void do_build(putki::builder::data *builder, const char *single_asset, bool make_patch)
		{
			sys::mutex in_db_mtx, tmp_db_mtx, out_db_mtx;
			db::data *input = putki::db::create(0, &in_db_mtx);
			db::data *tmp = putki::db::create(input, &tmp_db_mtx);
			db::data *output = putki::db::create(tmp, &out_db_mtx);

			load_tree_into_db(builder::obj_path(builder), input);

			builder::build_context *ctx = builder::create_context(builder, input, tmp, output);

			APP_INFO("Application packager...")

			char pkg_path[1024];
			sprintf(pkg_path, "%s/packages/", builder::out_path(builder));
			packaging_config pconf;
			pconf.package_path = pkg_path;
			pconf.rt = builder::runtime(builder);
			pconf.bdb = builder::get_build_db(builder);
			pconf.context = ctx;
			pconf.make_patch = make_patch;
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

			// save built objects.
			write_cache_json js;
			js.path_base = builder::built_obj_path(builder);
			js.db = output;
			js.builder = builder;
			for (unsigned int i=0;;i++)
			{
				const char *path = context_get_built_object(ctx, i);
				if (!path)
					break;

				if (db::is_aux_path(path))
					continue;
				
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
			db::free_and_destroy_objs(tmp);
			db::free_and_destroy_objs(output);
			builder::context_destroy(ctx);
		}

		void full_build(putki::builder::data *builder, bool make_patch)
		{
			do_build(builder, 0, make_patch);
		}

		void single_build(putki::builder::data *builder, const char *single_asset)
		{
			do_build(builder, single_asset, false);
		}
	}
}

