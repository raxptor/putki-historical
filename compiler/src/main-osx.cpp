#include <parser/parse.h>
#include <generator/generator.h>
#include <putki/sys/files.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
//#include <unistd.h>

namespace
{
	const char *s_inpath;
	const char *s_rt_outpath;
	const char *s_putki_outpath;
	const char *s_dll_outpath;
	std::stringstream s_bind_decl, s_bind_decl_dll, s_bind_calls, s_bind_calls_dll;
	std::stringstream s_blob_load_decl, s_blob_load_calls;
	
	int type_id = 0;
}

void write_out(putki::parsed_file & pf, const char *fullpath, const char *name, unsigned int len)
{
	std::string out_base = std::string(fullpath).substr(strlen(s_inpath), strlen(fullpath) - strlen(s_inpath));
	
	out_base = out_base.substr(0, out_base.size() - len);
	
	putki::sys::mk_dir_for_path((s_rt_outpath + out_base).c_str());
	putki::sys::mk_dir_for_path((s_putki_outpath + out_base).c_str());
	putki::sys::mk_dir_for_path((s_dll_outpath + out_base).c_str());
	
	std::cout << "File [" << name << "]" << std::endl;

	// runtime
	
	std::string rt_header = s_rt_outpath + out_base + ".h";
	std::string rt_impl   = s_rt_outpath + out_base + "_rt.cpp";
	
	std::ofstream f_rt_header(rt_header.c_str());
	std::ofstream f_rt_impl(rt_impl.c_str());
	
	std::cout << " -> writing [" << rt_header << "] and [" << rt_impl << "]" << std::endl;
	putki::write_runtime_header(&pf, putki::RUNTIME_CPP_WIN64, f_rt_header);
	putki::write_runtime_impl(&pf, putki::RUNTIME_CPP_WIN64, f_rt_impl);

	putki::write_runtime_blob_load_cases(&pf, s_blob_load_calls);
	putki::write_runtime_blob_load_decl(("outki/" + out_base.substr(1) + ".h").c_str(), s_blob_load_decl);
	
	// putki

	std::string putki_header = s_putki_outpath + out_base + ".h";
	std::string putki_impl   = s_putki_outpath + out_base + "_putki.cpp";
	
	std::ofstream f_putki_header(putki_header.c_str());
	std::ofstream f_putki_impl(putki_impl.c_str());
	
	std::cout << " -> writing [" << putki_header << "] and [" << putki_impl << "]" << std::endl;
	putki::write_putki_header(&pf, f_putki_header);
	putki::write_putki_impl(&pf, f_putki_impl);
	
	putki::write_bind_decl(&pf, s_bind_decl);
	putki::write_bind_calls(&pf, s_bind_calls);

	// editor
	std::string dll_impl = s_dll_outpath + out_base + "_impl.cpp";
	std::cout << " -> writing [" << dll_impl << "]" << std::endl;

	std::ofstream f_dll_impl(dll_impl.c_str());
	putki::write_dll_impl(&pf, f_dll_impl);
	putki::write_bind_decl_dll(&pf, s_bind_decl_dll);
	putki::write_bind_call_dll(&pf, s_bind_calls_dll);
	
}

void file(const char *fullpath, const char *name)
{
	const char *ending = ".typedef";
	const unsigned int len = (unsigned int)strlen(ending);
	std::string fn(name);
	if (fn.size() <= len)
		return;

	if (fn.substr(fn.size() - len, len) == ending)
	{
		type_id += 100;
		
		putki::parsed_file pf;
		putki::parse(fullpath, type_id, &pf);
		write_out(pf, fullpath, name, len);
	}
}


int main (int argc, char *argv[])
{
	s_inpath = "src";
	s_rt_outpath = "_gen/outki";
	s_putki_outpath = "_gen/putki";
	s_dll_outpath = "_gen/data-dll";
	
	const char *module_name = "test_project";

	// add internal records for packages
	
	putki::sys::search_tree(s_inpath, file);
	

	// bind calls
	std::ofstream f_bind((std::string(s_putki_outpath) + "/bind.cpp").c_str());        
	f_bind << "#include <putki/builder/typereg.h>" << std::endl << std::endl;
	f_bind << s_bind_decl.str() << std::endl;
	f_bind << "namespace putki {" << std::endl;
	f_bind << "void bind_" << module_name << "()" << std::endl << "{" << std::endl;
	f_bind << s_bind_calls.str() << std::endl;
	f_bind << "}" << std::endl;
	f_bind << "}" << std::endl;

	// bind dll calls
	std::ofstream f_bind_dll((std::string(s_putki_outpath) + "/bind-dll.cpp").c_str()); 
	f_bind_dll << "#include <putki/data-dll/dllinterface.h>" << std::endl;
	f_bind_dll << s_bind_decl_dll.str() << std::endl;
	f_bind_dll << "namespace putki {" << std::endl;
	f_bind_dll << "void bind_" << module_name << "_dll()" << std::endl << "{" << std::endl;
	f_bind_dll<< s_bind_calls_dll.str() << std::endl;
	f_bind_dll << "}" << std::endl;
	f_bind_dll << "}" << std::endl;
	
	// runtime switch case blob load
	std::ofstream f_switch((std::string(s_rt_outpath) + "/blobload.cpp").c_str());
	f_switch << s_blob_load_decl.str() << std::endl;
	f_switch << "#include <putki/blob.h>" << std::endl;
	f_switch << "#include <putki/types.h>" << std::endl;
	f_switch << "namespace outki {" << std::endl;
	f_switch << "char* post_blob_load_" << module_name << "(int type, depwalker_i *ptr_reg, char *begin, char *end)" << std::endl << "{" << std::endl;
	f_switch << "	switch (type) {" << std::endl;
	f_switch << s_blob_load_calls.str() << std::endl;
	f_switch << "		default: return 0;" << std::endl;
	f_switch << "	}" << std::endl;
	f_switch << "}" << std::endl;
	f_switch << "void bind_" << module_name << "_loaders()" << std::endl;
	f_switch << "{" << std::endl;
	f_switch << "   add_blob_loader(post_blob_load_" << module_name << ");" << std::endl;
	f_switch << "}" << std::endl;
	f_switch << "}" << std::endl;
	
 	return 0;
}
