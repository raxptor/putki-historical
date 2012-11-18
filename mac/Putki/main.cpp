#include <parser/parse.h>
#include <generator/generator.h>
#include <putki/sys/files.h>
#include <sstream>
#include <iostream>
#include <string>

void file(const char *fullpath, const char *name)
{
	const char *ending = ".typedef";
	const size_t len = strlen(ending);
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
    std::cout << "Putki compiler iOS verison." << std::endl;
	putki::sys::search_tree("../../src", file);
	return 0;
}
