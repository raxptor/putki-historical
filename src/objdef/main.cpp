#include <parser/parse.h>
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
		}
		catch (...)
		{
			std::cout << "Exception!" << std::endl;
		}
	}

	return 0;
}
