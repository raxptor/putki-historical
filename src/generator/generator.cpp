#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{   
	void write_ptr_walker(putki::parsed_file *file, putki::indentedwriter &out, bool runtime);

	void write_comment_block(const char *name, putki::indentedwriter &out)
	{
		out.line();
		out.line() << "/////////////////////////////////////////////////////////////////";
		out.line() << "// " << name;
		out.line() << "/////////////////////////////////////////////////////////////////";
		out.line();
	}

	const char *ptr_sub(putki::runtime rt)
	{
		if (rt == putki::RUNTIME_CPP_WIN32)
			return "int ";
		else
			return "long long ";
	}

	void write_runtime_header(putki::parsed_file *file, putki::runtime rt, putki::indentedwriter &out, bool for_putki)
	{
		std::string deftok("__outki_header" + file->filename + "__h__");

		if (!for_putki)
		{
			out.line() << "#ifndef " << deftok;
			out.line() << "#define " << deftok;
		}

		out.line() << "#pragma pack(push, 1)";

		if (!for_putki)
			out.line() << "#include <putki/types.h>";

		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			if (rt == putki::RUNTIME_CPP_WIN32 || rt == putki::RUNTIME_CPP_WIN64)
			{
				out.line();
				out.line() << "namespace outki {";
				out.indent(1);
				out.line() << "struct " << s->name << " {";
				out.indent(1);

				for (size_t i=0;i<s->fields.size();i++)
				{
					putki::parsed_field *f = &s->fields[i];
					if (!(f->domains & putki::DOMAIN_RUNTIME))
						continue;

					out.line();

					if (for_putki &&f->is_array)
					{
						out.cont() << ptr_sub(rt) << " ";
					}
					else
					{
						switch (s->fields[i].type)
						{
						case FIELDTYPE_INT32:
							out.cont() << "int ";
							break;
						case FIELDTYPE_BYTE:
							out.cont() << "char ";
							break;

						case FIELDTYPE_POINTER:
							{
								if (for_putki)
									out.cont() << ptr_sub(rt) << " ";
								else
									out.cont() << f->ref_type << " *";
							}
							break;
						case FIELDTYPE_STRUCT_INSTANCE:
							out.cont() << f->ref_type << " ";
							break;
						case FIELDTYPE_FILE:
						case FIELDTYPE_STRING:
							{
								if (for_putki)
									out.cont() << ptr_sub(rt) << " ";
								else
									out.cont() << "const char *";      
							}
							break;
						}
					}

					if (!for_putki &&f->is_array)
						out.cont() << " *";

					out.cont() << f->name << ";";

					if (s->fields[i].is_array)
						out.line() << "unsigned int " <<f->name << "_size;";
				}

				out.indent(-1);
				out.line() << "};";
				out.line();
				

				out.line() << "// loading processing for " << s->name;	
				out.line() << "char* post_blob_load_" << s->name << "(" << s->name << " *d, char *beg, char *end);";
				
				if (!for_putki)
					out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker);";

				out.indent(-1);
				out.line() << "}";
			}
		}
		out.line() << "#pragma pack(pop)";

		if (!for_putki)
			out.line() << "#endif";
	}

	void write_runtime_header(putki::parsed_file *file, putki::runtime rt, putki::indentedwriter &out)
	{
		write_runtime_header(file, rt, out, false);
	}

	const char *win32_field_type(putki::field_type f)
	{
		switch (f)
		{
		case FIELDTYPE_STRING:
			return "const char*";
		case FIELDTYPE_INT32:
			return "int";
		case FIELDTYPE_BYTE:
			return "char";
		default:
			return "???";
		}
	}

	const char *putki_field_type_pod(putki::field_type f)
	{
		switch (f)
		{
		case FIELDTYPE_STRING:
		case FIELDTYPE_FILE:
			return "std::string";
		case FIELDTYPE_INT32:
			return "int";
		case FIELDTYPE_BYTE:
			return "char";
		default:
			return 0;
		}
	}

	std::string putki_field_type(putki::parsed_field *pf)
	{
		if (pf->type == FIELDTYPE_STRUCT_INSTANCE)
			return pf->ref_type.c_str();
		else if (pf->type == FIELDTYPE_POINTER)
			return pf->ref_type + "*";

		return putki_field_type_pod(pf->type);
	}

	// cross-platform definitions namespace encapsulation
	//  
	const char *runtime_out_ns(putki::runtime rt)
	{
		if (rt == putki::RUNTIME_CPP_WIN64)
			return "out_ns_win64compat";
		else
			return "out_ns_win32compat";
	}

	void write_runtime_impl(putki::parsed_file *file, putki::runtime rt, putki::indentedwriter &out)
	{
		out.line() << "// Generated code!";
		out.line();
		out.line() << "#include \"" << file->filename << ".h\"";
		out.line();
		out.line() << "// These includes go to the runtime headers";
		out.line() << "#include <putki/blob.h>";
		out.line() << "#include <putki/types.h>";
		out.line();
		out.line() << "namespace outki {";
		out.indent(1);

		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			if (rt == putki::RUNTIME_CPP_WIN32 || rt == putki::RUNTIME_CPP_WIN64)
			{
				out.line() << "char* post_blob_load_" << s->name << "(" << s->name << " *d, char *aux_cur, char *aux_end)";
				out.line() << "{";
				out.indent(1);

				for (size_t j=0;j<s->fields.size();j++)
				{
					putki::parsed_field *f = &s->fields[j];
					if (!(f->domains & putki::DOMAIN_RUNTIME))
						continue;

					out.line() << "// field " << s->fields[j].name;

					std::string fref = std::string("d->") + s->fields[j].name;

					if (s->fields[j].is_array)
					{
						if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
						{
							out.line() << "d->" << s->fields[j].name << " = reinterpret_cast<outki::" << s->fields[j].ref_type << "*>(aux_cur);";
							out.line() << "aux_cur += sizeof(" << s->fields[j].ref_type << ") * d->" << s->fields[j].name << "_size;";
						}
						else if (s->fields[j].type == FIELDTYPE_POINTER)
						{
							out.line() << "d->" << s->fields[j].name << " = reinterpret_cast<" << s->fields[j].ref_type << "**>(aux_cur);";
						}
						else
						{
							out.line() << "d->" << s->fields[j].name << " = reinterpret_cast<" << win32_field_type(s->fields[j].type) << "*>(aux_cur);";
							out.line() << "aux_cur += sizeof(" << win32_field_type( s->fields[j].type ) << ") * d->" << s->fields[j].name << "_size;";
						}

						out.line() << "if (aux_cur > aux_end) return 0; ";
						out.line() << "for (unsigned int i=0;i<d->" << s->fields[j].name + "_size;i++)";
						out.line() << "{";
						out.indent(1);
						fref = fref + "[i]";
					}

					switch (s->fields[j].type)
					{
						/*
						case FIELDTYPE_POINTER:
						if (s->fields[j].is_array)
						out.line() << "			resolve_" << s->fields[j].ref_type << "_ptr(&d->" << s->name << "[i]);";
						else
						out.line() << "		resolve_" << s->fields[j].ref_type << "_ptr(d->" << s->name << ");";
						break;
						*/
					case FIELDTYPE_STRUCT_INSTANCE:
						out.line() << "aux_cur = outki::post_blob_load_" << s->fields[j].ref_type << "(&" << fref << ", aux_cur, aux_end);";
						break;
					case FIELDTYPE_INT32:
						out.line() << "putki::prep_int32_field((char*)&" << fref <<  ");";
						break;
					case FIELDTYPE_STRING:
						out.line() << "aux_cur = putki::post_blob_load_string(&" << fref << ", aux_cur, aux_end);";
						break;
					default:
						break;
					}

					if (s->fields[j].is_array)
					{
						out.indent(-1);
						out.line() << "}";
					}
				}

				out.line();
				out.line() << "return aux_cur;";
				out.indent(-1);
				out.line() << "}";
			}
		}

		out.indent(-1);
		out.line() << "}";

		write_ptr_walker(file, out, true);
	}

	void write_putki_header(putki::parsed_file *file, putki::indentedwriter &out)
	{
		std::string deftok = std::string("__OUTKI_") + file->filename + "_H__";
		out.line() << "#ifndef " << deftok;
		out.line() << "#define " << deftok;

		out.line() << "#include <string>";
		out.line() << "#include <vector>";
		out.line() << "#include <putki/builder/write.h>";
		out.line();
		out.line() << "namespace putki { struct depwalker_i; }";
		out.line();
		out.line() << "namespace inki {";
		out.indent(1);

		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			out.line() << "struct " << s->name << " {";
			out.indent(1);

			for (size_t j=0;j<s->fields.size();j++)
			{
				if (j > 0)
					out.line();

				out.line() << "// field " << s->fields[j].name;

				if (s->fields[j].is_array)
				{
					std::string r_type_name = putki_field_type(&s->fields[j]);
					out.line() << "std::vector<" << r_type_name << "> " << s->fields[j].name << ";";
				}
				else
				{
					out.line() << putki_field_type(&s->fields[j]) << " " << s->fields[j].name << ";";
				}
			}
			
			out.indent(-1);
			out.line() << "};";
			 
			if (s->domains & putki::DOMAIN_RUNTIME)
			{
				out.line() << "// writing processing for " << s->name;
				out.line() << "char *write_" << s->name << "_into_blob(inki::" << s->name << " *in, char *out_beg, char *out_end);";
			}
			
			out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker, bool traverseChildren = true);";
		
		}
		
		out.line();
		out.indent(-1);
		out.line() << "} // namespace inki";
		out.line() << "#endif";
	}

	void write_putki_field_parse(putki::parsed_field *f, putki::indentedwriter &out)
	{		
		out.line() << "// parse field " << f->name;

		std::string ref = "target->" + f->name;
		std::string node = "putki::parse::get_object_item(pn, \"" + f->name + "\")";

		if (f->is_array)
		{
			out.line();
			out.line() << "{";
			out.indent(1);
			out.line() << "// array parse block";

			std::string type = putki_field_type(f);
			out.line() << ref << ".clear();";
			out.line() << "unsigned int i = 0;";
			out.line() << "putki::parse::node *arr = putki::parse::get_object_item(pn, \"" << f->name << "\");";
			out.line() << "while (putki::parse::node * n = putki::parse::get_array_item(arr, i)) {";
			out.indent(1);
			out.line() << "  " << type << " tmp;";
			out.line() << "  (void)n;";
			out.line() << "  target->" << f->name << ".push_back(tmp); i++;";
			out.indent(-1);
			out.line() << "}";
			out.line() << "i = 0;";
			out.line() << "while (putki::parse::node * n = putki::parse::get_array_item(arr, i)) {";
			out.indent(1);
			out.line() << type << " & fixed_mem_obj = target->" << f->name << "[i];";

			ref = "fixed_mem_obj";
			node = "n";
		}

		if (f->type == FIELDTYPE_STRING)
		{
			out.line() << ref << " = " << " putki::parse::get_value_string(" << node << "); ";
		}
		else if (f->type == FIELDTYPE_INT32 || f->type == FIELDTYPE_BYTE)
		{
			out.line() << ref << " = " << "(" << putki_field_type(f) << ") putki::parse::get_value_int(" << node << "); ";
		}
		else if (f->type == FIELDTYPE_STRUCT_INSTANCE)
		{
			out.line() << "fill_" << f->ref_type << "_from_parsed(" << node << ", &" << ref << ", resolver);";
		}
		else if (f->type == FIELDTYPE_POINTER) 
		{
			out.line() << "resolver->resolve_pointer((putki::instance_t *)&" << ref << ", putki::parse::get_value_string(" << node << "));";
		}

		if (f->is_array)
		{
			out.line() << "i++;";
			out.indent(-1);
			out.line() << "}";
			out.indent(-1);
			out.line() << "} // end array parse block";
		}
	}

	void write_putki_parse(putki::parsed_file *file, putki::indentedwriter &out)
	{
		out.line() << "#include <putki/builder/parse.h>";
		out.line() << "namespace inki {";
		out.line();
		out.indent(1);
		for (int i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			
			out.line() << "void fill_" << s->name << "_from_parsed(putki::parse::node *pn, void *target_, putki::load_resolver_i *resolver)";
			out.line() << "{";
			out.indent(1);
			out.line() << s->name << " *target = (" << s->name << " *) target_;";

			// field parsing.
			for (unsigned int j=0;j<s->fields.size();j++)
			{
				out.line();
				write_putki_field_parse(&s->fields[j], out);
			}

			out.indent(-1);
			out.line() << "}";
			out.line();
		}

		out.indent(-1);
		out.line() << "} // namespace inki";
		out.line();
	}

	void write_blob_writer_call(putki::runtime rt, const char *name, putki::indentedwriter &out)
	{
		if (rt == putki::RUNTIME_CPP_WIN32)
			out.line() << "if (rt == putki::RUNTIME_CPP_WIN32)";
		else if (rt == putki::RUNTIME_CPP_WIN64)
			out.line() << "if (rt == putki::RUNTIME_CPP_WIN64)";
		else
			return;

		out.line(1) << "return " << runtime_out_ns(rt) << "::write_" << name << "_into_blob((" << name << "*) source, beg, end);";
	}

	void write_ptr_walker(putki::parsed_file *file, putki::indentedwriter &out, bool runtime)
	{
		write_comment_block("Dependency walking", out);

		const char *ns = runtime ? "outki" : "inki";
		
		out.line();
		out.line() << "namespace " << ns << " {";
		out.line();
		out.indent(1);

		for (int i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			
			if (runtime)
				out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker)";
			else
				out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker, bool traverseChildren)";

			out.line() << "{";
			out.indent(1);

			const char *levelCheck = runtime ? "" : "if (traverseChildren) ";

			for (size_t j=0;j<s->fields.size();j++)
			{
				putki::parsed_field & fd = s->fields[j];

				std::string ref = "input->" + fd.name;

				if (fd.is_array)
				{
					if (!runtime)
					{
						out.line() << "for (unsigned int i=0;i<input->" << fd.name << ".size();i++) {";
						ref = "input->" + fd.name + "[i]";
					}
					else
					{
						out.line() << "for (unsigned int i=0;i<input->" << fd.name << "_size;i++) {";
						ref = "input->" + fd.name + "[i]";
					}
					out.indent(1);
				}

				if (fd.type == putki::FIELDTYPE_STRUCT_INSTANCE)
				{
				    out.line() << levelCheck << "walk_dependencies_" << fd.ref_type << "(&" << ref << ", walker);";
				}
				else if (fd.type == putki::FIELDTYPE_POINTER)
				{
					out.line() << "walker->pointer((putki::instance_t *)&" << ref << ");";
					out.line() << "if (" << ref << ") { " << levelCheck << " walk_dependencies_" << fd.ref_type << "(" << ref << ", walker); }";
				}
				else
				{
					if (fd.is_array)
						out.line() << "{ } // do nothing loop implementation";

				}

				if (fd.is_array)
				{
					out.indent(-1);
					out.line() << "} // end array loop";
				}

			}

			out.indent(-1);
			out.line() << "}";
			out.line();
		}

		out.indent(-1);
		out.line() << "} // dependency walker section";
	}

	void write_json_writer(putki::parsed_struct *s, putki::indentedwriter &out)
	{
		out.line() << s->name << " * input = (" << s->name << " *)source;";

		for (size_t j=0;j<s->fields.size();j++)
		{
			putki::parsed_field & fd = s->fields[j];

			out.line();
			out.line() << "// field " << fd.name;

			std::string ref = "input->" + fd.name;
			std::string delim = "";

			out.line() << "out << putki::write::json_indent(indent+1) << \"\\\"" << fd.name << "\\\": \";";
			
			if (fd.is_array)
			{
				out.line() << "{";
				out.indent(1);
				out.line() << "out << \"[\";";
				out.line() << "const char *delim = \"\";";
				out.line() << "for (unsigned int i=0;i<input->" << fd.name << ".size();i++)";
				out.line() << "{";
				out.indent(1);
				delim = "delim << ";
				ref.append("[i]");
			}

			if (fd.type == FIELDTYPE_STRING)
			{
				out.line() << "out << " << delim << "putki::write::json_str(" << ref << ".c_str());";
			}
			else if (fd.type == FIELDTYPE_INT32 || fd.type == FIELDTYPE_BYTE)
			{
				out.line() << "out << " << delim << "(int)" << ref << ";";
			}
			else if (fd.type == FIELDTYPE_POINTER)
			{
				out.line() << "out << " << delim <<  "\"POINTERREF\";";
			}
			else if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
			{
				out.line() << "out << " << delim << "\"\\n\";";
				out.line() << "// write struct contents[" << fd.ref_type << "]";
				out.line() << fd.ref_type << "_handler thandler;";
				out.line() << "thandler.write_json(ref_source, &" << ref << ", out, indent + 1);";
			}
			else
			{
				out.line() << "out << " << delim << "\"UNSUPPORTED\";";
			}

			if (fd.is_array)
			{
				out.line() << "delim = \", \";";
				out.indent(-1);
				out.line() << "} // end for";
				out.indent(-1);
				out.line() << "out << \"]\";";
				out.line() << "} // end array";
			}

			if (j < s->fields.size()-1)
				out.line() << "out << \",\";";

			out.line() << "out << std::endl;";
		}
	}

	void write_putki_type_reg(putki::parsed_file *file, putki::indentedwriter &out)
	{
		write_comment_block("Type registration", out);

		out.line() << "#include <putki/builder/typereg.h>";
		out.line() << "#include <putki/builder/write.h>";
		out.line() << "#include <putki/builder/db.h>";
		out.line() << "#include <putki/runtime.h>";
		out.line();
		out.line() << "namespace inki {";
		out.indent(1);
		out.line();
		for (int i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line();
			out.line() << "// type handler for " << s->name;
			out.line() << "struct " << s->name << "_handler : public putki::type_handler_i {";
			out.indent(1);
			out.line();
			out.line() << "// alloc/free";
			out.line() << "putki::instance_t alloc() { return new " << s->name << "; }";
			out.line() << "void free(putki::instance_t p) { delete (" << s->name << "*) p; }";
			out.line();
			out.line() << "// info";
			out.line() << "const char *name() { return \"" << s->name << "\"; }";
			out.line() << "int id() { return " << s->unique_id << "; }";
			out.line();
			out.line() << "// deps";
			out.line() << "void walk_dependencies(putki::instance_t source, putki::depwalker_i *walker, bool traverseChildren) {";
			out.line(1) << "walk_dependencies_" << s->name << "( (" << s->name << " *) source, walker, traverseChildren);";
			out.line() << "}";
			out.line();
			out.line() << "// json writer";
			out.line() << "void write_json(putki::db::data *ref_source, putki::instance_t source, std::ostream & out, int indent)";
			out.line() << "{";
			out.indent(1);
			write_json_writer(s, out);
			out.indent(-1);
			out.line() << "}";
			out.line();
			out.line() << "// writer";
			out.line() << "char* write_into_buffer(putki::runtime rt, putki::instance_t source, char *beg, char *end)";
			out.line() << "{";

			out.indent(1);

			write_blob_writer_call(putki::RUNTIME_CPP_WIN32, s->name.c_str(), out);
			write_blob_writer_call(putki::RUNTIME_CPP_WIN64, s->name.c_str(), out);			

			out.line() << "return 0;";
			out.indent(-1);
			out.line() << "}";

			out.line();
			out.line();
			out.line()  << "void fill_from_parsed(putki::parse::node *pn, putki::instance_t target, putki::load_resolver_i *resolver) {";
			out.line(1) << "fill_" << s->name << "_from_parsed(pn, target, resolver);";
			out.line()  << "}";
			out.line();
			out.indent(-1);
			out.line() << "} s_" << s->name << "_handler;";
			out.line();
			out.line() << "void bind_type_" << s->name << "() { putki::typereg_register(\"" << s->name << "\", &s_" << file->structs[i].name << "_handler); } ";
		}

		out.indent(-1);
		out.line() << "}";
	}

	void write_blob_writers(putki::parsed_file *file, putki::indentedwriter &out, putki::runtime rt)
	{
		std::string ns = runtime_out_ns(rt);
		out.line() << "namespace " << ns << " {";
		out.indent(1);

		write_runtime_header(file, rt, out, true);

		std::string out_ns = ns + "::outki::";

		out.line();
		out.line() << "namespace { typedef unsigned short length_t; }";
		out.line();
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			out.line();
			out.line() << "char *write_" << s->name << "_aux(inki::" << s->name << " *in, " << out_ns << s->name << " *d, char *out_beg, char *out_end)";
			out.line() << "{";
			out.indent(1);

			for (size_t j=0;j<s->fields.size();j++)
			{
				putki::parsed_field & fd = s->fields[j];

				if (!(fd.domains & putki::DOMAIN_RUNTIME))
					continue;

				out.line() << "// field " << fd.name;

				std::string srcd = "in->" + fd.name;
				std::string outd = "d->" + fd.name;

				if (fd.is_array)
				{
					std::string ft = win32_field_type(fd.type);
					if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
						ft = out_ns + fd.ref_type;

					out.line() << outd << "_size = " << srcd << ".size();";
					out.line();
					out.line() << "{";
					out.indent(1);
					out.line() << "// array construct for field '" << fd.name << "'";
					out.line() << ft << " *inp = reinterpret_cast<" << ft << " *>(out_beg);";
					out.line() << "out_beg += sizeof(" << ft << ") * " << outd << "_size;";
					out.line() << "for (unsigned int i=0;i<" << outd << "_size;i++)";
					out.line() << "{";
					out.indent(1);
					srcd = srcd + "[i]";
					outd = "inp[i]";
				}

				if (fd.type == FIELDTYPE_STRING)
				{
					out.line() << "out_beg = putki::pack_string_field((char*) &" << outd << ", " << srcd << ".c_str(), out_beg, out_end);";
				}
				else if (fd.type == FIELDTYPE_INT32)
				{
					out.line() << "putki::pack_int32_field((char*)&" << outd << ", " << srcd << ");";
				}
				else if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
				{
					out.line() << "out_beg = write_" << fd.ref_type << "_aux(&" << srcd << ", &" << outd << ", out_beg, out_end);";
				}
				else if (fd.type == FIELDTYPE_POINTER || fd.type == FIELDTYPE_BYTE)
				{
					out.line() << outd << " = (" << ptr_sub(rt) << ")((char*)" << srcd << " - (char*)0);";
				}

				if (fd.is_array)
				{
					out.indent(-1);
					out.line() << "}";
					out.indent(-1);
					out.line() << "}";
				}
			}

			out.line();
			out.line() << "return out_beg;";
			out.indent(-1);
			out.line() << "}";

			std::string out_n(out_ns + s->name);

			out.line();
			out.line() << "char *write_" << s->name << "_into_blob(inki::" << s->name << " *in, char *out_beg, char *out_end)";
			out.line() << "{";
			out.indent(1);
			out.line() << "if (out_end - out_beg < sizeof(" << out_n << ")) return 0;";
			out.line() << out_n << " *d = (" << out_n << " *) out_beg;";
			out.line() << "return write_" << s->name << "_aux(in, d, out_beg + sizeof(" << out_n << "),  out_end);";
			out.indent(-1);
			out.line() << "}";

		}

		out.indent(-1);
		out.line() << "}";
	}

	void write_putki_impl(putki::parsed_file *file, putki::indentedwriter &out)
	{
		out.line() << "// Generated code!";
		out.line() << "#include \"" << file->filename << ".h\"";
		out.line() << "#include <putki/blob.h>";
		out.line() << "#include <putki/builder/typereg.h>";
		out.line() << "#include <iostream>";

		out.line();

		// for cross-writing
		write_blob_writers(file, out, putki::RUNTIME_CPP_WIN64);
		write_blob_writers(file, out, putki::RUNTIME_CPP_WIN32);

		write_putki_parse(file, out);

		write_ptr_walker(file, out, false);
		write_putki_type_reg(file, out);
	}

	void write_bind_decl(putki::parsed_file *file, putki::indentedwriter &out)
	{
		out.line() << "namespace inki {";
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "void bind_type_" << s->name << "();";
		}
		out.line() << "}";
	}

	void write_bind_calls(putki::parsed_file *file, putki::indentedwriter &out)
	{
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "inki::bind_type_" << s->name << "();";
		}
	}

	void write_runtime_blob_load_cases(putki::parsed_file *file, putki::indentedwriter &out)
	{
		for (unsigned int i=0;i<file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "case " << s->unique_id << ":";
			out.line() << "	{ char *out = post_blob_load_" << s->name << "((" << s->name << "*)begin, begin + sizeof(" << s->name << "), end);";
			out.line() << "	  if (out) walk_dependencies_" << s->name << "((" << s->name << "*)begin, ptr_reg);"<< std::endl;
			out.line() << "	  return out;";
			out.line() << "	}";
		}
	}

	void write_runtime_blob_load_decl(const char *hpath, putki::indentedwriter &out)
	{
		out.line() << "#include <" << hpath << ">";
	}	
}
