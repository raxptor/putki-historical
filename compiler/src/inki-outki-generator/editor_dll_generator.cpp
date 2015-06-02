#include "generator.h"

#include <putki/domains.h>
#include <writetools/indentedwriter.h>

#include <iostream>

namespace putki
{

	void write_plain_set(putki::indentedwriter out, putki::parsed_struct *s, size_t j, std::string const &field_ref)
	{
		out.line() << "((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << " = value;";
	}

	void write_plain_get(putki::indentedwriter out, putki::parsed_struct *s, size_t j, std::string const &field_ref)
	{
		out.line() << "return ((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << ";";
	}

	void write_pointer_set(putki::indentedwriter out, putki::parsed_struct *s, size_t j, std::string const &field_ref)
	{
		out.line() << "putki::mem_instance_real *mir = (putki::mem_instance_real *) obj;";
		out.line() << "if (!value || !value[0])";
		out.line() << "\t((inki::" << s->name << " *)(mir->inst))->" << field_ref << " = 0;";
		out.line() << "else";
		out.line() << "\t((inki::" << s->name << " *)(mir->inst))->" << field_ref << " = (inki::" << s->fields[j].ref_type << " *) putki::db::ptr_to_allow_unresolved(mir->refs_db, value);";
	}

	void write_integer_set_get(putki::indentedwriter out, int64_t min, int64_t max, const char *type_name, putki::parsed_struct *s, size_t j, std::string const &field_ref)
	{
		out.line() << "int set_integer(putki::mem_instance *obj, int64_t v) {";
		out.indent(1);
		out.line() << "if (v < " << min << "LL || v >= " << max << "LL)";
		out.line(1) << "return 0;";
		out.line() << type_name << " value = (" << type_name << ") v;";
		write_plain_set(out, s, j, field_ref);
		out.line() << "return 1;";
		out.indent(-1);
		out.line() << "}";

		// BYTE GET
		out.line() << "int64_t get_integer(putki::mem_instance *obj) {";
		out.line(1) << "return ((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << ";";
		out.line() << "}";
	}

	void write_field_handlers(putki::indentedwriter out, putki::parsed_struct *s)
	{
		for (size_t j=0; j!=s->fields.size(); j++)
		{
			if (s->fields[j].is_build_config)
				continue;
			
			out.line() << "//////////////////////////////////////////////////////////////";
			out.line() << "// Field handler for " << s->name << "." << s->fields[j].name;
			out.line() << "struct ed_field_handler_" << s->name << "_" << s->fields[j].name << " : public putki::ext_field_handler_i";
			out.line() << "{";
			out.indent(1);
			out.line() << "// get info";
			out.line() << "const char *name() { return \"" << s->fields[j].name << "\"; }";
			out.line() << "bool is_array() { return " << s->fields[j].is_array << "; }";

			if (s->fields[j].is_array)
			{
				std::string vec_ref = "((inki::" + s->name + " *)((putki::mem_instance_real*)obj)->inst)->" + s->fields[j].name + ".";
				out.line() << "int _idx;";
				out.line() << "void set_array_index(int i) { _idx = i; }";
				out.line() << "int get_array_size(putki::mem_instance *obj) { return " << vec_ref << "size(); }";

				std::string new_obj = "0";
				if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
					new_obj = s->fields[j].ref_type + "()";
				else if (s->fields[j].type == FIELDTYPE_STRING || s->fields[j].type == FIELDTYPE_FILE || s->fields[j].type == FIELDTYPE_PATH)
					new_obj = "\"\"";

				out.line() << "void array_insert(putki::mem_instance *obj) { " << vec_ref << "push_back(" + new_obj + "); }";
				out.line() << "void array_erase(putki::mem_instance *obj) { " << vec_ref <<"erase(" << vec_ref <<"begin() + _idx); }";
			}
			else
			{
				out.line() << "void set_array_index(int i) { }";
				out.line() << "int get_array_size(putki::mem_instance *obj) { return -1; }";
				out.line() << "void array_insert(putki::mem_instance *obj) { }";
				out.line() << "void array_erase(putki::mem_instance *obj) { }";
			}

			out.line() << "bool is_aux_ptr() { return " << (s->fields[j].is_aux_ptr ? "true" : "false") << "; }";

			bool showineditor = true;
			if (!(s->fields[j].domains & putki::DOMAIN_INPUT))
				showineditor = false;
			if (!s->fields[j].show_in_editor)
				showineditor = false;

			out.line() << "bool show_in_editor() { return " << (showineditor ? "true" : "false") << "; }";

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
				case FIELDTYPE_UINT32: out.cont() << " putki::EXT_FIELDTYPE_UINT32; "; break;
				case FIELDTYPE_POINTER: out.cont() << " putki::EXT_FIELDTYPE_POINTER; "; break;
				case FIELDTYPE_FILE: out.cont() << " putki::EXT_FIELDTYPE_FILE; "; break;
				case FIELDTYPE_PATH: out.cont() << " putki::EXT_FIELDTYPE_PATH; "; break;
				case FIELDTYPE_BYTE: out.cont() << " putki::EXT_FIELDTYPE_BYTE; "; break;
				case FIELDTYPE_FLOAT: out.cont() << " putki::EXT_FIELDTYPE_FLOAT; "; break;
				case FIELDTYPE_BOOL: out.cont() << " putki::EXT_FIELDTYPE_BOOL; "; break;
				case FIELDTYPE_STRUCT_INSTANCE: out.cont() << " putki::EXT_FIELDTYPE_STRUCT_INSTANCE; "; break;
				case FIELDTYPE_ENUM: out.cont() << " putki::EXT_FIELDTYPE_ENUM; "; break;
				default: out.cont() << " putki::EXT_FIELDTYPE_INVALID; "; break;
			}

			out.cont() << "}";

			std::string field_ref = s->fields[j].name;
			if (s->fields[j].is_array)
				field_ref += "[_idx]";

			// STRING SET
			out.line() << "void set_string(putki::mem_instance *obj, const char *value) {";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_STRING || s->fields[j].type == FIELDTYPE_FILE || s->fields[j].type == FIELDTYPE_PATH)
				write_plain_set(out, s, j, field_ref);
			out.indent(-1);
			out.line() << "}";

			// STRING GET
			out.line() << "const char* get_string(putki::mem_instance *obj) { ";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_STRING || s->fields[j].type == FIELDTYPE_FILE || s->fields[j].type == FIELDTYPE_PATH)
				out.line() << "return ((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << ".c_str();";
			else
				out.line() << "return \"!! field is not string!!\";";
			out.indent(-1);
			out.line() << "}";

			// ENUM SET
			out.line() << "void set_enum(putki::mem_instance *obj, const char *value) {";
			out.indent(1);

			if (s->fields[j].type == FIELDTYPE_ENUM)
				out.line() << "((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << " = " << s->fields[j].ref_type << "_from_string(value);";

			out.indent(-1);
			out.line() << "}";

			out.line() << "const char* get_enum(putki::mem_instance *obj) { ";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_ENUM)
				out.line() << "return " << s->fields[j].ref_type << "_to_string(((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << ");";
			else
				out.line() << "return 0;";
			out.indent(-1);
			out.line() << "}";

			out.line() << "const char* get_enum_possible_value(int i) { ";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_ENUM)
				out.line() << "return " << s->fields[j].ref_type << "_string_by_index(i);";
			else
				out.line() << "return 0;";
			out.indent(-1);
			out.line() << "}";

			// POINTER SET
			out.line() << "void set_pointer(putki::mem_instance *obj, const char *value) {";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_POINTER)
				write_pointer_set(out, s, j, field_ref);
			else if (s->fields[j].type == FIELDTYPE_PATH)
				write_plain_set(out, s, j, field_ref);

			out.indent(-1);
			out.line() << "}";

			// POINTER GET
			out.line() << "const char* get_pointer(putki::mem_instance *obj) { ";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_POINTER)
			{
				out.line() << "putki::instance_t ptr = ((inki::" << s->name << "*)((putki::mem_instance_real*)obj)->inst)->" << field_ref << ";";
				out.line() << "if (!ptr) return \"\";";
				out.line() << "return putki::db::pathof_including_unresolved(((putki::mem_instance_real*)obj)->refs_db, ptr);";
			}
			else if (s->fields[j].type == FIELDTYPE_PATH)
			{
				out.line() << "return ((inki::" << s->name << " *)((putki::mem_instance_real*)obj)->inst)->" << field_ref << ".c_str();";
			}
			else{
				out.line() << "return \"!! - not a pointer - !!\";";
			}
			out.indent(-1);
			out.line() << "}";

			switch (s->fields[j].type)
			{
				case FIELDTYPE_BYTE:
					write_integer_set_get(out, 0, 256, "unsigned char", s, j, field_ref);
					break;
				case FIELDTYPE_UINT32:
					write_integer_set_get(out, 0, 0x100000000LL, "unsigned int", s, j, field_ref);
					break;
				case FIELDTYPE_INT32:
					write_integer_set_get(out, -0x80000000LL, 0x80000000LL, "int", s, j, field_ref);
					break;
				case FIELDTYPE_BOOL:
					write_integer_set_get(out, 0, 2, "bool", s, j, field_ref);
					break;
				default:
					out.line() << "int set_integer(putki::mem_instance *obj, int64_t v) { return 0; }";
					out.line() << "int64_t get_integer(putki::mem_instance *obj) { return -1; }";
					break;
			}
			
			if (s->fields[j].type == FIELDTYPE_FLOAT)
			{
				out.line() << "void set_float(putki::mem_instance *obj, float value) {";
				write_plain_set(out, s, j, field_ref);
				out.line() << "}";
				out.line() << "float get_float(putki::mem_instance *obj) {";
				write_plain_get(out, s, j, field_ref);
				out.line() << "}";
			}
			else
			{
				out.line() << "void set_float(putki::mem_instance *obj, float v) { }";
				out.line() << "float get_float(putki::mem_instance *obj) { return 0; }";
			}

			//
			out.line() << "putki::mem_instance* make_struct_instance(putki::mem_instance *obj) {";
			out.indent(1);
			if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
			{
				out.line() << "putki::mem_instance_real *omr = (putki::mem_instance_real *)obj;";
				out.line() << "putki::mem_instance_real *mr = new putki::mem_instance_real();";
				out.line() << "mr->is_struct_instance = true;";
				out.line() << "mr->th = inki::get_" << s->fields[j].ref_type << "_type_handler();";
				out.line() << "mr->eth = inki::get_" << s->fields[j].ref_type << "_ext_type_handler();";
				out.line() << "mr->inst = &((inki::" << s->name << " *)omr->inst)->" << field_ref << ";";
				out.line() << "mr->refs_db = omr->refs_db;";
				out.line() << "mr->path = strdup(omr->path);";
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
		out.line() << "#include <putki/sys/compat.h>";
		out.line() << "#include <stdint.h>";
		out.line();
		write_includes(file, out, true);
		out.line();
		out.line() << "namespace inki {";
		out.line();
		out.indent(1);

		for (size_t i=0; i!=file->structs.size(); i++)
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
			out.line() << "const char *module_name() { return \"" << file->modulename << "\"; }";
			out.line() << "bool permit_as_asset() { return " << (s->permit_as_asset ? "true" : "false") << "; }";
			out.line() << "bool permit_as_aux_instance() { return " << (s->permit_as_auxptr ? "true" : "false") << "; }";
			out.line();

			if (s->parent.empty())
				out.line() << "const char *parent_name() { return 0; }";
			else
				out.line() << "const char *parent_name() { return \"" << s->parent << "\"; }";

			out.line() << "const char *inline_editor() { return \"" << s->inline_editor << "\"; }";
			
			out.line();
			out.line() << "bool write_json(char *buffer, unsigned int size)";
			out.line() << "{";
			out.line(1) << "strcpy(buffer, \"Gurka\");";
			out.line(1) << "return true;";
			out.line() << "}";

			out.line();
			out.line() << "void content_hash(char *buffer)";
			out.line() << "{";
			out.line(1) << "strcpy(buffer, \"BADF00D\");";
			out.line() << "}";

			out.line();
			out.line() << "putki::ext_field_handler_i *field(unsigned int idx)";
			out.line() << "{";

			out.indent(1);
			out.line() << "switch (idx) {";
			out.indent(1);



			int idx = 0;
			for (size_t j=0; j!=s->fields.size(); j++)
			{
				if (s->fields[j].is_build_config)
					continue;
				
				if (s->fields[j].name != "_rtti_type")
				{
					s->fields[j]._WROTE_DLL_FIELD_INDEX = idx;

					out.line();
					out.line() << "case " << idx++ << ": ";;
					out.line() << "{";
					out.indent(1);
					out.line() << "static ed_field_handler_" << s->name << "_" << s->fields[j].name << " efh;";
					out.line() << "return &efh;";
					out.indent(-1);
					out.line() << "}";
				}
			}

			out.line();
			out.line() << "case 999999: return 0;";
			out.line() << "default: return 0;";
			out.indent(-1);
			out.line() << "} // switch";
			out.indent(-1);
			out.line() << "}";
			out.indent(-1);
			out.line() << "};";
			out.line();

			out.line();
			out.line() << "putki::ext_type_handler_i * get_" << s->name << "_ext_type_handler()";
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
			out.line() << "	putki::add_ext_type_handler(\"" << s->name << "\", get_" << s->name << "_ext_type_handler());";
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
		for (unsigned int i=0; i<file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "void bind_type_" << s->name << "_dll();";
		}
		out.indent(-1);
		out.line() << "}";
	}

	void write_bind_call_dll(putki::parsed_file *file, putki::indentedwriter out)
	{
		for (unsigned int i=0; i<file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "inki::bind_type_" << s->name << "_dll();";
		}
	}


}
