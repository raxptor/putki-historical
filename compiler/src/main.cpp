#include <parser/typeparser.h>
#include <generator/generator.h>
#include <generator/indentedwriter.h>
#include <putki/sys/files.h>
#include <buildconfigs.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <deque>

//#include <unistd.h>

// from config
std::string g_module_name;
std::string g_loader_name;
int g_unique_id_counter = 1;
bool g_lazy_write = false;
bool g_ignore_config_typeid = false;

namespace
{
	std::deque<std::string> s_additional_paths;
	const char *s_inpath;
	const char *s_rt_outpath;
	const char *s_putki_outpath;
	const char *s_dll_outpath;
	const char *s_csharp_outki_outpath;
	const char *s_csharp_inki_outpath;
}

struct compile_context
{
	std::stringstream s_bind_decl, s_bind_decl_dll, s_bind_calls, s_bind_calls_dll;
	std::stringstream s_blob_load_decl, s_blob_load_calls;

	std::stringstream s_csharp_switch_case;
	std::stringstream s_csharp_switch_case_resolve;
	std::stringstream s_csharp_inki, s_csharp_outki;

	std::stringstream s_putki_master;
	std::stringstream s_runtime_master;

	bool s_wrote_anything;

	compile_context()
	{
		s_wrote_anything = false;
	}

	bool need_update(putki::parsed_file & pf, const char *filepath)
	{
		std::ifstream f(filepath);
		if (!f.good())
			return true;

		std::string l;
		getline(f, l);
		getline(f, l);

		if (l != pf.signature)
			std::cout << "Signature missing or not matching [" << filepath << "] " << l << std::endl;

		return l != pf.signature;
	}

	void write_signature_c(putki::parsed_file & pf, std::ostream & out)
	{
		out << "/*" << std::endl;
		out << pf.signature << std::endl;
		out << "*/" << std::endl;
	}

	void write_out(putki::parsed_file & pf, const char *fullpath, const char *name, unsigned int len)
	{
		std::string out_base = std::string(fullpath).substr(strlen(s_inpath), strlen(fullpath) - strlen(s_inpath));

		out_base = out_base.substr(0, out_base.size() - len);

		putki::sys::mk_dir_for_path((s_rt_outpath + out_base).c_str());
		putki::sys::mk_dir_for_path((s_putki_outpath + out_base).c_str());
		putki::sys::mk_dir_for_path((s_csharp_outki_outpath + out_base).c_str());
		putki::sys::mk_dir_for_path((s_dll_outpath + out_base).c_str());

		// runtime

		std::string rt_header = s_rt_outpath + out_base + ".h";
		std::string rt_impl   = s_rt_outpath + out_base + "_rt.cpp";
		std::string putki_header = s_putki_outpath + out_base + ".h";
		std::string putki_impl   = s_putki_outpath + out_base + "_inki.cpp";
		std::string csharp_code = s_csharp_outki_outpath + out_base + ".cs";
		std::string dll_impl = s_dll_outpath + out_base + "_impl.cpp";

		bool needupdate = need_update(pf, rt_header.c_str()) || need_update(pf, rt_impl.c_str())
		                  || need_update(pf, putki_header.c_str()) || need_update(pf, putki_impl.c_str())
		                  || need_update(pf, csharp_code.c_str())
		                  || need_update(pf, dll_impl.c_str());

		if (needupdate)
			s_wrote_anything = true;

		if (!g_lazy_write)
			needupdate = true;

		if (needupdate)
		{
			std::ofstream f_rt_header(rt_header.c_str());
			std::ofstream f_rt_impl(rt_impl.c_str());

			std::cout << " -> writing [" << rt_header << "] and [" << rt_impl << "]" << std::endl;
			write_signature_c(pf, f_rt_header);
			write_signature_c(pf, f_rt_impl);

			putki::write_runtime_header(&pf, 0, putki::indentedwriter(f_rt_header));
			putki::write_runtime_impl(&pf, 0, putki::indentedwriter(f_rt_impl));
		}

		putki::write_runtime_blob_load_cases(&pf, putki::indentedwriter(s_blob_load_calls));
		putki::write_runtime_blob_load_decl(("outki/" + out_base.substr(1) + ".h").c_str(), putki::indentedwriter(s_blob_load_decl));

		// putki

		if (needupdate)
		{
			std::ofstream f_putki_header(putki_header.c_str());
			std::ofstream f_putki_impl(putki_impl.c_str());

			std::cout << " -> writing [" << putki_header << "] and [" << putki_impl << "]" << std::endl;

			write_signature_c(pf, f_putki_header);
			write_signature_c(pf, f_putki_impl);

			putki::write_putki_header(&pf, putki::indentedwriter(f_putki_header));
			putki::write_putki_impl(&pf, putki::indentedwriter(f_putki_impl));
		}

		putki::write_bind_decl(&pf, putki::indentedwriter(s_bind_decl));
		putki::write_bind_calls(&pf, putki::indentedwriter(s_bind_calls));

		// editor
		if (needupdate)
		{
			std::cout << " -> writing [" << dll_impl << "]" << std::endl;

			std::ofstream f_dll_impl(dll_impl.c_str());
			write_signature_c(pf, f_dll_impl);

			f_dll_impl << "#pragma once" << std::endl;
			f_dll_impl << "#include <inki" << out_base << ".h>" << std::endl;

			putki::write_dll_impl(&pf, putki::indentedwriter(f_dll_impl));
		}

		s_putki_master << "#include \"data-dll" << out_base << "_impl.cpp\"" << std::endl;
		s_putki_master << "#include \"inki" << out_base << "_inki.cpp\"" << std::endl;

		putki::write_bind_decl_dll(&pf, putki::indentedwriter(s_bind_decl_dll));
		putki::write_bind_call_dll(&pf, putki::indentedwriter(s_bind_calls_dll));

		// csharp outki
		if (needupdate)
		{
			std::ofstream f_csharp_code(csharp_code.c_str());
			write_signature_c(pf, f_csharp_code);

			std::cout << " -> writing [" << csharp_code << "]" << std::endl;
			putki::indentedwriter casewriter(s_csharp_switch_case);
			putki::indentedwriter casewriter_resolve(s_csharp_switch_case_resolve);
			casewriter.indent(4);
			putki::write_csharp_runtime_class(&pf, putki::indentedwriter(f_csharp_code), casewriter, casewriter_resolve);
			casewriter.indent(-5);
		}

		// csharp inki
		{
			putki::write_csharp_inki_class(&pf, putki::indentedwriter(s_csharp_inki));
		}

	}

	void file(const char *fullpath, const char *name, void *userptr)
	{
		const char *ending = ".typedef";
		const unsigned int len = (unsigned int)strlen(ending);
		std::string fn(name);
		if (fn.size() <= len)
			return;

		if (fn.substr(fn.size() - len, len) == ending)
		{
			putki::parsed_file pf;
			pf.modulename = g_module_name;

			putki::parse(fullpath, name, &g_unique_id_counter, &pf);
			write_out(pf, fullpath, name, len);
		}
	}

	int compile()
	{
		std::ifstream config("putki-compiler.config");

		int id_counter;
		config >> g_module_name >> g_loader_name >> id_counter;

		if (!g_ignore_config_typeid)
			g_unique_id_counter = id_counter;

		std::string add;
		std::string dep_pfx("dep:");
		std::string config_pfx("config:");
		while (getline(config, add))
		{
			if (add.size() > dep_pfx.size() && add.substr(0, dep_pfx.size()) == dep_pfx)
			{
				std::string path = add.substr(dep_pfx.size(), add.size() - dep_pfx.size());
				s_additional_paths.push_back(path);
			}
			if (add.size() > config_pfx.size() && add.substr(0, config_pfx.size()) == config_pfx)
			{
				std::string config = add.substr(config_pfx.size(), add.size() - config_pfx.size());
				add_build_config(config.c_str());
			}
		}

		if (g_loader_name.empty() || g_unique_id_counter == 0)
		{
			std::cout << "Could not load settings from putki-compiler.config!" << std::endl;
			return 1;
		}

		s_inpath = "src";
		s_rt_outpath = "_gen/outki";
		s_putki_outpath = "_gen/inki";
		s_dll_outpath = "_gen/data-dll";
		s_csharp_outki_outpath = "_gen/outki_csharp";
		s_csharp_inki_outpath = "_gen/inki_csharp";

		// add internal records for packages

		s_wrote_anything = false;

		g_cur = this;
		putki::sys::search_tree(s_inpath, compile_context::s_file, 0);

		// only skip this when doing lazy writes because it might not have processed any files
		// and then we want to write out things.
		if (!s_wrote_anything && g_lazy_write)
			return 0;

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
		f_switch << "	switch (type) {"<< std::endl;
		f_switch << s_blob_load_calls.str() << std::endl;
		f_switch << "		default: return 0;"<< std::endl;
		f_switch << "	}"<< std::endl;
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
			std::ofstream f_switch((std::string(s_csharp_outki_outpath) + "/" + g_loader_name + "DataLoader.cs").c_str());
			f_switch << "namespace outki" << std::endl;
			f_switch << "{" << std::endl;
			f_switch << "	public class "<< g_loader_name << "DataLoader" << std::endl;
			f_switch << "	{"<< std::endl;
			f_switch << "		public static void ResolveFromPackage(int type, object obj, Putki.Package pkg)"<< std::endl;
			f_switch << "		{"<< std::endl;
			f_switch << "			switch (type)"<< std::endl;
			f_switch << "			{"<< std::endl;
			f_switch << s_csharp_switch_case_resolve.str() << std::endl;
			f_switch << "				default: return;"<< std::endl;
			f_switch << "			}"<< std::endl;
			f_switch << "		}"<< std::endl;

			f_switch << "		public static object LoadFromPackage(int type, Putki.PackageReader reader)"<< std::endl;
			f_switch << "		{"<< std::endl;
			f_switch << "			switch (type)"<< std::endl;
			f_switch << "			{"<< std::endl;
			f_switch << s_csharp_switch_case.str() << std::endl;
			f_switch << "				default: return null;"<< std::endl;
			f_switch << "			}"<< std::endl;
			f_switch << "		}"<< std::endl;
			f_switch << "	}"<< std::endl;
			f_switch << "}" << std::endl;
		}

		{
			std::string fn = std::string(s_csharp_inki_outpath) + "/" + g_module_name + ".cs";
			putki::sys::mk_dir_for_path(fn.c_str());
			std::ofstream f_cs_inki(fn.c_str());
			f_cs_inki << "using PutkEd;" << std::endl;
			f_cs_inki << std::endl;
			f_cs_inki << s_csharp_inki.str() << std::endl;
		}

		return 0;
	}

	static compile_context *g_cur;

	static void s_file(const char *fullpath, const char *name, void *userptr)
	{
		g_cur->file(fullpath, name, userptr);
	}
};

compile_context* compile_context::g_cur = 0;

int main (int argc, char *argv[])
{
	for (int i=1; i<argc; i++)
	{
		if (!strcmp(argv[i], "--lazy"))
			g_lazy_write = true;
		if (!strcmp(argv[i], "--ignore-config-typeid"))
			g_ignore_config_typeid = true;
	}
	
	while (true)
	{
		compile_context ctx;

		int r = ctx.compile();
		if (r)
		{
			std::cout << "Aborting on error." << std::endl;
			return r;
		}

		if (s_additional_paths.empty())
			return 0;

		putki::sys::chdir_push(s_additional_paths.front().c_str());
		s_additional_paths.pop_front();


	} while (!s_additional_paths.empty()) ;

}
