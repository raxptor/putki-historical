
#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/db.h>
#include <putki/builder/package.h>
#include <putki/sys/files.h>
#include <putki/runtime.h>

#include <iostream>
#include <fstream>
#include <string>

// Putki data builder.

// Application API
void app_bind_types();
void app_on_db_loaded(putki::db::data *db);
int app_builder_main();
void app_build_packages(putki::db::data *out);

namespace
{
	std::string output_path = "output/win32/out-db/";
	std::string package_path = "output/win32/packages/";

	putki::runtime output_runtime = putki::RUNTIME_CPP_WIN32;

	putki::db::data *output = 0;
	const unsigned long xbufSize = 32*1024*1024;
	char xbuf[xbufSize];
    
    struct read_source : public putki::db::enum_i
    {
		void record(const char *path, putki::type_handler_i* th, putki::instance_t obj)
		{
			std::cout << "Processing [" << path << "]..." << std::endl;

			putki::db::insert(output, path, th, obj);
		}
	};

	struct read_output : public putki::db::enum_i
	{
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

// API
void commit_package(putki::package::data *package, const char *out_path)
{
	std::string final_path = package_path + out_path;
	std::cout << "Saving package to [" << final_path << "] ..." << std::endl;
	
	long bytes_written = putki::package::write(package, output_runtime, xbuf, xbufSize);
	std::cout << "Wrote " << bytes_written << " bytes" << std::endl;

	putki::sys::mk_dir_for_path(final_path.c_str());

	std::ofstream pkg(final_path.c_str(), std::ios::binary);
	pkg.write(xbuf, bytes_written);
	
	putki::package::free(package);
}

// Impl.
void do_build_steps()
{
	putki::db::data *input = putki::db::create();
	putki::init_db_with_source("data", input);
	app_on_db_loaded(input);

	std::cout << "Building..." << std::endl;

	output = putki::db::create();

	read_source s;
	putki::db::read_all(input, &s);

/*
	std::cout << "Writing output records..." << std::endl;
	read_output o;
	putki::db::read_all(output, &o);
*/
	std::cout << "Building packages..." << std::endl;
	app_build_packages(output);
	

	std::cout << "Build done!" << std::endl;
}

int main(int argc, char **argv)
{
	app_bind_types();

	return app_builder_main();
}
