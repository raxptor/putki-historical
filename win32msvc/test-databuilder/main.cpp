#include <putki/cpp-runtime/resolve.h>
#include <putki/builder-config.h>
#include <iostream>

int main (int argc, char *argv[])
{	
	putki::data_builder_cfg::load("../../test-project/putki.config");

	std::cout << "I am building from [" << putki::data_builder_cfg::input_path() << "]" << std::endl;
	std::cout << "I am building to   [" << putki::data_builder_cfg::output_path() << "]" << std::endl;
	return 0;
}
