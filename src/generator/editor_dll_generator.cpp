#include "generator.h"

#include <putki/domains.h>
#include <putki/types.h>
#include <generator/indentedwriter.h>

#include <iostream>

namespace putki
{

	void write_plain_set(putki::indentedwriter out, putki::parsed_struct *s, size_t j)
	{
		out.line() << "((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << s->fields[j].name << " = value;";
	}

	void write_pointer_set(putki::indentedwriter out, putki::parsed_struct *s, size_t j)
	{
		out.line() << "putki::mem_instance_real *mir = (putki::mem_instance_real *) obj;";
		out.line() << "((inki::" << s->name << " *)(mir->inst))->" << s->fields[j].name << " = (inki::" << s->fields[j].ref_type << " *) putki::db::ptr_to_allow_unresolved(mir->refs_db, value);";
	}


	void write_field_handlers(putki::indentedwriter out, putki::parsed_struct *s)
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
			out.line() << "const char* ref_type_name() { ";			
			if (!s->fields[j].ref_type.empty())
				out.cont() << "return \"" << s->fields[j].ref_type << "\";";
			else
				out.cont() << "return 0;";
			out.cont() << " }";

			out.line() << "putki::ext_field_type type() { return ";

			switch (s->fields[j].type)
			{
				case FIELDTYPE_STRING: out.cont() << " putki::EXT_FIELDTYPE_STRING; "; break;
				case FIELDTYPE_INT32: out.cont() << " putki::EXT_FIELDTYPE_INT32; "; break;
				case FIELDTYPE_POINTER: out.cont() << " putki::EXT_FIELDTYPE_POINTER; "; break;
				case FIELDTYPE_STRUCT_INSTANCE: out.cont() << " putki::EXT_FIELDTYPE_STRUCT_INSTANCE; "; break;
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
			out.line() << "}";

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


			// POINTER SET
			out.line() << "// Pointer type handlers";
			out.line() << "void set_pointer(putki::mem_instance *obj, const char *value) {";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_POINTER)
				if (!s->fields[j].is_array)
					write_pointer_set(out, s, j);
			out.indent(-1);
			out.cont() << "	}";

			// POINTER GET
			out.line();
			out.line() << "const char* get_pointer(putki::mem_instance *obj) { ";
			out.indent(1);
			if (!s->fields[j].is_array && s->fields[j].type == FIELDTYPE_POINTER)
				out.line() << "return putki::db::pathof_including_unresolved(((putki::mem_instance_real*)obj)->refs_db, ((inki::" << s->name << "*)((putki::mem_instance_real*)obj)->inst)->" << s->fields[j].name << ");";
			else
				out.line() << "return \"NOT A POINTER\";";
			out.indent(-1);
			out.line() << "}";

			//
			out.line();
			out.line() << "putki::mem_instance* make_struct_instance(putki::mem_instance *obj) {";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
			{
				out.line() << "putki::mem_instance_real *omr = (putki::mem_instance_real *)obj;";
				out.line() << "putki::mem_instance_real *mr = new putki::mem_instance_real();";
				out.line() << "static inki::ed_type_handler_" << s->fields[j].ref_type << " eth;";
				out.line() << "mr->is_struct_instance = true;";
				out.line() << "mr->th = putki::typereg_get_handler(\"" << s->fields[j].ref_type << "\");";
				out.line() << "mr->eth = &eth;";
				out.line() << "mr->inst = &((inki::" << s->name << " *)omr->inst)->" << s->fields[j].name << ";";
				out.line() << "mr->refs_db = mr->refs_db;";
				out.line() << "mr->path = \"<struct-instance>\";";
				out.line() << "return mr;";
			}
			else
			{
				out.line() << "return 0;";
			}
				
			out.indent(-1);
			out.line() << "}";

		
			out.line();
			out.indent(-1);
			out.line() << "};";
			out.line();
		}
	}

	void write_dll_impl(putki::parsed_file *file, putki::indentedwriter out)
	{
		out.line();
		out.line() << "#include <putki/data-dll/dllinterface.h>";
		out.line() << "#include <putki/data-dll/dllinternal.h>";
		out.line() << "#include <putki/builder/db.h>";
		out.line() << "#include <putki/builder/typereg.h>";
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

	void write_bind_decl_dll(putki::parsed_file *file, putki::indentedwriter out)
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

	void write_bind_call_dll(putki::parsed_file *file, putki::indentedwriter out)
	{
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "inki::bind_type_" << s->name << "_dll();";
		}
	}


}
