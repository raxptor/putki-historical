#include "netki-generator.h"
#include "writetools/save_stream.h"
#include "writetools/indentedwriter.h"

#include <string>
#include <sstream>

namespace putki
{
	bool is_netki_struct(parsed_struct *s)
	{
		for (int i=0;i!=s->targets.size();i++)
			if (!strcmp(s->targets[i].c_str(), "netki"))
				return true;
		return false;
	}

	void write_struct_header(parsed_struct *s, indentedwriter &wr)
	{
		wr.line();
		wr.line() << "//";
		wr.line() << "struct " << s->name;
		wr.line() << "{";
		wr.indent(1);

		wr.line() << "// Fields defined by user";

		std::stringstream constructor;
		indentedwriter cw(constructor);
		cw.indent(3);

		for (int i=0;i!=s->fields.size();i++)
		{
			parsed_field *field = &s->fields[i];

			std::string type = "err";
			switch (field->type)
			{
				case FIELDTYPE_BOOL:
					type = "bool";
					break;
				case FIELDTYPE_INT32:
					type = "uint32_t";
					break;
				case FIELDTYPE_BYTE:
					type = "uint8_t";
					break;
				case FIELDTYPE_STRING:
					type = "const char*";
					break;
				case FIELDTYPE_POINTER:
					type = field->ref_type + "*";
					break;
				case FIELDTYPE_ENUM:
				case FIELDTYPE_STRUCT_INSTANCE:
					type = field->ref_type;
					break;
				default:
					type = "unknown!";
					break;
			}

			std::string defval = field->def_value;
			if (defval.empty())
				defval = "0";

			if (field->is_array)
			{
				wr.line() << "uint32_t " << field->name << "_size;";
				wr.line() << type << "* " << field->name << ";";
				cw.line() << field->name << "_size = 0;";
			}
			else
			{
				wr.line() << type << " " << field->name << ";";
				cw.line() << field->name << " = " << defval << ";";
			}
		}

		wr.line();
		wr.line() << "// Automatically generated fields";
		wr.line() << "static inline int type_id() { return " << s->unique_id << "; }";
		wr.line() << "static bool write_into_bitstream(" << s->name << " *source, netki::bitstream::buffer *dest);";
		wr.line() << "static bool parse_from_bitstream(" << s->name << " *target, netki::bitstream::buffer *dest);";
		wr.line();
		wr.line() << s->name << "()";
		wr.line() << "{";
		wr.cont() << constructor.str();
		wr.indent(1);
		wr.indent(-1);
		wr.line() << "}";

		wr.indent(-1);
		wr.line() << "};";
	}

	void write_struct_impl(parsed_struct *s, indentedwriter &wr)
	{
		wr.line() << "bool " << s->name << "::write_into_bitstream(" << s->name << " *source, netki::bitstream::buffer *dest)";
		wr.line() << "{";
		wr.indent(1);

		for (int i=0;i!=s->fields.size();i++)
		{
			parsed_field *field = &s->fields[i];

			std::string field_ref = std::string("source->") + field->name;
			if (field->is_array)
			{
				wr.line() << "{";
				wr.indent(1);
				wr.line() << "bitstream::insert_bits<32>(dest, " << field_ref << "_size);";
				wr.line() << "for (int i=0;i!=" << field_ref << "_size;i++)";
				wr.line() << "{";
				wr.indent(1);
				field_ref.append("[i]");
			}

			switch (field->type)
			{
				case FIELDTYPE_BOOL:
					wr.line() << "bitstream::insert_bits<1>(dest, " << field_ref << ");";
					break;
				case FIELDTYPE_INT32:
					wr.line() << "bitstream::insert_bits<32>(dest, " << field_ref << ");";
					break;
				case FIELDTYPE_BYTE:
					wr.line() << "bitstream::insert_bits<8>(dest, " << field_ref << ");";
					break;
				case FIELDTYPE_STRING:
					wr.line() << "bitstream::insert_bits<16>(dest, strlen(" << field_ref << "));";
					wr.line() << "bitstream::insert_bytes(dest, (uint8_t*) " << field_ref << ", strlen(" << field_ref << "));";
					break;
				case FIELDTYPE_ENUM:
					wr.line() << "bitstream::insert_bits<32>(dest, (int)" << field_ref << ");";
					break;
				case FIELDTYPE_STRUCT_INSTANCE:
					wr.line() << field->ref_type << "::write_into_bitstream(&" << field_ref << ", dest);";
					break;
				default:
					wr.line() << "<compile error trying to write pointer field " << s->name << ">";
					break;
			}

			if (field->is_array)
			{
				wr.indent(-1);
				wr.line() << "}";
				wr.indent(-1);
				wr.line() << "}";
			}
		}


		wr.line() << "return false;";
		wr.indent(-1);
		wr.line() << "}";
		wr.line();
		wr.line() << "bool " << s->name << "::parse_from_bitstream(" << s->name << " *source, netki::bitstream::buffer *dest)";
		wr.line() << "{";
		wr.indent(1);
		wr.line() << "return false;";
		wr.indent(-1);
		wr.line() << "}";
		wr.line();
	}

	void build_netki_project(project *proj)
	{
		std::string out_base(proj->start_path);
		out_base.append("/_gen");
		std::string netki_base(out_base + "/netki");

		std::stringstream netki_master;

		for (int i=0;i!=proj->files.size();i++)
		{
			parsed_file *pf = &proj->files[i];
			std::string subpath = pf->sourcepath.substr(proj->base_path.size()+1);
			std::string rt_path = netki_base + "/" + subpath;

			{
				std::stringstream hdr;
				indentedwriter hw(hdr);
				hw.cont() << "// Netki generated header";
				hw.line();
				hw.line() << "namespace netki";
				hw.line() << "{";
				hw.indent(1);
				hw.line() << "namespace bitstream { struct buffer; }";

				for (int j=0;j!=pf->structs.size();j++)
				{
					parsed_struct *s = &pf->structs[j];
					if (is_netki_struct(s))
					{
						write_struct_header(s, hw);
					}
				}
				hw.indent(-1);
				hw.line() << "}";
				save_stream(rt_path + ".h", hdr);
			}

			{
				std::stringstream impl;
				indentedwriter iw(impl);
				iw.cont() << "// Netki generated implementation";
				iw.line();
				iw.line() << "#include \"" << pf->filename << ".h\"";
				iw.line() << "#include <netki/bitstream.h>";
				iw.line() << "#include <cstring>";
				iw.line();
				iw.line() << "namespace netki";
				iw.line() << "{";
				iw.indent(1);

				for (int j=0;j!=pf->structs.size();j++)
				{
					parsed_struct *s = &pf->structs[j];
					if (is_netki_struct(s))
					{
						write_struct_impl(s, iw);
					}
				}
				iw.indent(-1);
				iw.line() << "}";
				save_stream(rt_path + ".cpp", impl);

				netki_master << "#include \"netki/" << subpath << ".cpp\"\n";
			}
		}

		putki::save_stream(out_base + "/" + proj->module_name + "-netki-runtime-master.cpp", netki_master);
	}

}
