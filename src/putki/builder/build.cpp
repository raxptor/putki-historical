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
#include <putki/builder/app.h>

#include <putki/sys/files.h>

#include <iostream>
#include <fstream>
#include <string>

namespace
{
	const unsigned long xbufSize = 32*1024*1024;
	char xbuf[xbufSize];
    
	struct build_source_file : public putki::db::enum_i
	{
		putki::db::data *input, *output;
		putki::builder::data *builder;
		
		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			std::cout << "Building source file [" << path << "]..." << std::endl;
			putki::builder::build_source_object(builder, input, path, output);
		}
	};
	
	struct domain_switch : public putki::db::enum_i
	{
		putki::db::data *input, *output;
		
		struct depwalker : public putki::depwalker_i
		{
			domain_switch *parent;
			putki::instance_t source;
			
			void pointer(putki::instance_t *on)
			{
				putki::type_handler_i *th;
				putki::instance_t obj = 0;
				
				const char *path = putki::db::pathof_including_unresolved(parent->input, *on);
				if (!path)
				{
					// this would mean the object exists neither in the input nor output domain.
					if (!putki::db::pathof_including_unresolved(parent->output, *on))
						std::cout << "!!! A wild object appears!" << std::endl;
					return;
				}
				
				if (!putki::db::fetch(parent->output, path, &th, &obj))
				{
					std::cout << "!!! Referenced object missing in output! !!!" << std::endl;
				}
				
				std::cout << "Updating reference in [" << putki::db::pathof(parent->output, source) << "] to [" << path << "] in output" << std::endl;
				// rewrite to output domain
				*on = obj;
			}
		};
		
		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			depwalker dp;
			dp.parent = this;
			dp.source = obj;
			th->walk_dependencies(obj, &dp, false);
		}
	};	
	

	struct read_output : public putki::db::enum_i
	{
		std::string output_path;
		putki::runtime output_runtime;
		
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

}

namespace putki
{
	namespace build
	{
		struct packaging_config
		{
			std::string package_path;
			putki::runtime rt;
		};
		
		void post_build_ptr_update(db::data *input, db::data *output)
		{
			// Move all references from objects in the output
			domain_switch dsw;
			dsw.input = input;
			dsw.output = output;
			db::read_all(output, &dsw);
		}
			
		void full_build(putki::builder::data *builder, const char *input_path, const char *output_path, const char *package_path)
		{
			std::cout << "=> Starting full build" << std::endl;
			
			db::data *input = putki::db::create();
			load_tree_into_db("data", input);
		
			std::cout << "=> Loaded DB, building source files" << std::endl;
			
			build_source_file bsf;
			bsf.input = input;
			bsf.output = db::create();
			bsf.builder = builder;
			
			// go!
			db::read_all(input, &bsf);
			
			
			std::cout << "=> Domain switching pointers" << std::endl;
			post_build_ptr_update(input, bsf.output);			

			std::cout << "=> Packaging data." << std::endl;
			
			packaging_config pconf;
			pconf.package_path = package_path;
			pconf.rt = builder::runtime(builder);
			app_build_packages(bsf.output, &pconf);			
		}

		void commit_package(putki::package::data *package, packaging_config *packaging, const char *out_path)
		{
			std::string final_path = packaging->package_path + out_path;
			std::cout << "Saving package to [" << final_path << "] ..." << std::endl;
	
			long bytes_written = putki::package::write(package, packaging->rt, xbuf, xbufSize);
			std::cout << "Wrote " << bytes_written << " bytes" << std::endl;

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
