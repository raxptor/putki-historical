
#include <putki/builder/typereg.h>
#include <putki/builder/source.h>
#include <putki/builder/db.h>
#include <putki/sys/files.h>

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
	const unsigned int xbufSize = 32*1024*1024;
	char xbuf[xbufSize];

	void on_record(const char *path, putki::i_type_handler* th, putki::type_inst obj)
	{
		std::cout << "Processing [" << path << "]..." << std::endl;

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
}

// Impl.
void do_build_steps()
{
	putki::db::data *d = putki::db::create();
	putki::init_db_with_source("data", d);
	app_on_db_loaded(d);

	std::cout << "Building..." << std::endl;
	putki::db::read_all(d, on_record);
	std::cout << "Build done!" << std::endl;
}

int main(int argc, char **argv)
{
	bind_app_types();

	return app_builder_main();
}
