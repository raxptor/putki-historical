#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{
	void write_runtime_header(putki::parsed_file *file, putki::runtime rt, std::ostream &out)
	{
		out << "#pragma pack(push, 1)" << std::endl;
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			if (rt == putki::RUNTIME_CPP_WIN32)
			{				
				out << "namespace outki {" << std::endl;
				out << "	struct " << s->name << " {" << std::endl;

				for (size_t i=0;i<s->fields.size();i++)
				{
					out << "		";
					switch (s->fields[i].type)
					{
						case FIELDTYPE_INT32:
							out << "int ";
							break;
						case FIELDTYPE_POINTER:
							out << s->fields[i].ref_type << " *";
							break;
						case FIELDTYPE_STRUCT_INSTANCE:
							out << s->fields[i].ref_type << " ";
							break;
						case FIELDTYPE_STRING:
							out << "const char *";
							break;
					}

					if (s->fields[i].is_array)
						out << "*";

					out << s->fields[i].name << ";" <<std::endl;

					if (s->fields[i].is_array)
						out << "		int " << s->fields[i].name << "_count;" << std::endl;
				}

				out << "	};" << std::endl;
				out << "}" << std::endl;				
			}
		}
		out << "#pragma pack(pop)" << std::endl;
	}

	const char *win32_field_type(putki::field_type f)
	{
		switch (f)
		{
			case FIELDTYPE_STRING:
				return "const char*";
			case FIELDTYPE_INT32:
				return "int";
			default:
				return "???";
		}
	}

	void write_runtime_impl(putki::parsed_file *file, putki::runtime rt, std::ostream &out)
	{		
		out << "namespace outki {" << std::endl;
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			if (rt == putki::RUNTIME_CPP_WIN32)
			{								
				out << "	char* post_blob_load_" << s->name << "(" << s->name << " *d, char *aux_cur, char *aux_end)" << std::endl;
				out << "	{" << std::endl;

				for (size_t j=0;j<s->fields.size();j++)
				{				
					if (j > 0)
						out << std::endl;
					out << "		// field " << s->fields[j].name << std::endl;
					if (s->fields[j].is_array)
					{
						if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
						{
							out << "		" << "d->" << s->fields[j].name << " = reinterpret_cast<" << s->fields[j].ref_type << "*>(aux_cur);" << std::endl;
							out << "		" << "aux_cur += sizeof(" << s->fields[j].ref_type << ") * d->" << s->fields[j].name << "_count;" << std::endl;
						}
						else if (s->fields[j].type == FIELDTYPE_POINTER)
						{
							out << "		" << "d->" << s->fields[j].name << " = reinterpret_cast<" << s->fields[j].ref_type << "**>(aux_cur);" << std::endl;
							out << "		" << "aux_cur += sizeof(" << s->fields[j].ref_type << "*) * d->" << s->fields[j].name << "_count;" << std::endl;
						}
						else
						{
							out << "		" << "d->" << s->fields[j].name << " = reinterpret_cast<" << win32_field_type(s->fields[j].type) << "*>(aux_cur);" << std::endl;
							out << "		" << "aux_cur += sizeof(" << win32_field_type(s->fields[j].type) << ") * d->" << s->fields[j].name << "_count;" << std::endl;
						}
						

						out << "		for (int i=0;i<d->" << s->fields[j].name + "_count;i++)" << std::endl;
					}

					switch (s->fields[j].type)
					{
						case FIELDTYPE_POINTER:
							if (s->fields[j].is_array)
								out << "			resolve_" << s->fields[j].ref_type << "_ptr(&d->" << s->name << "[i]);" << std::endl;
							else
								out << "		resolve_" << s->fields[j].ref_type << "_ptr(d->" << s->name << ");" << std::endl;
							break;
						case FIELDTYPE_STRUCT_INSTANCE:							

							if (s->fields[j].is_array)
								out << "			aux_cur = post_blob_load_" << s->fields[j].ref_type << "(&s->" << s->fields[j].name << "[i], aux_cur, aux_end);" << std::endl;
							else
								out << "		aux_cur = post_blob_load_" << s->fields[j].ref_type << "(&s->" << s->fields[j].name << ", aux_cur, aux_end);" << std::endl;
							
							break;						
						case FIELDTYPE_STRING:
							if (s->fields[j].is_array)
								out << "			aux_cur = post_blob_load_string(&s->" << s->fields[j].name << "[i], aux_cur, aux_end);" << std::endl;
							else
								out << "		aux_cur = post_blob_load_string(&s->" << s->fields[j].name << ", aux_cur, aux_end);" << std::endl;							
							break;
						default:												
							if (s->fields[j].is_array)
								out << "			;" << std::endl;
							break;
					}

				/*
					if (s->fields[i].is_array)
						out << "*";

					out << s->fields[i].name << ";" <<std::endl;

					if (s->fields[i].is_array)
						out << "		int " << s->fields[i].name << "_count;" << std::endl;						
				}
				*/				
				}

				out << "		return aux_cur;" << std::endl;
				out << "	}" << std::endl;				
			}
		}
		out << "}" << std::endl;
	}

	void write_meta_header(putki::parsed_file *file, std::ostream &out)
	{

	}

	void write_meta_impl(putki::parsed_file *file, std::ostream &out)
	{

	}
}
