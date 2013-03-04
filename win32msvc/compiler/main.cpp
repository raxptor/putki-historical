#include <parser/parse.h>
#include <generator/generator.h>
#include <putki/sys/files.h>
#include <sstream>
#include <iostream>
#include <string>

void file(const char *fullpath, const char *name)
{
	const char *ending = ".typedef";
	const unsigned int len = strlen(ending);
	std::string fn(name);
	if (fn.size() <= len)
		return;

	if (fn.substr(fn.size() - len, len) == ending)
	{
		std::cout << "File [" << name << "] => opening " << fullpath << std::endl;
		putki::parsed_file pf;
		putki::parse(fullpath, &pf);
		putki::write_runtime_header(&pf, putki::RUNTIME_CPP_WIN32);
	}
}

int main (int argc, char *argv[])
{
	if (argc > 1)
	{
		try 
		{
			putki::parsed_file pf;
			putki::parse(argv[1], &pf);

			putki::write_runtime_header(&pf, putki::RUNTIME_CPP_WIN32);
			putki::write_runtime_impl(&pf, putki::RUNTIME_CPP_WIN32);
		}
		catch (...)
		{
			std::cout << "Exception!" << std::endl;
		}
	}

	return 0;
}
