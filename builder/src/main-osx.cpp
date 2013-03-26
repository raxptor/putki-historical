
#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/db.h>
#include <putki/sys/files.h>
#include <putki/runtime.h>

#include <iostream>
#include <fstream>
#include <string>

// Putki data builder.

// Application API
void bind_app_types();
void app_on_db_loaded(putki::db::data *db);
int app_builder_main();

namespace
{    
	putki::db::data *output = 0;
    const unsigned int xbufSize = 32*1024*1024;
    char xbuf[xbufSize];
    
    struct read_source : public putki::db::enum_i
    {
		void record(const char *path, putki::i_type_handler* th, putki::instance_t obj)
		{
			std::cout << "Processing [" << path << "]..." << std::endl;

			putki::db::insert(output, path, th, obj);
		}
	};

	struct read_output : public putki::db::enum_i
	{
		void record(const char *path, putki::i_type_handler* th, putki::instance_t obj)
		{
			std::string outpath = std::string("output/") + path;
			char *ptr = th->write_into_buffer(putki::runtime::RUNTIME_CPP_WIN32, obj, xbuf, xbuf + xbufSize);
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

	std::cout << "Writing output records..." << std::endl;

	read_output o;
	putki::db::read_all(output, &o);

	std::cout << "Build done!" << std::endl;
}

int main(int argc, char **argv)
{
	bind_app_types();

	return app_builder_main();
}
