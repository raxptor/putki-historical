#include "generator.h"

#include <putki/domains.h>
#include <putki/types.h>

#include <iostream>

namespace putki
{

	void write_plain_set(std::ostream &out, putki::parsed_struct *s, size_t j)
	{
		out << "	((inki::" << s->name << " *)obj->inst)->" << s->fields[j].name << " = value;" << std::endl;
	}

	void write_field_handlers(std::ostream &out, putki::parsed_struct *s)
	{
		for (size_t j=0;j!=s->fields.size();j++)
		{
			out << "//////////////////////////////////////////////////////////////" << std::endl;
			out << "// Field handler for " << s->name << "." << s->fields[j].name << std::endl;
			out << "struct ed_field_handler_" << s->name << "_" << s->fields[j].name << " : public putki::ext_field_handler_i {" << std::endl;
			out << "	const char *name() { return \"" << s->fields[j].name << "\"; }" << std::endl;
			out << "    putki::ext_field_type type() { return ";

			switch (s->fields[j].type)
			{
				case FIELDTYPE_STRING: out << " putki::EXT_FIELDTYPE_STRING; "; break;
				case FIELDTYPE_INT32: out << " putki::EXT_FIELDTYPE_INT32; "; break;
				default: out << " putki::EXT_FIELDTYPE_INVALID; "; break;
			}

			out << "}" << std::endl;
						
			// STRING SET
			out << "	void set_string(putki::mem_instance *obj, const char *value) { " << std::endl;
			if (s->fields[j].type == FIELDTYPE_STRING)
				if (!s->fields[j].is_array)
					write_plain_set(out, s, j);
			out << "	}" << std::endl;

			// STRING GET
			out << "	const char* get_string(putki::mem_instance *obj) { " << std::endl;
			if (!s->fields[j].is_array && s->fields[j].type == FIELDTYPE_STRING)
				out << "	return ((inki::" << s->name << " *)obj->inst)->" << s->fields[j].name << ".c_str();" << std::endl;
			else
				out << "	return \"####NOT-STRING[" << s->name << "]#####\";" << std::endl;
			out << "	}" << std::endl;

		
			out << "};" << std::endl;
		}
	}

	void write_dll_impl(putki::parsed_file *file, std::ostream &out)
	{
		out << std::endl;
		out << "#include <putki/data-dll/dllinterface.h>" << std::endl;
		out << "#include <putki/data-dll/dllinternal.h>" << std::endl;
		out << std::endl;
		out << "namespace inki {" << std::endl;
		out << std::endl;

		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			write_field_handlers(out, s);

			out << std::endl;
			out << "//////////////////////////////////////////////////////////////" << std::endl;
			out << "// Struct handler for " << s->name << std::endl;
			out << "//////////////////////////////////////////////////////////////" << std::endl;
			out << std::endl;

			out << "struct ed_type_handler_" << s->name << " : public putki::ext_type_handler_i {" << std::endl;
			out << "	const char *name() { return \"" << s->name << "\"; }" << std::endl;
			out << "	putki::ext_field_handler_i *field(unsigned int idx) {" << std::endl;
			out << "		switch (idx) {" << std::endl;
			for (size_t j=0;j!=s->fields.size();j++)
			{
				out << "		case " << j << ": " << std::endl;;
				out << "		{" << std::endl;
				out << "			 static ed_field_handler_" << s->name << "_" << s->fields[j].name << " efh;" << std::endl;
				out << "			 return &efh;" << std::endl;
				out << "		}" << std::endl;
			}


			out << "			default: return 0;" << std::endl;
			out << "		}" << std::endl;
			out << "	}" << std::endl;
			out << "};" << std::endl;
			out << std::endl;

			out << "putki::ext_type_handler_i * get_type_handler_" << s->name << "()" << std::endl;
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
		out << "namespace inki {" << std::endl;
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
			out << "inki::bind_type_" << s->name << "_dll();" << std::endl;
		}
	}


}
