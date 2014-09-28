#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/builder/log.h>
#include <putki/runtime.h>

#include <stdint.h>
#include <cstdlib>

#if defined(USE_WINSOCK)
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "wsock32.lib")
	#include <windows.h>
#else
	#include <pthread.h>
#endif

#include <iostream>

putki::liveupdate::data *s_live_update = 0;

void liveupdate_thread_real(int socket)
{
	std::cout << "Hello from the thread, socket=" << socket << std::endl;
	putki::liveupdate::service_client(s_live_update, "data/", socket);

	std::cout << "Client exiting" << std::endl;
}


#if defined(USE_WINSOCK)

DWORD WINAPI liveupdate_thread(LPVOID arg)
{
	liveupdate_thread_real((int)arg);
	return 0;
}

#else

void* liveupdate_thread(void *arg)
{
	liveupdate_thread_real((intptr_t) arg);
	return 0;
}

#endif

int run_putki_builder(int argc, char **argv)
{
#if defined(USE_WINSOCK)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSA init failure" << std::endl;
		return 1;
	}
#endif

	// configure builder with app handlers.

	putki::runtime::descptr rt = putki::runtime::running();

	const char *single_asset = 0;
	const char *build_config = "Default";
	bool incremental = false;
	bool patch = false;
	int threads = 0;

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

	if (argc > 1 && !strcmp(argv[1], "--liveupdate"))
	{
		s_live_update = putki::liveupdate::start_server(0);
		while (true)
		{
			int s = putki::liveupdate::accept(s_live_update);

			intptr_t skt = s;

#if defined(USE_WINSOCK)
			CreateThread(0, 0, &liveupdate_thread, (void*)skt, 0, 0);
#else
			pthread_t thr;
			pthread_create(&thr, 0, &liveupdate_thread, (void*)skt);
#endif
		}
		putki::liveupdate::stop_server(s_live_update);

	}

	return 0;
}
