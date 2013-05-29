#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{

	void write_dll_impl(putki::parsed_file *file, std::ostream &out)
	{
		out << "#include <putki/data-dll/dllinterface.h>" << std::endl;
		out << "#include <putki/data-dll/dllinternal.h>" << std::endl;
		out << std::endl;
		out << "namespace putki {" << std::endl;
		out << std::endl;

		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			out << "struct ed_type_handler_" << s->name << " : public putki::ext_type_handler_i {" << std::endl;
			out << "	const char *name() { return \"" << s->name << "\"; }" << std::endl;
			out << "};" << std::endl;
			out << std::endl;

			out << "__declspec(dllexport) putki::ext_type_handler_i * __cdecl get_type_handler_" << s->name << "()" << std::endl;
			out << "{" << std::endl;
			out << "	static ed_type_handler_" << s->name << " impl;" << std::endl;
			out << "	return &impl;" << std::endl;
			out << "}" << std::endl;
			out << std::endl;
			out << std::endl;
			out << "void bind_type_" << s->name << "_dll()" << std::endl;
			out << "{" << std::endl;
			out << "	putki::add_ext_type_handler(\"" << s->name << "\", get_type_handler_" << s->name << "());" << std::endl;
			out << "}" << std::endl;
			out << std::endl;
		}

		out << std::endl;
		out << "}" << std::endl;
		out << std::endl;

	}

	void write_bind_decl_dll(putki::parsed_file *file, std::ostream &out)
	{
		out << "namespace putki {" << std::endl;
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out << "void bind_type_" << s->name << "_dll();" << std::endl;
		}
		out << "}" << std::endl;
	}

	void write_bind_call_dll(putki::parsed_file *file, std::ostream &out)
	{
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out << "putki::bind_type_" << s->name << "_dll();" << std::endl;
		}
	}


}
