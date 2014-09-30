#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/builder/log.h>
#include <putki/runtime.h>

#include <stdint.h>
#include <cstdlib>
#include <iostream>


int run_putki_builder(int argc, char **argv)
{

	// configure builder with app handlers.


	const char *single_asset = 0;
	const char *build_config = "Default";
	bool incremental = false;
	bool patch = false;
	int threads = 0;
	bool liveupdate = false;

	std::string runtime_name;

#if defined(BUILDER_DEFAULT_RUNTIME)
	#define BXSTR(x) BSTR(x)
	#define BSTR(x) #x
	runtime_name = BXSTR(BUILDER_DEFAULT_RUNTIME);
	putki::runtime::descptr rt = 0;
#else
	putki::runtime::descptr rt = putki::runtime::running();
#endif

	// putki::runtime::get(j);

	for (int i=1; i<argc; i++)
	{
		if (!strcmp(argv[i], "--csharp"))
		{
			static putki::runtime::desc r;
			r.platform = putki::runtime::PLATFORM_CSHARP;
			r.language = putki::runtime::LANGUAGE_CSHARP;
			r.ptrsize = 2;
			r.low_byte_first = true;
			rt = &r;
		}
		else if (!strcmp(argv[i], "--asset"))
		{
			if (i+1 < argc)
				single_asset = argv[++i];
		}
		else if (!strcmp(argv[i], "--config"))
		{
			if (i+1 < argc)
				build_config = argv[++i];
		}
		else if (!strcmp(argv[i], "--platform"))
		{
			if (i+1 < argc)
				runtime_name = argv[++i];
		}
		else if (!strcmp(argv[i], "--patch"))
		{
			patch = true;
		}
		else if (!strcmp(argv[i], "--incremental"))
		{
			incremental = true;
		}
		else if (!strcmp(argv[i], "--threads"))
		{
			if (i+1 < argc)
				threads = atoi(argv[i+1]);
		}
		else if (!strcmp(argv[i], "--liveupdate"))
		{
			liveupdate = true;
		}
		else if (!strcmp(argv[i], "--no-color"))
		{
			putki::set_use_ansi_color(false);
		}
		else if (!strcmp(argv[i], "--loglevel"))
		{
			bool match = false;
			if (i+1 < argc)
			{
				struct lvl {
					const char *name;
					putki::LogType type;
				} levels[] = {
					{"debug", putki::LOG_DEBUG},
					{"info", putki::LOG_INFO},
					{"warning", putki::LOG_WARNING},
					{"error", putki::LOG_ERROR}
				};

				for (int q=0;q<4;q++)
				{
					if (!strcmp(argv[i+1], levels[q].name))
					{
						putki::set_loglevel(levels[q].type);
						match = true;
					}
				}
			}

			if (!match)
			{
				std::cerr << "--loglevel debug/info/warning/error" << std::endl;
				exit(1);
			}
		}
	}

	if (!rt || !runtime_name.empty())
	{
		for (int j=0;;j++)
		{
			putki::runtime::descptr r = putki::runtime::get(j);
			if (!r)
			{
				std::cerr << "Unknown runtime " << runtime_name << std::endl;
				return -1;
			}
			else
			{
				if (!strcmp(putki::runtime::desc_str(r), runtime_name.c_str()))
				{
					rt = r;
					break;
				}
			}
		}
	}



	// reload build database if incremental build
	putki::builder::data *builder = putki::builder::create(rt, ".", !incremental, build_config, threads);

	if (single_asset)
	{
		APP_WARNING("Single building is experimental and may get the build database out of sync.")
		putki::build::single_build(builder, single_asset);
	}
	else
	{
		putki::build::full_build(builder, patch);
		putki::builder::write_build_db(builder);
	}

	putki::builder::free(builder);

	if (liveupdate)
	{
		putki::liveupdate::data *lu = putki::liveupdate::create();
		putki::liveupdate::run_server(lu);
		putki::liveupdate::free(lu);
	}

	return 0;
}
