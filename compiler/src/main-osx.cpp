#include <parser/typeparser.h>
#include <generator/generator.h>
#include <generator/indentedwriter.h>
#include <putki/sys/files.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>

//#include <unistd.h>

// from config
std::string g_module_name;
std::string g_loader_name;
int g_unique_id_counter = 0;

namespace
{
	const char *s_inpath;
	const char *s_rt_outpath;
	const char *s_putki_outpath;
	const char *s_dll_outpath;
	const char *s_csharp_outpath;

	std::stringstream s_bind_decl, s_bind_decl_dll, s_bind_calls, s_bind_calls_dll;
	std::stringstream s_blob_load_decl, s_blob_load_calls;

	std::stringstream s_csharp_switch_case;
	std::stringstream s_csharp_switch_case_resolve;

	std::stringstream s_putki_master;
	std::stringstream s_runtime_master;
	
	int type_id = 0;
}

void write_out(putki::parsed_file & pf, const char *fullpath, const char *name, unsigned int len)
{
	std::string out_base = std::string(fullpath).substr(strlen(s_inpath), strlen(fullpath) - strlen(s_inpath));
	
	out_base = out_base.substr(0, out_base.size() - len);	
	
	putki::sys::mk_dir_for_path((s_rt_outpath + out_base).c_str());
	putki::sys::mk_dir_for_path((s_putki_outpath + out_base).c_str());
	putki::sys::mk_dir_for_path((s_csharp_outpath + out_base).c_str());
	putki::sys::mk_dir_for_path((s_dll_outpath + out_base).c_str());	
	
	std::cout << "File [" << name << "]" << std::endl;

	// runtime
	
	std::string rt_header = s_rt_outpath + out_base + ".h";
	std::string rt_impl   = s_rt_outpath + out_base + "_rt.cpp";
	
	std::ofstream f_rt_header(rt_header.c_str());
	std::ofstream f_rt_impl(rt_impl.c_str());
	
	std::cout << " -> writing [" << rt_header << "] and [" << rt_impl << "]" << std::endl;
		
	putki::write_runtime_header(&pf, 0, putki::indentedwriter(f_rt_header));
	putki::write_runtime_impl(&pf, 0, putki::indentedwriter(f_rt_impl));

	putki::write_runtime_blob_load_cases(&pf, putki::indentedwriter(s_blob_load_calls));
	putki::write_runtime_blob_load_decl(("outki/" + out_base.substr(1) + ".h").c_str(), putki::indentedwriter(s_blob_load_decl));
	
	// putki

	std::string putki_header = s_putki_outpath + out_base + ".h";
	std::string putki_impl   = s_putki_outpath + out_base + "_inki.cpp";	
	
	std::ofstream f_putki_header(putki_header.c_str());
	std::ofstream f_putki_impl(putki_impl.c_str());
	
	std::cout << " -> writing [" << putki_header << "] and [" << putki_impl << "]" << std::endl;
	
	putki::write_putki_header(&pf, putki::indentedwriter(f_putki_header));
	putki::write_putki_impl(&pf, putki::indentedwriter(f_putki_impl));
	
	putki::write_bind_decl(&pf, putki::indentedwriter(s_bind_decl));
	putki::write_bind_calls(&pf, putki::indentedwriter(s_bind_calls));

	// editor
	std::string dll_impl = s_dll_outpath + out_base + "_impl.cpp";
	std::cout << " -> writing [" << dll_impl << "]" << std::endl;

	std::ofstream f_dll_impl(dll_impl.c_str());
	f_dll_impl << "#pragma once" << std::endl;
	f_dll_impl << "#include <inki" << out_base << ".h>" << std::endl;

	s_putki_master << "#include \"data-dll" << out_base << "_impl.cpp\"" << std::endl;
	s_putki_master << "#include \"inki" << out_base << "_inki.cpp\"" << std::endl;
	
	putki::write_dll_impl(&pf, putki::indentedwriter(f_dll_impl));

	putki::write_bind_decl_dll(&pf, putki::indentedwriter(s_bind_decl_dll));
	putki::write_bind_call_dll(&pf, putki::indentedwriter(s_bind_calls_dll));

	// csharp.

	std::string csharp_code = s_csharp_outpath + out_base + ".cs";
	std::ofstream f_csharp_code(csharp_code.c_str());
	
	std::cout << " -> writing [" << csharp_code << "]" << std::endl;
	putki::indentedwriter casewriter(s_csharp_switch_case);
	putki::indentedwriter casewriter_resolve(s_csharp_switch_case_resolve);
	casewriter.indent(4);
	putki::write_csharp_runtime_class(&pf, putki::indentedwriter(f_csharp_code), casewriter, casewriter_resolve);
	casewriter.indent(-5);
	
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
		putki::parsed_file pf;
		putki::parse(fullpath, name, &g_unique_id_counter, &pf);
		write_out(pf, fullpath, name, len);
	}
}


int main (int argc, char *argv[])
{
	std::ifstream config("putki-compiler.config");
	config >> g_module_name >> g_loader_name >> g_unique_id_counter;
	if (g_loader_name.empty() || g_unique_id_counter == 0)
	{
		std::cout << "Could not load settings from putki-compiler.config!" << std::endl;
		return 1;
	}

	s_inpath = "src";
	s_rt_outpath = "_gen/outki";
	s_putki_outpath = "_gen/inki";
	s_dll_outpath = "_gen/data-dll";
	s_csharp_outpath = "_gen/outki_csharp";
		
	// add internal records for packages
	
	putki::sys::search_tree(s_inpath, file);
	
	// bind calls
	std::ofstream f_bind((std::string(s_putki_outpath) + "/bind.cpp").c_str());        
	f_bind << "#include <putki/builder/typereg.h>" << std::endl << std::endl;
	f_bind << s_bind_decl.str() << std::endl;
	f_bind << "namespace inki {" << std::endl;
	f_bind << "void bind_" << g_module_name << "()" << std::endl << "{" << std::endl;
	f_bind << s_bind_calls.str() << std::endl;
	f_bind << "}" << std::endl;
	f_bind << "}" << std::endl;

	// bind dll calls
	std::ofstream f_bind_dll((std::string(s_putki_outpath) + "/bind-dll.cpp").c_str()); 
	f_bind_dll << s_bind_decl_dll.str() << std::endl;
	f_bind_dll << "namespace inki {" << std::endl;
	f_bind_dll << "void bind_" << g_module_name << "_dll()" << std::endl << "{" << std::endl;
	f_bind_dll<< s_bind_calls_dll.str() << std::endl;
	f_bind_dll << "}" << std::endl;
	f_bind_dll << "}" << std::endl;
	
	// runtime switch case blob load
	std::ofstream f_switch((std::string(s_rt_outpath) + "/blobload.cpp").c_str());
	f_switch << s_blob_load_decl.str() << std::endl;
	f_switch << "#include <putki/blob.h>" << std::endl;
	f_switch << "#include <putki/types.h>" << std::endl;
	f_switch << "namespace outki {" << std::endl;
	f_switch << "char* post_blob_load_" << g_module_name << "(int type, putki::depwalker_i *ptr_reg, char *begin, char *end)" << std::endl << "{" << std::endl;
	f_switch << "	switch (type) {" << std::endl;
	f_switch << s_blob_load_calls.str() << std::endl;
	f_switch << "		default: return 0;" << std::endl;
	f_switch << "	}" << std::endl;
	f_switch << "}" << std::endl;
	f_switch << "void bind_" << g_module_name << "_loaders()" << std::endl;
	f_switch << "{" << std::endl;
	f_switch << "   add_blob_loader(post_blob_load_" << g_module_name << ");" << std::endl;
	f_switch << "}" << std::endl;
	f_switch << "}" << std::endl;

	std::ofstream f_putki_master((std::string("_gen") + "/" + g_module_name + "-putki-master.cpp").c_str());
	f_putki_master << s_putki_master.str() << std::endl;
	f_putki_master << "#include \"inki/bind.cpp\"" << std::endl;
	f_putki_master << "#include \"inki/bind-dll.cpp\"" << std::endl;
	
	std::ofstream f_runtime_master((std::string("_gen") + "/" + g_module_name + "-runtime-master.cpp").c_str());
	f_runtime_master << s_runtime_master.str() << std::endl;
	f_runtime_master << "#include \"outki/blobload.cpp\"" << std::endl;


	// runtime c# switch case blob load
	{
		std::ofstream f_switch((std::string(s_csharp_outpath) + "/" + g_loader_name + "DataLoader.cs").c_str());
		f_switch << "namespace outki" << std::endl;
		f_switch << "{" << std::endl;
		f_switch << "	public class " << g_loader_name << "DataLoader" << std::endl;
		f_switch << "	{" << std::endl;
		f_switch << "		public static void ResolveFromPackage(int type, object obj, Putki.Package pkg)" << std::endl;
		f_switch << "		{" << std::endl;
		f_switch << "			switch (type)" << std::endl;
		f_switch << "			{" << std::endl;
		f_switch << s_csharp_switch_case_resolve.str() << std::endl;
		f_switch << "				default: return;" << std::endl;
		f_switch << "			}" << std::endl;
		f_switch << "		}" << std::endl;

		f_switch << "		public static object LoadFromPackage(int type, Putki.PackageReader reader)" << std::endl;
		f_switch << "		{" << std::endl;
		f_switch << "			switch (type)" << std::endl;
		f_switch << "			{" << std::endl;
		f_switch << s_csharp_switch_case.str() << std::endl;
		f_switch << "				default: return null;" << std::endl;
		f_switch << "			}" << std::endl;
		f_switch << "		}" << std::endl;
		f_switch << "	}" << std::endl;
		f_switch << "}" << std::endl;
	}


	
 	return 0;
}
