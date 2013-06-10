#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/app.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/runtime.h>

#ifndef _WIN32
#include <pthread.h>
#else
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#include <windows.h>
#endif

#include <iostream>

putki::liveupdate::data *s_live_update = 0;

void liveupdate_thread_real(int socket)
{
	std::cout << "Hello from the thread, socket=" << socket << std::endl;
	putki::liveupdate::service_client(s_live_update, "data/", socket);
	
	std::cout << "Client exiting" << std::endl;
}


#ifdef _WIN32

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
#if defined(_WIN32)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
	{
		std::cerr << "WSA init failure" << std::endl;
		return 1;
	}
#endif

	// configure builder with app handlers.
	putki::builder::data *builder = putki::builder::create(putki::RUNTIME_CPP_WIN32);
	app_register_handlers(builder);
	
	putki::build::full_build(builder, "data/", "out/win32/temp/", "out/win32/packages/");
	putki::builder::free(builder);
	
	if (argc > 1 && !strcmp(argv[1], "--liveupdate"))
	{
		s_live_update = putki::liveupdate::start_server(0);
		while (true)
		{
			int s = putki::liveupdate::accept(s_live_update);
			
			intptr_t skt = s;

#ifdef _WIN32
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
