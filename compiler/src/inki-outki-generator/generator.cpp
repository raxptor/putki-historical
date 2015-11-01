#include "generator.h"

#include <putki/domains.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace putki
{
	bool is_putki_struct(putki::parsed_struct *s)
	{
		for (int i=0;i!=s->targets.size();i++)
			if (!strcmp(s->targets[i].c_str(), "putki"))
				return true;
		return false;
	}

	void write_ptr_walker(putki::parsed_file *file, putki::indentedwriter out, bool runtime);

	void write_includes(putki::parsed_file *file, putki::indentedwriter out, bool inki)
	{
		for (unsigned int i=0; i<file->includes.size(); i++)
		{
			std::string &inc = file->includes[i];
			int np = inc.find_last_of('.');
			if (np != std::string::npos)
			{
				if (inki)
					out.line() << "#include <inki/" << inc.substr(0, np) << ".h>";
				else
					out.line() << "#include <outki/" << inc.substr(0, np) << ".h>";
			}
		}
	}

	void write_comment_block(const char *name, putki::indentedwriter out)
	{
		out.line();
		out.line() << "/////////////////////////////////////////////////////////////////";
		out.line() << "// " << name;
		out.line() << "/////////////////////////////////////////////////////////////////";
		out.line();
	}

	const char *ptr_sub(runtime::descptr rt)
	{
		// returns
		if (runtime::ptr_size(rt) == 4)
			return "int ";
		else if (runtime::ptr_size(rt) == 8)
			return "long long ";
		else if (runtime::ptr_size(rt) == 2)
			return "short ";
		else
			return "<unknown ptr sub>";
	}

	const char *win32_field_type(putki::field_type f)
	{
		switch (f)
		{
			case FIELDTYPE_STRING:
				return "const char*";
			case FIELDTYPE_ENUM:
			case FIELDTYPE_INT32:
				return "int";
			case FIELDTYPE_UINT32:
				return "unsigned int";
			case FIELDTYPE_BYTE:
				return "unsigned char";
			case FIELDTYPE_FLOAT:
				return "float";
			default:
				return "???";
		}
	}

	const char *rt_wrap_field_type(putki::field_type f, runtime::descptr rt)
	{
		if (f == FIELDTYPE_POINTER || f == FIELDTYPE_PATH || f == FIELDTYPE_STRING) // note string too! (because char*)
		{
			return ptr_sub(rt);
		}
		else if (f == FIELDTYPE_ENUM)
		{
			return "int";
		}
		else if (f == FIELDTYPE_BOOL)
		{
			if (rt->boolsize == 1)
				return "char";
			else if (rt->boolsize == 4)
				return "int";
			else
				return "<bool?>";
		}
		else{
			return win32_field_type(f);
		}
	}

	const char *putki_field_type_pod(putki::field_type f)
	{
		switch (f)
		{
			case FIELDTYPE_STRING:
			case FIELDTYPE_PATH:
			case FIELDTYPE_FILE:
				return "std::string";
			case FIELDTYPE_INT32:
				return "int";
			case FIELDTYPE_UINT32:
				return "unsigned int";
			case FIELDTYPE_BYTE:
				return "unsigned char";
			case FIELDTYPE_POINTER:
				return "void*";
			case FIELDTYPE_FLOAT:
				return "float";
			case FIELDTYPE_BOOL:
				return "bool";
			default:
				return 0;
		}
	}

	std::string putki_field_type(putki::parsed_field *pf)
	{
		if (pf->type == FIELDTYPE_STRUCT_INSTANCE || pf->type == FIELDTYPE_ENUM)
			return std::string("inki::") + pf->ref_type;
		else if (pf->type == FIELDTYPE_POINTER)
			return std::string("inki::") + pf->ref_type + "*";

		return putki_field_type_pod(pf->type);
	}

	// cross-platform definitions namespace encapsulation
	//
	const char *runtime_out_ns(runtime::descptr rt)
	{
		static char tmp[1024];
		sprintf(tmp, "out_ns_%s", runtime::desc_str(rt));
		return tmp;
	}

	void write_runtime_header(putki::parsed_file *file, runtime::descptr rt, putki::indentedwriter out)
	{
		std::string deftok("__outki_header" + file->filename + "__h__");

		if (!rt)
		{
			out.line() << "#ifndef " << deftok;
			out.line() << "#define " << deftok;
			out.line();
			write_includes(file, out);
			out.line() << "#include <putki/types.h>";
			out.line();
		}

		out.line();
		

		out.line() << "namespace outki {";
		out.indent(1);
		out.line() << "// Enums";

		for (size_t i=0; i<file->enums.size(); i++)
		{
			putki::parsed_enum *e = &file->enums[i];
			out.line() << "enum " << e->name;
			out.line() << "{";
			for (size_t j=0; j<e->values.size(); j++)
			{
				if (j > 0)
					out.cont() << ",";
				out.line(1) << e->values[j].name << " = " << e->values[j].value;
			}
			out.line() << "};";
		}

		out.line();
		out.indent(-1);
		out.line() << "}";
		out.line();

		for (size_t i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			out.line();
			out.line() << "namespace outki {";
			out.indent(1);
			out.line();

			out.line() << "#pragma pack(push, 1)";
			out.line() << "struct " << s->name << " {";
			out.indent(1);

			for (size_t i=0; i<s->fields.size(); i++)
			{
				putki::parsed_field *f = &s->fields[i];
				if (!(f->domains & putki::DOMAIN_RUNTIME))
					continue;

				out.line();

				if (rt &&f->is_array)
					out.cont() << ptr_sub(rt) << " ";
				else
					switch (s->fields[i].type)
					{
						case FIELDTYPE_INT32:
							out.cont() << "int ";
							break;
						case FIELDTYPE_UINT32:
							out.cont() << "unsigned int ";
							break;
						case FIELDTYPE_BYTE:
							out.cont() << "unsigned char ";
							break;
						case FIELDTYPE_FLOAT:
							out.cont() << "float ";
							break;
						case FIELDTYPE_BOOL:
							if (rt)
								out.cont() << rt_wrap_field_type(FIELDTYPE_BOOL, rt) << " ";
							else
								out.cont() << "bool ";
							break;
						case FIELDTYPE_POINTER:
						{
							if (rt)
								out.cont() << ptr_sub(rt) << " ";
							else
								out.cont() << "outki::" << f->ref_type << " *";
						}
						break;
						case FIELDTYPE_ENUM:
							if (rt)
								out.cont() << rt_wrap_field_type(FIELDTYPE_ENUM, rt) << " ";
							else
								out.cont() << "outki::" << f->ref_type << " ";
							break;
						case FIELDTYPE_STRUCT_INSTANCE:
							out.cont() << "outki::" << f->ref_type << " ";
							break;
						case FIELDTYPE_FILE:
						case FIELDTYPE_PATH:
						case FIELDTYPE_STRING:
						{
							if (rt)
								out.cont() << ptr_sub(rt) << " ";
							else
								out.cont() << "const char *";
						}
						break;
					}

				if (!rt &&f->is_array)
					out.cont() << " *";

				out.cont() << f->name << ";";

				if (s->fields[i].is_array)
					out.line() << "unsigned int " <<f->name << "_size;";
			}

			if (!s->parent.empty())
				out.line() << "inline int & rtti_type_ref() { return parent.rtti_type_ref(); }";
			else if (s->is_type_root)
				out.line() << "inline int & rtti_type_ref() { return _rtti_type; }";

			out.line();
			out.line() << "static inline int type_id() { return " << s->unique_id << "; }";
			out.line() << "enum { TYPE_ID = " << s->unique_id << " };";
			out.line();

			if (s->is_type_root || !s->parent.empty())
			{
				out.line() << "template<typename Target>";
				out.line() << "inline Target* exact_cast() { if (rtti_type_ref() == Target::type_id()) return (Target*) this; else return 0; }";
			}

			if (!s->parent.empty())
			{
				out.line() << "template<typename Target>";
				out.line() << "inline Target* up_cast() { if (" << s->unique_id << " == Target::type_id()) return (Target*)this; return parent.up_cast<Target>(); }";
			}
			else if (s->is_type_root)
			{
				out.line() << "template<typename Target>";
				out.line() << "inline Target* up_cast() { if (" << s->unique_id << " == Target::type_id()) return this; return 0; }";
			}

			out.indent(-1);
			out.line() << "};";
			out.line() << "#pragma pack(pop)";
			out.line();

			out.line() << "// loading processing for " << s->name;
			out.line() << "char* post_blob_load_" << s->name << "(" << s->name << " *d, char *beg, char *end);";

			if (!rt)
				out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker);";

			out.indent(-1);
			out.line();
			out.line() << "} // namespace outki";
			out.line();

			if (rt)
				out.line() << "char *write_" << s->name << "_aux(inki::" << s->name << " *in, outki::" << s->name << " *d, char *out_beg, char *out_end);";
		}

		if (!rt)
			out.line() << "#endif";
	}

	void write_runtime_impl(putki::parsed_file *file, runtime::descptr rt, putki::indentedwriter out)
	{
		out.line() << "// Code generated by Putki Compiler.";
		out.line();

		out.line() << "#include \"" << file->filename << ".h\"";
		out.line();
		out.line() << "// These includes go to the runtime headers";
		out.line() << "#include <putki/blob.h>";
		out.line() << "#include <putki/runtime.h>";
		out.line();
		out.line() << "namespace outki {";
		out.indent(1);

		for (size_t i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			out.line() << "char* post_blob_load_" << s->name << "(" << s->name << " *d, char *aux_cur, char *aux_end)";
			out.line() << "{";
			out.indent(1);

			if (s->is_type_root)
				out.line() << "d->rtti_type_ref() = " << s->name << "::type_id();";

			for (size_t j=0; j<s->fields.size(); j++)
			{
				putki::parsed_field *f = &s->fields[j];
				if (!(f->domains & putki::DOMAIN_RUNTIME))
					continue;

				bool needs_loop = false;
				switch (s->fields[j].type)
				{
					// field types which might need fixup
					case FIELDTYPE_STRUCT_INSTANCE:
					case FIELDTYPE_PATH:
					case FIELDTYPE_STRING:
						needs_loop = true;
						break;
					default:
						break;
				}

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
						out.line() << "aux_cur += sizeof(" << s->fields[j].ref_type << "*) * d->" << s->fields[j].name << "_size;";
					}
					else
					{
						out.line() << "d->" << s->fields[j].name << " = reinterpret_cast<" << win32_field_type(s->fields[j].type) << "*>(aux_cur);";
						out.line() << "aux_cur += sizeof(" << win32_field_type(s->fields[j].type) << ") * d->" << s->fields[j].name << "_size;";
					}

					if (needs_loop)
					{
						out.line() << "if (aux_cur > aux_end) return 0; ";
						out.line() << "for (unsigned int i=0;i<d->" << s->fields[j].name + "_size;i++)";
						out.line() << "{";
						out.indent(1);
					}
					fref = fref + "[i]";
				}

				switch (s->fields[j].type)
				{
					case FIELDTYPE_STRUCT_INSTANCE:
						out.line() << "aux_cur = outki::post_blob_load_" << s->fields[j].ref_type << "(&" << fref << ", aux_cur, aux_end);";
						if (!strcmp(s->fields[j].name.c_str(), "parent"))
							out.line() << "d->rtti_type_ref() = " << s->name << "::type_id();";
						break;
					case FIELDTYPE_PATH:
					case FIELDTYPE_STRING:
						out.line() << "aux_cur = putki::post_blob_load_string(&" << fref << ", aux_cur, aux_end);";
						break;
					default:
						break;
				}

				if (s->fields[j].is_array && needs_loop)
				{
					out.indent(-1);
					out.line() << "}";
				}
			}


			out.line() << "return aux_cur;";
			out.indent(-1);
			out.line() << "}";
		}

		out.indent(-1);
		out.line() << "}";

		write_ptr_walker(file, out, true);
	}

	void write_putki_header(putki::parsed_file *file, putki::indentedwriter out)
	{
		std::string deftok = std::string("__outki_") + file->filename + "_h__";
		out.line() << "#ifndef " << deftok;
		out.line() << "#define " << deftok;

		out.line();
		write_includes(file, out, true);
		out.line();
		out.line() << "#include <putki/builder/write.h>";
		out.line() << "#include <putki/builder/typereg.h>";
		out.line() << "#include <string>";
		out.line() << "#include <cstring>";
		out.line() << "#include <vector>";
		out.line();
		out.line() << "namespace putki { struct depwalker_i; }";
		out.line();
		out.line() << "namespace inki {";
		out.indent(1);


		for (size_t i=0; i<file->enums.size(); i++)
		{
			putki::parsed_enum *e = &file->enums[i];
			out.line() << "enum " << e->name;
			out.line() << "{";
			for (size_t j=0; j<e->values.size(); j++)
			{
				if (j > 0)
					out.cont() << ",";
				out.line(1) << e->values[j].name << " = " << e->values[j].value;
			}
			out.line() << "};";
			out.line();
			out.line() << "inline " << e->name << " " << e->name << "_from_string(const char *val)";
			out.line() << "{";
			out.indent(1);
			for (size_t j=0; j<e->values.size(); j++)
				out.line() << "if (!strcmp(val, \"" << e->values[j].name << "\")) return " << e->values[j].name << ";";
			out.line() << "return (" << e->name << ") 0;";
			out.indent(-1);
			out.line() << "}";

			out.line() << "inline const char * " << e->name << "_to_string(" << e->name << " val)";
			out.line() << "{";
			out.indent(1);
			out.line() << "switch (val)";
			out.line() << "{";
			out.indent(1);
			for (size_t j=0; j<e->values.size(); j++)
				out.line() << "case " << e->values[j].value << ": return \"" << e->values[j].name << "\";";
			out.line() << "default: return \"unknown-enum-val-" << e->name << "\";";
			out.indent(-1);
			out.line() << "}";
			out.indent(-1);
			out.line() << "}";

			out.line() << "inline const char * " << e->name << "_string_by_index(int idx)";
			out.line() << "{";
			out.indent(1);
			out.line() << "switch (idx)";
			out.line() << "{";
			out.indent(1);
			for (size_t j=0; j<e->values.size(); j++)
				out.line() << "case " << j << ": return \"" << e->values[j].name << "\";";
			out.line() << "default: return 0;";
			out.indent(-1);
			out.line() << "}";
			out.indent(-1);
			out.line() << "}";
		}

		for (size_t i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			out.line() << "void fill_" << s->name << "_from_parsed(putki::parse::node *pn, void *target_, putki::load_resolver_i *resolver);";
			out.line() << "putki::type_handler_i *get_" << s->name << "_type_handler();";
			out.line() << "putki::ext_type_handler_i *get_" << s->name << "_ext_type_handler();";


			out.line() << "struct " << s->name << " {";
			out.indent(1);

			out.line() << "static inline " << s->name << " * alloc() { return (" << s->name << " *) get_" << s->name << "_type_handler()->alloc(); }";
			out.line() << "static inline putki::type_handler_i * th() { return get_" << s->name << "_type_handler(); }";

			// default constructor

			out.line();
			out.line() << s->name << "()";
			out.line() << "{";
			out.indent(1);
			for (size_t j=0; j<s->fields.size(); j++)
			{
				if (s->fields[j].is_array)
					continue;
				if (s->fields[j].is_build_config)
					continue;

				if (!s->fields[j].def_value.empty())
					out.line() << s->fields[j].name << " = " << s->fields[j].def_value << ";";
				else
					switch (s->fields[j].type)
					{
						case FIELDTYPE_INT32:
						case FIELDTYPE_UINT32:
						case FIELDTYPE_BYTE:
						case FIELDTYPE_FLOAT:
						case FIELDTYPE_POINTER:
							if (!strcmp(s->fields[j].name.c_str(), "_rtti_type"))
								out.line() << s->fields[j].name << " = " << s->unique_id << ";";
							else
								out.line() << s->fields[j].name << " = 0;";
							break;
						case FIELDTYPE_ENUM:
							out.line() << s->fields[j].name << " = (" << s->fields[j].ref_type << ")0;";
							break;
						case FIELDTYPE_BOOL:
							out.line() << s->fields[j].name << " = false;";
							break;
						default:
							break;
					}
			}
			out.indent(-1);
			out.line() << "}";



			for (size_t j=0; j<s->fields.size(); j++)
			{
				if (j > 0)
					out.line();

				std::string cont = s->fields[j].name + ";";
				if (s->fields[j].is_build_config)
				{
					cont = "& " + s->fields[j].name + "(const char *build_config) {";
				}

				if (s->fields[j].is_array)
				{
					std::string r_type_name = putki_field_type(&s->fields[j]);
					out.line() << "std::vector<" << r_type_name << "> " << cont;
				}
				else
				{
					out.line() << putki_field_type(&s->fields[j]) << " " << cont;
				}
				
				if (s->fields[j].is_build_config)
				{
					for (unsigned i=1;;i++)
					{
						// swap last and first
						const char *test = get_build_config(i);
						const char *real = test;
						if (!test)
						{
							real = get_build_config(0);
						}
						else
						{
							out.line(1) << "if (!strcmp(build_config, \"" << real << "\"))";
						}
						out.line(test ? 2 : 1) << "return " << s->fields[j].name << real << ";";
						if (!test)
							break;
					}
					out.line(0) << "}";
				}
			}

			if (!s->parent.empty())
				out.line() << "inline " << putki_field_type_pod(FIELDTYPE_INT32) << " & rtti_type_ref() { return parent.rtti_type_ref(); }";
			else if (s->is_type_root)
				out.line() << "inline " << putki_field_type_pod(FIELDTYPE_INT32) << " & rtti_type_ref() { return _rtti_type; }";

			out.line();
			out.line() << "static inline int type_id() { return " << s->unique_id << "; }";
			out.indent(-1);
			out.line() << "};";

			if (s->domains & putki::DOMAIN_RUNTIME)
			{
				out.line() << "// writing processing for " << s->name;
				out.line() << "char *write_" << s->name << "_into_blob(inki::" << s->name << " *in, char *out_beg, char *out_end);";
			}

			out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker, bool traverseChildren, bool skipInputOnly, bool rttiDispatch);";
		}

		out.line();
		out.indent(-1);
		out.line() << "} // namespace inki";
		out.line();

		for (int i=0;; i++)
		{
			runtime::descptr rt = runtime::get(i);
			if (!rt) break;

			std::string ns = runtime_out_ns(rt);
			out.line();
			out.line() << "namespace " << ns << " {";
			out.indent(1);
			write_runtime_header(file, rt, out);
			out.indent(-1);
			out.line() << "} // namespace " << ns;
		}


		out.line() << "#endif";
	}

	void write_putki_field_parse(putki::parsed_field *f, putki::indentedwriter out)
	{
		std::string ref = "target->" + f->name;
		std::string node = "putki::parse::get_object_item(pn, \"" + f->name + "\")";

		if (f->is_array)
		{
			if (f->type == FIELDTYPE_BYTE)
			{
				out.line() << "if (!putki::parse::parse_stringencoded_byte_array(" << node << ", " << ref << "))";
				out.line() << "{";
				out.indent(1); 
			}
			
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


		out.line() << "if (" << node << ")";
		out.indent(1);

		if (f->type == FIELDTYPE_STRING || f->type == FIELDTYPE_FILE || f->type == FIELDTYPE_PATH)
		{
			out.line() << ref << " = " << " putki::parse::get_value_string(" << node << "); ";
		}
		else if (f->type == FIELDTYPE_INT32 || f->type == FIELDTYPE_UINT32 || f->type == FIELDTYPE_BYTE)
		{
			out.line() << ref << " = " << "(" << putki_field_type(f) << ") putki::parse::get_value_int(" << node << "); ";
		}
		else if (f->type == FIELDTYPE_FLOAT)
		{
			out.line() << ref << " = " << "(" << putki_field_type(f) << ") atof(putki::parse::get_value_string(" << node << ")); ";
		}
		else if (f->type == FIELDTYPE_BOOL)
		{
			out.line() << ref << " = putki::parse::get_value_int(" << node << ") != 0;";
		}
		else if (f->type == FIELDTYPE_ENUM)
		{
			out.line() << ref << " = inki::" << f->ref_type << "_from_string(putki::parse::get_value_string(" << node << "));";
		}
		else if (f->type == FIELDTYPE_STRUCT_INSTANCE)
		{
			out.line() << "fill_" << f->ref_type << "_from_parsed(" << node << ", &" << ref << ", resolver);";
		}
		else if (f->type == FIELDTYPE_POINTER)
		{
			out.line() << "{";
			out.line(1) << "const char *str = putki::parse::get_value_string(" << node << ");";
			out.line(1) << "if (!str || !str[0])";
			out.line(2) << ref << " = 0;";
			out.line(1) << "else";
			out.line(2) << "resolver->resolve_pointer((putki::instance_t *)&" << ref << ", putki::parse::get_value_string(" << node << "));";
			out.line() << "}";
		}
		out.indent(-1);

		if (f->is_array)
		{
			out.line() << "i++;";
			out.indent(-1);
			out.line() << "}";
			out.indent(-1);
			out.line() << "} // end array parse block";
			
			if (f->type == FIELDTYPE_BYTE)
			{
				out.indent(-1);
				out.line() << "} // hex";
			}
		}
	}

	void write_putki_parse(putki::parsed_file *file, putki::indentedwriter out)
	{
		out.line() << "#include <putki/builder/parse.h>";
		out.line() << "namespace inki {";
		out.line();
		out.indent(1);
		for (int i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			out.line() << "void fill_" << s->name << "_from_parsed(putki::parse::node *pn, void *target_, putki::load_resolver_i *resolver)";
			out.line() << "{";
			out.indent(1);
			out.line() << s->name << " *target = (" << s->name << " *) target_;";

			// field parsing.
			for (unsigned int j=0; j<s->fields.size(); j++)
			{
				if (s->fields[j].is_build_config)
					continue;
				
				out.line();

				if (s->fields[j].name != "_rtti_type")
					write_putki_field_parse(&s->fields[j], out);
			}

			if (s->is_type_root || !s->parent.empty())
				out.line() << "target->rtti_type_ref() = " << s->unique_id << ";";

			out.indent(-1);

			out.line() << "}";
			out.line();
		}



		out.indent(-1);
		out.line() << "} // namespace inki";
		out.line();
	}

	const char *runtime_equals(runtime::descptr rt)
	{
		static char tmp[4096];
		sprintf(tmp, "(int(rt->platform) == %d && rt->ptrsize == %d && rt->low_byte_first == %s)", rt->platform, rt->ptrsize, rt->low_byte_first ? "true" : "false");
		return tmp;
	}

	void write_blob_writer_call(runtime::descptr rt, const char *name, putki::indentedwriter out)
	{
		out.line() << "if (" << runtime_equals(rt) << ")";
		out.line(1) << "return " << runtime_out_ns(rt) << "::write_" << name << "_into_blob((" << name << "*) source, beg, end);";
	}

	void write_ptr_walker(putki::parsed_file *file, putki::indentedwriter out, bool runtime)
	{
		write_comment_block("Dependency walking", out);

		const char *ns = runtime ? "outki" : "inki";

		out.line();
		out.line() << "namespace " << ns << " {";
		out.line();
		out.indent(1);

		for (int i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (runtime && !(s->domains & putki::DOMAIN_RUNTIME))
				continue;
			
			if (runtime)
				out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker)";
			else
				out.line() << "void walk_dependencies_" << s->name << "(" << s->name << " *input, putki::depwalker_i *walker, bool traverseChildren, bool skipInputOnly, bool rttiDispatch)";
					
			out.line() << "{";
			out.indent(1);
		
			if ((s->is_type_root || !s->parent.empty()) && !runtime)
			{
				// here we detect the actual type and call into the right one... in the runtime we never end up walking dependencies
				// and calling into the root type in a chain, but we might when loading unknown data.
				out.line() << "if (!rttiDispatch) {";
				out.line(1) << "putki::typereg_get_handler(input->rtti_type_ref())->walk_dependencies(input, walker";
				if (!runtime)
					out.cont() << ", traverseChildren, skipInputOnly, true";
				out.cont() << ");";
				out.line(1) << "return;";
				out.line() << "}";
			}
			
			const char *levelCheck = runtime ? "" : "if (traverseChildren) ";
			const char *traverseArgs  = runtime ? "" : ", traverseChildren";

			for (size_t j=0; j<s->fields.size(); j++)
			{
				putki::parsed_field & fd = s->fields[j];
				if (fd.is_build_config)
					continue;

				if (runtime && !(fd.domains & putki::DOMAIN_RUNTIME))
					continue;
					
				// only these two can have dependencies.
				if (fd.type != putki::FIELDTYPE_STRUCT_INSTANCE && fd.type != putki::FIELDTYPE_POINTER)
					continue;

				bool skip_check = false;
				if (!runtime)
				{
					out.line() << "if (!skipInputOnly || putki::typereg_get_handler(\"" << fd.ref_type << "\")->in_output())";
					out.line() << "{";
					skip_check = true;
				}

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
				
				std::string inkiArgs;
				if (!runtime)
				{
					inkiArgs = (fd.name == "parent") ? ", skipInputOnly, true" : ", skipInputOnly, false";
				}
				
				if (fd.type == putki::FIELDTYPE_STRUCT_INSTANCE)
				{
					// structs are not considered chlidren
					out.line() << "walk_dependencies_" << fd.ref_type << "(&" << ref << ", walker";
					out.cont() << traverseArgs << inkiArgs << ");";
				}
				else if (fd.type == putki::FIELDTYPE_POINTER)
				{
					if (runtime)
						out.line() << "if (walker->pointer_pre_filter((putki::instance_t *)&" << ref << "))";
					else
						out.line() << "if (walker->pointer_pre_filter((putki::instance_t *)&" << ref << ", \"" << fd.ref_type << "\"))";

					out.line() << "{";
					out.line(1) << "if (" << ref << ") { " << levelCheck << " walk_dependencies_" << fd.ref_type << "(" << ref << ", walker "
					            << (runtime ? "" : ", true ") << inkiArgs << "); }";
					out.line() << "}";
					out.line() << "walker->pointer_post((putki::instance_t *)&" << ref << ");";
				}

				if (fd.is_array)
				{
					out.indent(-1);
					out.line() << "} // end array loop";
				}
				
				if (skip_check)
				{
					out.indent(-1);
					out.line() << "}";
				}
			}

			out.indent(-1);
			out.line() << "}";
			out.line();
		}

		out.indent(-1);
		out.line() << "} // dependency walker section";
	}

	void write_json_writer(putki::parsed_struct *s, putki::indentedwriter out)
	{
		out.line() << s->name << " * input = (" << s->name << " *)source;";
		out.line() << "char ibuf[128];";

		// copy and massage a little.
		std::vector<putki::parsed_field> copy = s->fields;
		for (size_t i=0; i<copy.size(); i++) {
			// this is internal and should not be written
			if (!strcmp(copy[i].name.c_str(), "_rtti_type")) {
				copy.erase(copy.begin() + i);
				--i;
			}
		}

		for (size_t j=0; j<copy.size(); j++)
		{
			putki::parsed_field & fd = copy[j];
			if (fd.is_build_config)
				continue;

			out.line();

			std::string ref = "input->" + fd.name;
			std::string delim = "";

			out.line() << "out << putki::write::json_indent(ibuf, indent+1) << \"\\\"" << fd.name << "\\\": \";";

			if (fd.is_array && fd.type == FIELDTYPE_BYTE)
			{
				out.line() << "out << \"\\\"\"; putki::write::json_stringencode_byte_array(out, " << ref << "); out << \"\\\"\";";
				if (j < copy.size()-1)
					out.line() << "out << \",\";";
				out.line() << "out << \"\\n\";";
				continue;
			}

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

			if (fd.type == FIELDTYPE_STRING || fd.type == FIELDTYPE_FILE || fd.type == FIELDTYPE_PATH)
			{
				out.line() << "out << " << delim << "putki::write::json_str(" << ref << ".c_str());";
			}
			else if (fd.type == FIELDTYPE_INT32 || fd.type == FIELDTYPE_UINT32 || fd.type == FIELDTYPE_BYTE)
			{
				out.line() << "out << " << delim << "(int)" << ref << ";";
			}
			else if (fd.type == FIELDTYPE_UINT32)
			{
				out.line() << "out << " << delim << "(unsigned int)" << ref << ";";
			}
			else if (fd.type == FIELDTYPE_FLOAT || fd.type == FIELDTYPE_BOOL)
			{
				out.line() << "out << " << delim << "" << ref << ";";
			}
			else if (fd.type == FIELDTYPE_ENUM)
			{
				out.line() << "out << " << delim << " \"\\\"\" << inki::" << fd.ref_type << "_to_string(" << ref << ") << \"\\\"\" ;";
			}
			else if (fd.type == FIELDTYPE_POINTER)
			{
				out.line() << "out << " << delim <<  "putki::write::json_str(putki::db::pathof_including_unresolved(ref_source, " << ref << "));";
			}
			else if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
			{
				out.line() << "out << " << delim << "\"{\\n\";";
				out.line() << "// write struct contents[" << fd.ref_type << "]";
				out.line() << "{";
				out.line(1) << "putki::type_handler_i *thandler = inki::get_" << fd.ref_type << "_type_handler();";
				out.line(1) << "thandler->write_json(ref_source, &" << ref << ", out, indent + 1);";
				out.line() << "}";
				out.line() << "out << putki::write::json_indent(ibuf, indent+1) << \"}\";";
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
				out.line() << "out << \"]\";";
				out.indent(-1);
				out.line() << "} // end array";
			}

			if (j < copy.size()-1)
				out.line() << "out << \",\";";

			out.line() << "out << \"\\n\";";
		}
	}

	void write_putki_type_reg(putki::parsed_file *file, putki::indentedwriter out)
	{
		write_comment_block("Type registration", out);

		out.line() << "#include <putki/builder/typereg.h>";
		out.line() << "#include <putki/builder/write.h>";
		out.line() << "#include <putki/builder/db.h>";
		out.line() << "#include <putki/runtime.h>";
		out.line() << "#include <putki/sys/sstream.h>";
		out.line();
		out.line() << "namespace inki {";
		out.indent(1);
		out.line();
		for (int i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line();
			out.line() << "// type handler for " << s->name;
			out.line() << "struct " << s->name << "_handler : public putki::type_handler_i {";
			out.indent(1);
			out.line();
			out.line() << "// parent=" << s->parent << " is_type_root=" << s->is_type_root;;

			if ((!s->parent.empty()) || s->is_type_root)
				out.line() << "putki::instance_t alloc() { " << s->name << " *tmp = new " << s->name << "; tmp->rtti_type_ref() = " << s->unique_id << "; return tmp; }";
			else
				out.line() << "putki::instance_t alloc() { return new " << s->name << "; }";

			out.line() << "putki::instance_t clone(putki::instance_t source) { " << s->name << " *tmp = (" << s->name << "*) alloc(); *tmp = *((" << s->name << " *)source); return tmp; }";
			out.line() << "void free(putki::instance_t p) { delete (" << s->name << "*) p; }";
			out.line();
			out.line() << "// info";
			out.line() << "const char *name() { return \"" << s->name << "\"; }";
			out.line() << "type_handler_i *parent_type() { return ";
			if (!s->parent.empty())
				out.cont() << "get_" << s->parent << "_type_handler();";
			else
				out.cont() << "0;";
			out.cont() << " }";
			
			out.line() << "int id() { return " << s->unique_id << "; }";
			out.line() << "bool in_output() { return " << (s->domains & putki::DOMAIN_RUNTIME ? "true" : "false") << "; }";
			out.line();
			out.line() << "// deps";
			out.line() << "void walk_dependencies(putki::instance_t source, putki::depwalker_i *walker, bool traverseChildren, bool skipInputOnly, bool rttiDispatch) {";
			out.line(1) << "walk_dependencies_" << s->name << "( (" << s->name << " *) source, walker, traverseChildren, skipInputOnly, rttiDispatch);";
			out.line() << "}";
			out.line();
			out.line() << "// json writer";
			out.line() << "void write_json(putki::db::data *ref_source, putki::instance_t source, putki::sstream & out, int indent)";
			out.line() << "{";
			out.indent(1);
			write_json_writer(s, out);
			out.indent(-1);
			out.line() << "}";
			out.line();
			out.line() << "// writer";
			out.line() << "char* write_into_buffer(putki::runtime::descptr rt, putki::instance_t source, char *beg, char *end)";
			out.line() << "{";

			out.indent(1);

			if (s->domains & putki::DOMAIN_RUNTIME)
			{
				for (int p=0;; p++)
				{
					// blob writers for all runtimes.
					runtime::descptr t = runtime::get(p);
					if (!t) break;
					write_blob_writer_call(t, s->name.c_str(), out);
				}
			}

			out.line() << "return 0;";
			out.indent(-1);
			out.line() << "}";

			out.line();
			out.line();
			out.line() << "void fill_from_parsed(putki::parse::node *pn, putki::instance_t target, putki::load_resolver_i *resolver) {";
			out.line(1) << "fill_" << s->name << "_from_parsed(pn, target, resolver);";
			out.line() << "}";
			out.line();
			out.indent(-1);
			out.line() << "} s_" << s->name << "_handler;";
			out.line();
			out.line() << "void bind_type_" << s->name << "() { putki::typereg_register(\"" << s->name << "\", &s_" << file->structs[i].name << "_handler); } ";
			out.line() << "putki::type_handler_i *get_" << s->name << "_type_handler() { return &s_" << s->name << "_handler; }";
		}

		out.indent(-1);
		out.line() << "}";
	}

	void write_blob_writers(putki::parsed_file *file, putki::indentedwriter out, runtime::descptr rt)
	{
		std::string ns = runtime_out_ns(rt);
		out.line() << "namespace " << ns << " {";
		out.indent(1);

		std::string out_ns = ns + "::outki::";

		out.line();
		out.line() << "namespace { typedef unsigned short length_t; }";
		out.line();
		for (size_t i=0; i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			out.line();
			out.line() << "char *write_" << s->name << "_aux(inki::" << s->name << " *in, " << out_ns << s->name << " *d, char *out_beg, char *out_end)";
			out.line() << "{";
			out.indent(1);

			for (size_t j=0; j<s->fields.size(); j++)
			{
				putki::parsed_field & fd = s->fields[j];

				if (!(fd.domains & putki::DOMAIN_RUNTIME))
					continue;

				std::string srcd = "in->" + fd.name;
				std::string outd = "d->" + fd.name;

				if (fd.is_array)
				{
					std::string ft = rt_wrap_field_type(fd.type, rt);
					if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
						ft = out_ns + fd.ref_type;

					out.line() << outd << "_size = " << srcd << ".size();";
					out.line();
					out.line() << "{";
					out.indent(1);
					out.line() << ft << " *inp = reinterpret_cast<" << ft << " *>(out_beg);";
					out.line() << "out_beg += sizeof(" << ft << ") * " << outd << "_size;";
					out.line() << "for (unsigned int i=0;i<" << outd << "_size;i++)";
					out.line() << "{";
					out.indent(1);
					srcd = srcd + "[i]";
					outd = "inp[i]";
				}

				if (fd.type == FIELDTYPE_STRING || fd.type == FIELDTYPE_PATH)
					out.line() << "out_beg = putki::pack_string_field(" << rt->ptrsize << ", (char*) &" << outd << ", " << srcd << ".c_str(), out_beg, out_end);";
				else if (fd.type == FIELDTYPE_INT32 || fd.type == FIELDTYPE_UINT32)
					out.line() << "putki::pack_int32_field((char*)&" << outd << ", " << srcd << ");";
				else if (fd.type == FIELDTYPE_UINT32)
					out.line() << "putki::pack_int32_field((char*)&" << outd << ", (int)" << srcd << ");";
				else if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
					out.line() << "out_beg = write_" << fd.ref_type << "_aux(&" << srcd << ", &" << outd << ", out_beg, out_end);";
				else if (fd.type == FIELDTYPE_POINTER)
					out.line() << outd << " = (" << ptr_sub(rt) << ")((char*)" << srcd << " - (char*)0);";
				else if (fd.type == FIELDTYPE_ENUM)
					out.line() << outd << " = (" << rt_wrap_field_type(fd.type, rt) << ") " << srcd << ";";
				else if (fd.type == FIELDTYPE_BOOL)
					out.line() << outd << " = (" << rt_wrap_field_type(fd.type, rt) << ") " << srcd << ";";
				else if (fd.type == FIELDTYPE_BYTE || fd.type == FIELDTYPE_FLOAT)
					out.line() << outd << " = " << srcd << ";";

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

	void write_putki_impl(putki::parsed_file *file, putki::indentedwriter out)
	{
		out.line() << "// Code generated by Putki Compiler!";
		out.line() << "#include \"" << file->filename << ".h\"";
		out.line() << "#include <putki/blob.h>";
		out.line() << "#include <putki/builder/typereg.h>";
		out.line() << "#include <putki/sys/sstream.h>";
		out.line() << "#include <putki/runtime.h>";
		out.line() << "#include <cstring>";

		out.line();

		// for cross-writing
		for (int p=0;; p++)
		{
			// blob writers for all runtimes.
			runtime::descptr t = runtime::get(p);
			if (!t) break;
			write_blob_writers(file, out, t);
		}

		write_putki_parse(file, out);

		write_ptr_walker(file, out, false);
		write_putki_type_reg(file, out);
	}

	void write_bind_decl(putki::parsed_file *file, putki::indentedwriter out)
	{
		out.line() << "namespace inki {";
		for (unsigned int i=0; i<file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "void bind_type_" << s->name << "();";
		}
		out.line() << "}";
	}

	void write_bind_calls(putki::parsed_file *file, putki::indentedwriter out)
	{
		for (unsigned int i=0; i<file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "inki::bind_type_" << s->name << "();";
		}
	}

	void write_runtime_blob_load_cases(putki::parsed_file *file, putki::indentedwriter out)
	{
		for (unsigned int i=0; i<file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			out.line() << "case " << s->unique_id << ":";
			out.line() << "{";
			out.line(1) << "char *out = post_blob_load_" << s->name << "((" << s->name << "*)begin, begin + sizeof(" << s->name << "), end);";
			out.line(1) << "if (out) walk_dependencies_" << s->name << "((" << s->name << "*)begin, ptr_reg);";
			out.line(1) << "return out;";
			out.line() << "}";
		}
	}

	void write_runtime_blob_load_decl(const char *hpath, putki::indentedwriter out)
	{
		out.line() << "#include <" << hpath << ">";
	}
}
