#include <parser/parse.h>
#include <generator/generator.h>
#include <sstream>
#include <iostream>

int main (int argc, char *argv[])
{
	if (argc > 1)
	{
		try 
		{
			putki::parsed_file pf;
			putki::parse(argv[1], &pf);

			putki::write_runtime_header(&pf, putki::RUNTIME_CPP_WIN32);
		}
		catch (...)
		{
			std::cout << "Exception!" << std::endl;
		}
	}

	return 0;
}
