#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/builder/log.h>
#include <putki/runtime.h>

#include <stdint.h>
#include <cstdlib>

#include <pthread.h>

#include <iostream>

putki::liveupdate::data *s_live_update = 0;

void liveupdate_thread_real(int socket)
{
	std::cout << "Hello from the thread, socket=" << socket << std::endl;
	putki::liveupdate::service_client(s_live_update, "data/", socket);

	std::cout << "Client exiting" << std::endl;
}

void* liveupdate_thread(void *arg)
{
	liveupdate_thread_real((intptr_t) arg);
	return 0;
}

int run_putki_builder(int argc, char **argv)
{

	// configure builder with app handlers.

	putki::runtime::descptr rt = putki::runtime::running();

	const char *single_asset = 0;
	const char *build_config = "Default";
	bool incremental = false;
	bool patch = false;
	int threads = 0;
	bool liveupdate = false;

	for (int i=1; i<argc; i++)
	{
		if (!strcmp(argv[i], "--csharp"))
		{
			static putki::runtime::desc r;
			r.platform = putki::runtime::PLATFORM_CSHARP;
			r.language = putki::runtime::LANGUAGE_CSHARP;
			r.ptrsize = 4;
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
		putki::liveupdate::editor_listen_thread();
	}

	return 0;
}
