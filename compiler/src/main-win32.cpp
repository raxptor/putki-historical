#include <parser/parse.h>
#include <generator/generator.h>
#include <putki/sys/files.h>
#include <sstream>
#include <iostream>
#include <string>

namespace
{
	const char *s_inpath;
	const char *s_outpath;
}

void file(const char *fullpath, const char *name)
{
	const char *ending = ".typedef";
	const unsigned int len = strlen(ending);
	std::string fn(name);
	if (fn.size() <= len)
		return;

	if (fn.substr(fn.size() - len, len) == ending)
	{
		std::string out_base = std::string(s_outpath) + std::string(fullpath).substr(strlen(s_inpath), strlen(fullpath) - strlen(s_inpath));
		out_base = out_base.substr(0, out_base.size() - len);
		
		std::cout << "File [" << name << "]" << std::endl;
		
		putki::parsed_file pf;
		putki::parse(fullpath, &pf);


		std::string rt_header = out_base + ".h";
		std::string rt_impl   = out_base + ".cpp";

		std::cout << " -> writing [" << rt_header << "] and [" << rt_impl << "]" << std::endl;
		//putki::write_runtime_header(&pf, putki::RUNTIME_CPP_WIN32, std::cout);		
	}
}


int main (int argc, char *argv[])
{
	if (argc > 2)
	{

		s_inpath = argv[1];
		s_outpath = argv[2];
		putki::sys::search_tree(argv[1], file);
	
		/*
		try 
		{
			putki::parsed_file pf;
			putki::parse(argv[1], &pf);

			putki::write_runtime_header(&pf, putki::RUNTIME_CPP_WIN32, std::cout);
			putki::write_runtime_impl(&pf, putki::RUNTIME_CPP_WIN32, std::cout);
		}
		catch (...)
		{
			std::cout << "Exception!" << std::endl;
		}
		*/
	}
	else
	{
		std::cerr << argv[0] << " <infile> <outpath-prefx>" << std::endl;
	}

	return 0;
}
