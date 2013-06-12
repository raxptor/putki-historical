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
	
#if defined(_WIN32)
	putki::builder::data *builder = putki::builder::create(putki::RUNTIME_CPP_WIN32);
	putki::build::full_build(builder, "data/", "out/win32/temp/", "out/win32/packages/");
#elif defined(__APPLE__) && defined(__amd64__)
	putki::builder::data *builder = putki::builder::create(putki::RUNTIME_CPP_WIN64);
	putki::build::full_build(builder, "data/", "out/mac64/temp/", "out/mac64/packages/");
#endif
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
