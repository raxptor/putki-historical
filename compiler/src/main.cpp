#include <parser/typeparser.h>
#include <parser/treeparser.h>
#include <parser/resolve.h>

#include <putki/sys/files.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <generator/generator.h>
#include <writetools/indentedwriter.h>

void save_stream(std::string path, std::stringstream &str)
{
	char *tmp = new char[str.str().size()];
	std::ifstream p(path.c_str());
	if (p.good())
	{
		p.read(tmp, str.str().size());
		if (p.gcount() == str.str().size())
		{
			if (!memcmp(tmp, str.str().c_str(), str.str().size()))
			{
				std::cout << "Skipped writing " << path << ", reason: unchanged content." << std::endl;
				return;
			}
		}
	}
	p.close();
	delete [] tmp;

	putki::sys::mk_dir_for_path(path.c_str());
	std::ofstream f(path.c_str());
	f << str.str();
	std::cout << "Wrote " << path << std::endl;
}

void generate_project(putki::project *p)
{
	std::string out_base(p->start_path);
	out_base.append("/_gen");

	std::string outki_base(out_base + "/outki");
	std::string inki_base(out_base + "/inki");
	std::string editor_base(out_base + "/editor");
	std::string csharp_outki(out_base + "/outki_csharp");
	std::string csharp_inki(out_base + "/inki_csharp");

	std::stringstream rt_blob_load_calls;
	std::stringstream rt_blob_load_header;

	std::stringstream bind_decl, bind_decl_dll;
	std::stringstream bind_calls, bind_calls_dll;

	std::stringstream inki_master, runtime_master;

	std::stringstream csharp_switch_case;
	std::stringstream csharp_switch_case_resolve;
	std::stringstream csharp_runtime, csharp_inki_code;

	for (int i=0;i!=p->files.size();i++)
	{
		putki::parsed_file *pf = &p->files[i];

		std::string subpath = pf->sourcepath.substr(p->base_path.size()+1);
		std::string rt_path = outki_base + "/" + subpath;
		std::string inki_path = inki_base + "/" + subpath;
		std::string editor_path = editor_base + "/" + subpath;

		// c++ runtime header
		std::stringstream rt_header;
		putki::write_runtime_header(pf, 0, putki::indentedwriter(rt_header));
		save_stream(rt_path + ".h", rt_header);

		// c++ runtime implementation
		std::stringstream rt_impl;
		putki::write_runtime_impl(pf, 0, putki::indentedwriter(rt_impl));
		save_stream(rt_path + ".cpp", rt_impl);

		// c++ runtime blob load calls
		putki::indentedwriter iw(rt_blob_load_calls);
		iw.indent(3);
		putki::write_runtime_blob_load_cases(pf, iw);
		rt_blob_load_header << "#include <outki/" << subpath << ".h>\n";

		// c++ inki header
		std::stringstream inki_header;
		putki::write_putki_header(pf, putki::indentedwriter(inki_header));
		save_stream(inki_path + ".h", inki_header);

		// c++ runtime implementation
		std::stringstream inki_impl;
		putki::write_putki_impl(pf, putki::indentedwriter(inki_impl));
		save_stream(inki_path + ".cpp", inki_impl);

		// csharp runtime
		putki::indentedwriter casewriter(csharp_switch_case);
		putki::indentedwriter casewriter_resolve(csharp_switch_case_resolve);
		casewriter.indent(4);
		putki::write_csharp_runtime_class(pf, putki::indentedwriter(csharp_runtime), casewriter, casewriter_resolve);
		casewriter.indent(-5);

		// dll impl
		std::stringstream dll_impl;
		dll_impl << "#include <inki/" << subpath << ".h>\n";
		putki::write_dll_impl(pf, putki::indentedwriter(dll_impl));
		save_stream(editor_path + ".cpp", dll_impl);

		// bindings
		putki::write_bind_decl(pf, putki::indentedwriter(bind_decl));
		putki::write_bind_calls(pf, putki::indentedwriter(bind_calls));
		putki::write_bind_decl_dll(pf, putki::indentedwriter(bind_decl_dll));
		putki::write_bind_call_dll(pf, putki::indentedwriter(bind_calls_dll));

		inki_master << "#include \"inki/" << subpath << ".cpp\"\n";
		inki_master << "#include \"editor/" << subpath << ".cpp\"\n";
		runtime_master << "#include \"outki/" << subpath << ".cpp\"\n";

		// csharp inki
		putki::write_csharp_inki_class(pf, putki::indentedwriter(csharp_inki_code));
	}

	// bind dll calls
	{
		std::stringstream bind_editor;
		bind_editor << bind_decl_dll.str() << std::endl;

		putki::indentedwriter iw(bind_editor);
		iw.line() << "namespace inki";
		iw.line() << "{";
		iw.indent(1);
		iw.line() << "void bind_" << p->module_name << "_editor()";
		iw.line() << "{";
		iw.indent(1);
		iw.line();
		bind_editor << bind_calls_dll.str();
		iw.indent(-1);
		iw.line() << "}";
		iw.indent(-1);
		iw.line() << "}";
		save_stream(editor_base + "/bind.cpp", bind_editor);
		inki_master << "#include \"editor/bind.cpp\"\n";
	}

	// bind calls
	{
		std::stringstream bind;
		bind << bind_decl.str() << std::endl;

		putki::indentedwriter iw(bind);
		iw.line() << "namespace inki";
		iw.line() << "{";
		iw.indent(1);
		iw.line() << "void bind_" << p->module_name << "()";
		iw.line() << "{";
		iw.indent(1);
		iw.line();
		bind << bind_calls.str();
		iw.indent(-1);
		iw.line() << "}";
		iw.indent(-1);
		iw.line() << "}";
		save_stream(inki_base + "/bind.cpp", bind);

		inki_master << "#include \"inki/bind.cpp\"\n";
	}

	// c++ runtime blob switcher
	{
		std::stringstream output;
		output << rt_blob_load_header.str();
		putki::indentedwriter iw(output);
		iw.line() << "#include <putki/blob.h>";
		iw.line() << "#include <putki/types.h>";
		iw.line();
		iw.line() << "namespace outki";
		iw.line() << "{";
		iw.indent(1);
		iw.line() << "char* post_blob_load_" << p->module_name << "(int type, putki::depwalker_i *ptr_reg, char *begin, char *end)";
		iw.line() << "{";
		iw.indent(1);
		iw.line() << "switch (type)";
		iw.line() << "{";
		output << rt_blob_load_calls.str();
		iw.line(1) << "default:";
		iw.line(2) << "return 0;";
		iw.line() << "}";
		iw.indent(-1);
		iw.line() << "}";
		iw.line();
		iw.line() << "void bind_" << p->module_name << "_loaders()";
		iw.line() << "{";
		iw.line(1) << "add_blob_loader(post_blob_load_" << p->module_name << ");";
		iw.line() << "}" << std::endl;
		iw.indent(-1);
		iw.line() << "}";
		save_stream(outki_base + "/blobload.cpp", output);

		runtime_master << "#include \"outki/blobload.cpp\"\n";
	}

	save_stream(out_base + "/" + p->module_name + "-inki-master.cpp", inki_master);
	save_stream(out_base + "/" + p->module_name + "-runtime-master.cpp", runtime_master);

	// runtime c# switch case blob load
	{
		std::stringstream csharp_outki_loader;
		putki::indentedwriter iw(csharp_outki_loader);
		iw.line() << "namespace outki" << std::endl;
		iw.line() << "{";
		iw.line() << "	public class "<< p->loader_name << "DataLoader";;
		iw.line() << "	{";
		iw.line() << "		public static void ResolveFromPackage(int type, object obj, Putki.Package pkg)";
		iw.line() << "		{";;
		iw.line() << "			switch (type)";
		iw.line() << "			{";
		iw.line() << csharp_switch_case_resolve.str();
		iw.line() << "				default: return;";
		iw.line() << "			}";
		iw.line() << "		}";
		iw.line() << "		public static object LoadFromPackage(int type, Putki.PackageReader reader)";
		iw.line() << "		{";
		iw.line() << "			switch (type)";
		iw.line() << "			{";
		iw.line() << csharp_switch_case.str() ;
		iw.line() << "				default: return null;";
		iw.line() << "			}";
		iw.line() << "		}";
		iw.line() << "	}";
		iw.line() << "}" ;
		save_stream(csharp_outki + "/" + p->loader_name + "DataLoader.cs", csharp_outki_loader);
	}

	save_stream(csharp_outki + "/" + p->module_name + ".cs", csharp_runtime);

	{
		std::stringstream ik;
		ik << "using PutkEd;\n";
		ik << "\n";
		ik << csharp_inki_code.str();
		save_stream(csharp_inki + "/" + p->module_name + ".cs", ik);
	}
}

int main (int argc, char *argv[])
{
	putki::grand_parse parse;
	if (!putki::parse_all_with_deps(&parse))
	{
		std::cerr << "Aborting on parse error." << std::endl;
		return -1;
	}

	putki::resolved_parse res;
	if (!putki::resolve_parse(&parse, &res))
	{
		std::cerr << "Aborting on resolve error." << std::endl;
		return -1;
	}

	for (int i=0;i!=parse.projects.size();i++)
	{
		generate_project(&parse.projects[i]);
	}

	return 0;
}
