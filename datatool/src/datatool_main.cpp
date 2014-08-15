#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/typereg.h>
#include <putki/liveupdate/liveupdate.h>
#include <putki/runtime.h>
#include <stdint.h>
#include <iostream>

int run_putki_datatool(int argc, char **argv)
{
	std::cout << "Putki data tool!" << std::endl;
	
	for (unsigned int i=0;true;i++) {
		putki::type_handler_i *type = putki::typereg_get_handler_by_index(i);
		if (!type) {
			std::cout << "Got a total of " << i << " types." << std::endl;
			break;
		}
	}

	

	return 0;
}
