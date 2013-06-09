#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/app.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/runtime.h>
#include <pthread.h>

#include <iostream>

void* liveupdate_thread(void *arg)
{
	intptr_t s = (intptr_t) arg;
	
	std::cout << "Hello from the thread, socket=" << s << std::endl;
	putki::liveupdate::service_client("data/", s);
	
	std::cout << "Client exiting" << std::endl;
	return 0;
}

int run_putki_builder(int argc, char **argv)
{
	// configure builder with app handlers.
	putki::builder::data *builder = putki::builder::create(putki::RUNTIME_CPP_WIN32);
	app_register_handlers(builder);
	
	putki::build::full_build(builder, "data/", "out/win32/temp/", "out/win32/packages/");
	putki::builder::free(builder);
	
	if (argc > 1 && !strcmp(argv[1], "--liveupdate"))
	{
		putki::liveupdate::data *d = putki::liveupdate::start_server();
		while (true)
		{
			int s = putki::liveupdate::accept(d);
			
			intptr_t skt = s;
			pthread_t thr;
			pthread_create(&thr, 0, &liveupdate_thread, (void*)skt);
		}
		putki::liveupdate::stop_server(d);
	
	}
	
	return 0;
}
