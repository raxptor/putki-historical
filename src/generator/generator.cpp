#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{    
	void write_runtime_header(putki::parsed_file *file, putki::runtime rt, std::ostream &out, bool for_putki)
	{
        std::string ptr_subst("long long ");
        
        if (rt == putki::RUNTIME_CPP_WIN32)
            std::string ptr_subst("int ");
        
		out << "#pragma pack(push, 1)" << std::endl;
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;
            
			if (rt == putki::RUNTIME_CPP_WIN32 || rt == putki::RUNTIME_CPP_WIN64)
			{				
				out << "namespace outki {" << std::endl;
				out << "	struct " << s->name << " {" << std::endl;

				for (size_t i=0;i<s->fields.size();i++)
				{
					out << "		";
                    
                    if (for_putki && s->fields[i].is_array)
                        out << ptr_subst << " ";
                    else
                    {
					switch (s->fields[i].type)
					{
						case FIELDTYPE_INT32:
							out << "int ";
							break;
						case FIELDTYPE_POINTER:
                            {
                                if (for_putki)
                                    out << ptr_subst << " ";
                                else
                                    out << s->fields[i].ref_type << " *";
                            }
                            break;
						case FIELDTYPE_STRUCT_INSTANCE:
							out << s->fields[i].ref_type << " ";
							break;
						case FIELDTYPE_STRING:
                            {
                                if (for_putki)
                                    out << ptr_subst << " ";
                                else
                                    out << "const char *";      
                            }
							break;
					}
                    }
                    
                    if (!for_putki && s->fields[i].is_array)
                        out << " *";

					out << s->fields[i].name << ";" <<std::endl;

					if (s->fields[i].is_array)
						out << "		int " << s->fields[i].name << "_count;" << std::endl;
				}

				out << "	};" << std::endl;
                out << "    char* post_blob_load_" << s->name << "(" << s->name << " *d, char *beg, char *end);" << std::endl;
				out << "}" << std::endl;				
			}
		}
		out << "#pragma pack(pop)" << std::endl;
	}
    
 	void write_runtime_header(putki::parsed_file *file, putki::runtime rt, std::ostream &out)
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
			default:
				return "???";
		}
	}
    
	const char *putki_field_type(putki::field_type f)
	{
		switch (f)
		{
			case FIELDTYPE_STRING:
				return "std::string";
			case FIELDTYPE_INT32:
				return "int";
			default:
				return "???";
		}
	}

	void write_runtime_impl(putki::parsed_file *file, putki::runtime rt, std::ostream &out)
	{
        std::string deftok = std::string("__OUTKI_") + file->filename + "_H__";
        out << "// Generated code!" << std::endl;
        out << "#ifndef " << deftok << std::endl;
        out << "#define " << deftok << std::endl;
        out << std::endl;
        out << "#include \"" << file->filename << ".h\"" << std::endl;
        out << "#include <putki/blob.h>" << std::endl;
		out << "namespace outki {" << std::endl;
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			if (rt == putki::RUNTIME_CPP_WIN32 || rt == putki::RUNTIME_CPP_WIN64)
			{
				out << "	char* post_blob_load_" << s->name << "(" << s->name << " *d, char *aux_cur, char *aux_end)" << std::endl;
				out << "	{" << std::endl;

				for (size_t j=0;j<s->fields.size();j++)
				{				
					if (j > 0)
						out << std::endl;
					out << "		// field " << s->fields[j].name << std::endl;
					
                    std::string fref = std::string("d->") + s->fields[j].name;
                    
                    if (s->fields[j].is_array)
					{
						if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
						{
							out << "		" << "d->" << s->fields[j].name << " = reinterpret_cast<outki::" << s->fields[j].ref_type << "*>(aux_cur);" << std::endl;
                            out << "        aux_cur += sizeof(" << s->fields[j].ref_type << ") * d->" << s->fields[j].name << "_count;" << std::endl;
						}
						else if (s->fields[j].type == FIELDTYPE_POINTER)
						{
							out << "		" << "d->" << s->fields[j].name << " = reinterpret_cast<" << s->fields[j].ref_type << "**>(aux_cur);" << std::endl;
						}
						else
						{
							out << "		" << "d->" << s->fields[j].name << " = reinterpret_cast<" << win32_field_type(s->fields[j].type) << "*>(aux_cur);" << std::endl;
                            out << "        aux_cur += sizeof(" << win32_field_type( s->fields[j].type ) << ") * d->" << s->fields[j].name << "_count;" << std::endl;
                         
						}

                        out << " if (aux_cur > aux_end) return 0; " << std::endl;
						out << "		for (int i=0;i<d->" << s->fields[j].name + "_count;i++)" << std::endl;

                        out << "{" << std::endl;
                        fref = fref + "[i]";
					}

					switch (s->fields[j].type)
					{
                            /*
						case FIELDTYPE_POINTER:
							if (s->fields[j].is_array)
								out << "			resolve_" << s->fields[j].ref_type << "_ptr(&d->" << s->name << "[i]);" << std::endl;
							else
								out << "		resolve_" << s->fields[j].ref_type << "_ptr(d->" << s->name << ");" << std::endl;
							break;
                             */
						case FIELDTYPE_STRUCT_INSTANCE:
								out << "		aux_cur = post_blob_load_" << s->fields[j].ref_type << "(&" << fref << ", aux_cur, aux_end);" << std::endl;

							break;
                        case FIELDTYPE_INT32:
                                out << "            prep_int32_field((char*)&" << fref <<  ");" << std::endl;
                            break;
						case FIELDTYPE_STRING:
								out << "		aux_cur = post_blob_load_string(&" << fref << ", aux_cur, aux_end);" << std::endl;
							break;
						default:
							break;
					}
                    
                    if (s->fields[j].is_array)
                        out << "}" << std::endl;

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
        out << "#endif" << std::endl;
	}

	void write_putki_header(putki::parsed_file *file, std::ostream &out)
	{
        out << "#include <string>" << std::endl;
        out << "#include <vector>" << std::endl;
        out << std::endl;
		out << "namespace putki {" << std::endl;
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
            
            out << "struct " << s->name << " {" << std::endl;
            
            for (size_t j=0;j<s->fields.size();j++)
            {
                if (j > 0)
                    out << std::endl;
                
                out << "        // field " << s->fields[j].name << std::endl;
            
                if (s->fields[j].is_array)
                {
                    std::string r_type_name = putki_field_type(s->fields[j].type);
                    if (s->fields[j].type == FIELDTYPE_STRUCT_INSTANCE)
                        r_type_name = s->fields[j].ref_type;
                    else if (s->fields[j].type == FIELDTYPE_POINTER)
                        r_type_name = s->fields[j].ref_type + "*";
                        
                    out << "        std::vector<" << r_type_name << "> " << s->fields[j].name << ";" << std::endl;
                }
                else
                {
                    out << "        " << putki_field_type(s->fields[j].type) << " " << s->fields[j].name
                    << ";" << std::endl;
                }
            }
    
            out << "};" << std::endl;
            
            if (s->domains & putki::DOMAIN_RUNTIME)
            {
                /*
                out << " char *write_" << s->name << "_aux(" << s->name << " *in, outki::" << s->name << " *d, char *out_beg, char *out_end);" << std::endl;
                 */
                out << " char *write_" << s->name << "_into_blob(" << s->name << " *in, char *out_beg, char *out_end);" << std::endl;
            }
        }
        out << "} // namespace putki" << std::endl;
	}

	void write_putki_impl(putki::parsed_file *file, std::ostream &out)
	{
        out << "// Generated code!" << std::endl;
        out << "#include \"" << file->filename << ".h\"" << std::endl;
        out << "#include <putki/blob.h>" << std::endl;
        out << "#include <iostream>" << std::endl;
       
        out << std::endl;
        
        // for cross-writing
        std::string ns("out_ns");
        out << "namespace " << ns << " {" << std::endl;
        write_runtime_header(file, putki::RUNTIME_CPP_WIN64, out, true);
        out << "}" << std::endl;
        std::string out_ns(ns + "::outki::");
        
        out << std::endl;
		out << "namespace putki {" << std::endl;
        out << " namespace { typedef unsigned short length_t; }";
        out << std::endl;
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;
            
            out << "char *write_" << s->name << "_aux(" << s->name << " *in, " << out_ns << s->name << " *d, char *out_beg, char *out_end)" << std::endl;
            out << "{" << std::endl;

            for (size_t j=0;j<s->fields.size();j++)
            {
                if (j > 0)
                    out << std::endl;
            
                putki::parsed_field & fd = s->fields[j];
                out << "        // field " << fd.name << std::endl;
                
                std::string srcd = "in->" + fd.name;
                std::string outd = "d->" + fd.name;
                
                if (fd.is_array)
                {
                    std::string ft = win32_field_type(fd.type);
                    if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
                        ft = out_ns + fd.ref_type;
                    
                    out << "    " << outd << "_count = " << srcd << ".size();" << std::endl;
                    out << "{" << std::endl;
                    out << "    " << ft << " *inp = reinterpret_cast<" << ft << " *>(out_beg);" << std::endl;
                    out << "   out_beg += sizeof(" << ft << ") * " << outd << "_count;" << std::endl;
                    out << "    for (unsigned int i=0;i<" << outd << "_count;i++)" << std::endl;
                    out << "    {" << std::endl;
                    srcd = srcd + "[i]";
                    outd = "inp[i]";
                }

                if (fd.type == FIELDTYPE_STRING)
                {
                    out << "out_beg = pack_string_field((char*) &" << outd << ", " << srcd << ".c_str(), out_beg, out_end);";
                }
                else if (fd.type == FIELDTYPE_INT32)
                {
                    out << "pack_int32_field((char*)&" << outd << ", " << srcd << ");" << std::endl;
                }
                else if (fd.type == FIELDTYPE_STRUCT_INSTANCE)
                {
                    out << " out_beg = write_" << fd.ref_type << "_aux(&" << srcd << ", &" << outd << ", out_beg, out_end);" << std::endl;
                }
                
                if (fd.is_array)
                    out << "}}" << std::endl;
            }
            
            out << "return out_beg;" << std::endl;
            out << "}" << std::endl;
            
            std::string out_n(out_ns + s->name);
            
            out << " char *write_" << s->name << "_into_blob(" << s->name << " *in, char *out_beg, char *out_end)" << std::endl;
            out << "{" << std::endl;
            out << "   std::cout << \"packing a " << s->name;
            out << " (\" << sizeof(" << out_n;
            out << ") << \")!\" << std::endl;" << std::endl;
            out << "  if (out_end - out_beg < sizeof(" << out_n << ")) return 0; " << std::endl;
            out << "  " << out_n << " *d = (" << out_n << " *) out_beg;" << std::endl;
            out << "  return write_" << s->name << "_aux(in, d, out_beg + sizeof(" << out_n << "),  out_end);" << std::endl;
            out << "}" << std::endl;
           
        }
        out << "}" << std::endl;
	}
}
