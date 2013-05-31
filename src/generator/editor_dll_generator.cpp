#include "generator.h"

#include <putki/domains.h>
#include <putki/types.h>
#include <generator/indentedwriter.h>

#include <iostream>

namespace putki
{

	void write_plain_set(putki::indentedwriter &out, putki::parsed_struct *s, size_t j)
	{
		out.line() << "((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << s->fields[j].name << " = value;";
	}

	void write_field_handlers(putki::indentedwriter &out, putki::parsed_struct *s)
	{
		for (size_t j=0;j!=s->fields.size();j++)
		{
			out.line() << "//////////////////////////////////////////////////////////////";
			out.line() << "// Field handler for " << s->name << "." << s->fields[j].name;
			out.line() << "struct ed_field_handler_" << s->name << "_" << s->fields[j].name << " : public putki::ext_field_handler_i";
			out.line() << "{";
			out.indent(1);
			out.line() << "// get info";
			out.line() << "const char *name() { return \"" << s->fields[j].name << "\"; }";
			out.line() << "putki::ext_field_type type() { return ";

			switch (s->fields[j].type)
			{
				case FIELDTYPE_STRING: out.cont() << " putki::EXT_FIELDTYPE_STRING; "; break;
				case FIELDTYPE_INT32: out.cont() << " putki::EXT_FIELDTYPE_INT32; "; break;
				default: out.cont() << " putki::EXT_FIELDTYPE_INVALID; "; break;
			}

			out.cont() << "}";
						
			// STRING SET
			out.line() << "// String type handlers";
			out.line() << "void set_string(putki::mem_instance *obj, const char *value) {";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_STRING)
				if (!s->fields[j].is_array)
					write_plain_set(out, s, j);
			out.indent(-1);
			out.cont() << "	}";

			// STRING GET
			out.line();
			out.line() << "const char* get_string(putki::mem_instance *obj) { ";
			out.indent(1);
			if (!s->fields[j].is_array && s->fields[j].type == FIELDTYPE_STRING)
				out.line() << "return ((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << s->fields[j].name << ".c_str();";
			else
				out.line() << "return \"####NOT-STRING[" << s->name << "]#####\";";
			out.indent(-1);
			out.line() << "}";

		
			out.line();
			out.indent(-1);
			out.line() << "};";
			out.line();
		}
	}

	void write_dll_impl(putki::parsed_file *file, putki::indentedwriter &out)
	{
		out.line();
		out.line() << "#include <putki/data-dll/dllinterface.h>";
		out.line() << "#include <putki/data-dll/dllinternal.h>";
		out.line();
		out.line() << "namespace inki {";
		out.line();
		out.indent(1);

		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			write_field_handlers(out, s);

			out.line();
			out.line() << "//////////////////////////////////////////////////////////////";
			out.line() << "// Struct handler for " << s->name;
			out.line() << "//////////////////////////////////////////////////////////////";
			out.line();

			out.line() << "struct ed_type_handler_" << s->name << " : public putki::ext_type_handler_i {";
			out.indent(1);
			out.line();
			out.line() << "// basic info";
			out.line() << "const char *name() { return \"" << s->name << "\"; }";
			out.line() << "putki::ext_field_handler_i *field(unsigned int idx)";			
			out.line() << "{";
			out.indent(1);
			out.line() << "switch (idx) {";
			out.indent(1);
			for (size_t j=0;j!=s->fields.size();j++)
			{
				out.line();
				out.line() << "case " << j << ": ";;
				out.line() << "{";
				out.indent(1);
				out.line() << "static ed_field_handler_" << s->name << "_" << s->fields[j].name << " efh;";
				out.line() << "return &efh;";
				out.indent(-1);
				out.line() << "}";
			}

			out.line();
			out.line() << "default: return 0;";
			out.indent(-1);
			out.line() << "} // switch";
			out.indent(-1);
			out.line() << "}";
			out.indent(-1);
			out.line() << "};";
			out.line();

			out.line();
			out.line() << "putki::ext_type_handler_i * get_type_handler_" << s->name << "()";
			out.line() << "{";
			out.indent(1);
			out.line() << "static ed_type_handler_" << s->name << " impl;";
			out.line() << "return &impl;";
			out.indent(-1);
			out.line() << "}";
			out.line();
			out.line();
			out.line() << "void bind_type_" << s->name << "_dll()";
			out.line() << "{";
			out.indent(1);
			out.line() << "	putki::add_ext_type_handler(\"" << s->name << "\", get_type_handler_" << s->name << "());";
			out.indent(-1);
			out.line() << "}";
			out.line();
		}

		out.line();
		out.indent(-1);
		out.line() << "}";
		out.line();

	}

	void write_bind_decl_dll(putki::parsed_file *file, putki::indentedwriter &out)
	{
		out.line();
		out.line() << "namespace inki {";
		out.indent(1);
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "void bind_type_" << s->name << "_dll();";
		}
		out.indent(-1);
		out.line() << "}";
	}

	void write_bind_call_dll(putki::parsed_file *file, putki::indentedwriter &out)
	{
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "inki::bind_type_" << s->name << "_dll();";
		}
	}


}
