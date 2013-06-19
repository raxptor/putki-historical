#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/app.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/runtime.h>

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
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
	{
		std::cerr << "WSA init failure" << std::endl;
		return 1;
	}
#endif

	// configure builder with app handlers.

	putki::runtime::descptr rt = putki::runtime::running();

	for (int i=1;i<argc;i++)
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
	}
	
	putki::builder::data *builder = putki::builder::create(putki::runtime::running());

	std::cout << "# Starting full build for platform [" << putki::runtime::desc_str(rt) << "]" << std::endl;

	char pkg_path[1024];
	sprintf(pkg_path, "out/%s/packages/", putki::runtime::desc_str(rt));

	putki::build::full_build(builder, "data/", "out/junk/temp/", pkg_path);

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
