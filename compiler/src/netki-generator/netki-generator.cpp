#include "netki-generator.h"
#include "inki-outki-generator/generator.h"
#include "writetools/save_stream.h"
#include "writetools/indentedwriter.h"

#include <string>
#include <sstream>
#include <cstring>

namespace putki
{
	bool is_netki_struct(parsed_struct *s)
	{
		for (int i=0; i!=s->targets.size(); i++)
			if (!strcmp(s->targets[i].c_str(), "netki"))
				return true;
		return false;
	}

	std::string field_type(parsed_field *field)
	{
		std::string type = "err";
		switch (field->type)
		{
			case FIELDTYPE_BOOL:
				type = "bool";
				break;
			case FIELDTYPE_INT32:
				type = "int32_t";
				break;
			case FIELDTYPE_UINT32:
				type = "uint32_t";
				break;
			case FIELDTYPE_BYTE:
				type = "uint8_t";
				break;
			case FIELDTYPE_STRING:
				type = "const char*";
				break;
			case FIELDTYPE_FLOAT:
				type = "float";
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
		return type;
	}

	std::string field_type_csharp(parsed_field *field)
	{
		std::string type = "err";
		switch (field->type)
		{
			case FIELDTYPE_BOOL:
				type = "bool";
				break;
			case FIELDTYPE_INT32:
				type = "int";
				break;
			case FIELDTYPE_UINT32:
				type = "uint";
				break;
			case FIELDTYPE_BYTE:
				type = "byte";
				break;
			case FIELDTYPE_FLOAT:
				type = "float";
				break;
			case FIELDTYPE_STRING:
				type = "string";
				break;
			case FIELDTYPE_POINTER:
				type = field->ref_type;
				break;
			case FIELDTYPE_ENUM:
			case FIELDTYPE_STRUCT_INSTANCE:
				type = field->ref_type;
				break;
			default:
				type = "unknown!";
				break;
		}
		return type;
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

		for (int i=0; i!=s->fields.size(); i++)
		{
			parsed_field *field = &s->fields[i];

			std::string type = field_type(field);
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
		wr.line() << "static bool parse_from_bitstream(" << s->name << " *target, netki::bitstream::buffer *source, netki::bitstream::buffer *allocbuf);";
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

		for (int i=0; i!=s->fields.size(); i++)
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
					wr.line() << "bitstream::insert_compressed_int(dest, " << field_ref << ");";
					break;
				case FIELDTYPE_UINT32:
					wr.line() << "bitstream::insert_compressed_uint(dest, " << field_ref << ");";
					break;
				case FIELDTYPE_BYTE:
					wr.line() << "bitstream::insert_bits<8>(dest, " << field_ref << ");";
					break;
				case FIELDTYPE_FLOAT:
					wr.line() << "// FIXME";
					break;
				case FIELDTYPE_STRING:
					wr.line() << "if (" << field_ref << ")";
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "bitstream::insert_bits<16>(dest, strlen(" << field_ref << "));";
					wr.line() << "bitstream::insert_bytes(dest, (uint8_t*) " << field_ref << ", strlen(" << field_ref << "));";
					wr.indent(-1);
					wr.line() << "}";
					wr.line() << "else";
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "bitstream::insert_bits<16>(dest, 0xffff);";
					wr.indent(-1);
					wr.line() << "}";
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


		wr.line() << "return (dest->error == 0);";
		wr.indent(-1);
		wr.line() << "}";
		wr.line();
		wr.line() << "bool " << s->name << "::parse_from_bitstream(" << s->name << " *dest, netki::bitstream::buffer *source, netki::bitstream::buffer *unpackbuf)";
		wr.line() << "{";
		wr.indent(1);

		// parsers
		for (int i=0; i!=s->fields.size(); i++)
		{
			parsed_field *field = &s->fields[i];

			std::string field_ref = std::string("dest->") + field->name;
			std::string type = field_type(field);

			if (field->is_array)
			{
				wr.line() << "{";
				wr.indent(1);
				wr.line() << field_ref << "_size = bitstream::read_bits<32>(source);";
				wr.line() << field_ref << " = (" << type << " *)bitstream::alloc(unpackbuf, sizeof(" << type << ") * " << field_ref << "_size);";
				wr.line() << "if (!" << field_ref << ")";
				wr.line(1) << "return false;";
				wr.line() << "for (int i=0;i!=" << field_ref << "_size;i++)";
				wr.line() << "{";
				wr.indent(1);
				field_ref.append("[i]");
			}

			switch (field->type)
			{
				case FIELDTYPE_BOOL:
					wr.line() << field_ref << " = bitstream::read_bits<1>(source);";
					break;
				case FIELDTYPE_UINT32:
					wr.line() << field_ref << " = bitstream::read_compressed_uint(source);";
					break;
				case FIELDTYPE_INT32:
					wr.line() << field_ref << " = bitstream::read_compressed_int(source);";
					break;
				case FIELDTYPE_BYTE:
					wr.line() << field_ref << " = bitstream::read_bits<8>(source);";
					break;
				case FIELDTYPE_FLOAT:
					wr.line() << field_ref << " = 12345678.0;";
					break;
				case FIELDTYPE_STRING:
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "uint32_t len = (uint32_t) bitstream::read_bits<16>(source);";
					wr.line() << "if (len == 0xffff)";
					wr.line() << "{";
					wr.line(1) << field_ref << " = 0;";
					wr.line() << "}";
					wr.line() << "else";
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "uint8_t *buf = (uint8_t*) bitstream::alloc(unpackbuf, len + 1);";
					wr.line() << field_ref << " = (const char*) buf;";
					wr.line() << "if (!" << field_ref << ")";
					wr.line(1) << "return false;";
					wr.line() << "bitstream::read_bytes(source, buf, len);";
					wr.line() << "buf[len] = 0;";
					wr.indent(-1);
					wr.line() << "}";
					wr.indent(-1);
					wr.line() << "}";
					break;
				case FIELDTYPE_ENUM:
					wr.line() << field_ref << " = (" << type << ") = bitstream::read_bits<32>(source);";
					break;
				case FIELDTYPE_STRUCT_INSTANCE:
					wr.line() << "if (!" << field->ref_type << "::parse_from_bitstream(&" << field_ref << ", source, unpackbuf))";
					wr.line(1) << "return false;";
					break;
				default:
					wr.line() << "<compile error trying to write field " << s->name << ">";
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

		wr.line() << "return (source->error == 0);";
		wr.indent(-1);
		wr.line() << "}";
		wr.line();
	}

	void write_struct_csharp(parsed_struct *s, indentedwriter &wr, indentedwriter &sp, indentedwriter &enc)
	{
		wr.line();
		wr.line() << "// Generated from struct '" << s->name << "'";
		wr.line() << "public class " << s->name << " : Packet";
		wr.line() << "{";
		wr.indent(1);
		wr.line() << "public const int TYPE_ID = " << s->unique_id << ";";
		wr.line();
		wr.line() << "public " << s->name << "() : base(TYPE_ID)";
		wr.line() << "{";
		wr.line() << "}";
		wr.line();

		for (int i=0; i!=s->fields.size(); i++)
		{
			parsed_field *field = &s->fields[i];

			std::string type = field_type_csharp(field);
			std::string defval = field->def_value;
			if (!defval.empty())
				defval = " = " + defval;

			if (field->is_array)
			{
				wr.line() << "public " << type << "[] " << field->name << ";";
			}
			else
			{
				wr.line() << "public " << type << " " << field->name << defval << ";";
			}
		}

		wr.line();
		wr.line() << "public static bool WriteIntoBitstream(Bitstream.Buffer buf, " << s->name << " obj)";
		wr.line() << "{";
		wr.indent(1);

		for (int i=0; i!=s->fields.size(); i++)
		{
			parsed_field *field = &s->fields[i];

			std::string field_ref = std::string("obj.") + field->name;
			
			if (field->is_array)
			{
				wr.line() << "{";
				wr.indent(1);
				wr.line() << "int length = -1;";
				wr.line() << "if (" << field_ref << " != null)";
				wr.line(1) << "length = " << field_ref << ".Length;";
				wr.line() << "Bitstream.PutCompressedInt(buf, length);";
				if (field->type == FIELDTYPE_BYTE)
				{
					wr.line() << "if (" << field_ref << " != null)";
					wr.line(1) << "Bitstream.PutBytes(buf, " << field_ref << ");";
					wr.indent(-1);
					wr.line() << "}";
					continue;
				}
				wr.line() << "for (int i=0;i!=length && length > 0;i++)";
				wr.line() << "{";
				wr.indent(1);
				field_ref.append("[i]");
			}

			switch (field->type)
			{
				case FIELDTYPE_BOOL:
					wr.line() << "Bitstream.PutBits(buf, 1, " << field_ref << " ? (uint)1 : (uint)0);";
					break;
				case FIELDTYPE_INT32:
					wr.line() << "Bitstream.PutCompressedInt(buf, " << field_ref << ");";
					break;
				case FIELDTYPE_UINT32:
					wr.line() << "Bitstream.PutCompressedUint(buf, " << field_ref << ");";
					break;
				case FIELDTYPE_BYTE:
					wr.line() << "Bitstream.PutBits(buf, 8, " << field_ref << ");";
					break;
				case FIELDTYPE_STRING:
					wr.line() << "if (" << field_ref << " != null)";
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "byte[] data = System.Text.Encoding.UTF8.GetBytes(" << field_ref << ");";
					wr.line() << "Bitstream.PutBits(buf, 16, (uint)data.Length);";
					wr.line() << "Bitstream.PutBytes(buf, data);";
					wr.indent(-1);
					wr.line() << "}";
					wr.line() << "else";
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "Bitstream.PutBits(buf, 16, 0xffff);";
					wr.indent(-1);
					wr.line() << "}";
					break;
				case FIELDTYPE_FLOAT:
					wr.line() << "// fix me";
					break;
				case FIELDTYPE_ENUM:
					wr.line() << "Bitstream.PutBits(buf, 32, (int)" << field_ref << ");";
					break;
				case FIELDTYPE_STRUCT_INSTANCE:
					wr.line() << field->ref_type << ".WriteIntoBitstream(buf, " << field_ref << ");";
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

		wr.line() << "return (buf.error == 0);";
		wr.indent(-1);
		wr.line() << "}";
		wr.line();
		wr.line() << "public static bool ReadFromBitstream(Bitstream.Buffer buf, " << s->name << " into)";
		wr.line() << "{";
		wr.indent(1);

		// parsers
		for (int i=0; i!=s->fields.size(); i++)
		{
			parsed_field *field = &s->fields[i];

			std::string field_ref = std::string("into.") + field->name;
			std::string type = field_type_csharp(field);

			if (field->is_array)
			{
				wr.line() << "{";
				wr.indent(1);
				wr.line() << "int size = Bitstream.ReadCompressedInt(buf);";
				wr.line() << "if (size < 0)";
				wr.line() << "{";
				wr.line(1) << field_ref << " = null;";
				wr.line() << "}";
				wr.line() << "else";
				wr.line() << "{";
				wr.indent(1);
				
				// minimum sizes to prevent allocation too much and also early exit
				int size;
				switch (field->type)
				{
					case FIELDTYPE_BOOL: size = 1; break;
					case FIELDTYPE_STRING: size = 16; break;
					case FIELDTYPE_UINT32:
					case FIELDTYPE_INT32: size = 32; break;
					default: size = 8; break;
				}
				wr.line() << "if (buf.BitsLeft() < " << size << " * size)";
				wr.line() << "{";
				wr.line(1) << "buf.error = 2;";
				wr.line(1) << "return false;";
				wr.line() << "};";
				
				if (field->type == FIELDTYPE_BYTE)
				{
					wr.line() << field_ref << " = Bitstream.ReadBytes(buf, (int)size);";
					wr.line() << "if (" << field_ref << " == null) return false;";
					wr.indent(-1);
					wr.line() << "}";
					wr.indent(-1);
					wr.line() << "}";
					continue;
				}
				
				wr.line() << field_ref << " = new " << type << "[size];";
				wr.line() << "for (int i=0;i!=size;i++)";
				wr.line() << "{";
				wr.indent(1);
				field_ref.append("[i]");
			}

			switch (field->type)
			{
				case FIELDTYPE_BOOL:
					wr.line() << field_ref << " = (Bitstream.ReadBits(buf, 1) != 0);";
					break;
				case FIELDTYPE_INT32:
					wr.line() << field_ref << " = Bitstream.ReadCompressedInt(buf);";
					break;
				case FIELDTYPE_UINT32:
					wr.line() << field_ref << " = Bitstream.ReadCompressedUint(buf);";
					break;
				case FIELDTYPE_BYTE:
					wr.line() << field_ref << " = (byte)Bitstream.ReadBits(buf, 8);";
					break;
				case FIELDTYPE_STRING:
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "int length = (int)Bitstream.ReadBits(buf, 16);";
					wr.line() << "if (length == 0xffff)";
					wr.line() << "{";
					wr.line(1) << field_ref << " = null;";
					wr.line() << "}";
					wr.line() << "else";
					wr.line() << "{";
					wr.indent(1);
					wr.line() << "byte[] strData = Bitstream.ReadBytes(buf, length);";
					wr.line() << "if (strData == null) { buf.error = 1; return false; }";
					wr.line() << field_ref << " = System.Text.Encoding.UTF8.GetString(strData);";
					wr.indent(-1);
					wr.line() << "}";
					wr.indent(-1);
					wr.line() << "}";
					break;
				case FIELDTYPE_FLOAT:
					wr.line() << field_ref << " = 1234567.0f;";
					break;
				case FIELDTYPE_ENUM:
					wr.line() << field_ref << " = (" << type << ") = Bitstream.ReadBits(buf, 32);";
					break;
				case FIELDTYPE_STRUCT_INSTANCE:
					wr.line() << field_ref << " = new " << type << "();";
					wr.line() << "if (!" << field->ref_type << ".ReadFromBitstream(buf, " << field_ref << "))";
					wr.line(1) << "return false;";
					break;
				default:
					wr.line() << "<compile error trying to write field " << s->name << ">";
					break;
			}

			if (field->is_array)
			{
				wr.indent(-1);
				wr.line() << "}";
				wr.indent(-1);
				wr.line() << "}";
				wr.indent(-1);
				wr.line() << "}";
			}
		}

		wr.line() << "return (buf.error == 0);";
		wr.indent(-1);
		wr.line() << "}";
		
		wr.indent(-1);
		wr.line() << "}";
		
		// decode
		sp.line(4) << "case " << s->name << ".TYPE_ID:";
		sp.line(5) << "{";
		sp.line(6) << s->name << " dst = new " << s->name << "();";
		sp.line(6) << "if (" << s->name << ".ReadFromBitstream(bs, dst))";
		sp.line(6) << "{";
		sp.line(7) << "pkt.packet = dst;";
		sp.line(7) << "pkt.type_id = " << s->name << ".TYPE_ID;";
		sp.line(7) << "return true;";
		sp.line(6) << "}";
		sp.line(6) << "return false;";
		sp.line(5) << "}";

		// encode
		enc.line(4) << "case " << s->name << ".TYPE_ID:";
		enc.line(5) << s->name << ".WriteIntoBitstream(buffer, (" << s->name << ") packet);";
		enc.line(5) << "return;";
	}

	void build_netki_project_c(project *proj)
	{
		std::string out_base(proj->start_path);
		out_base.append("/_gen");
		std::string netki_base(out_base + "/netki");

		std::stringstream netki_master;

		bool write_master = false;

		for (int i=0; i!=proj->files.size(); i++)
		{
			parsed_file *pf = &proj->files[i];
			std::string subpath = pf->sourcepath.substr(proj->base_path.size()+1);
			std::string rt_path = netki_base + "/" + subpath;
			bool contained_anything = false;

			{
				std::stringstream hdr;
				indentedwriter hw(hdr);
				hw.cont() << "// Netki generated header";
				hw.line() << "#include <cstdint>";
				hw.line();
				hw.line() << "namespace netki";
				hw.line() << "{";
				hw.indent(1);
				hw.line() << "namespace bitstream { struct buffer; }";

				for (int j=0; j!=pf->structs.size(); j++)
				{
					parsed_struct *s = &pf->structs[j];
					if (is_netki_struct(s))
					{
						write_struct_header(s, hw);
						contained_anything = true;
						write_master = true;
					}
				}
				hw.indent(-1);
				hw.line() << "}";
				save_stream(rt_path + ".h", hdr);
			}

			if (contained_anything)
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

				for (int j=0; j!=pf->structs.size(); j++)
				{
					parsed_struct *s = &pf->structs[j];
					if (is_netki_struct(s))
						write_struct_impl(s, iw);
				}
				iw.indent(-1);
				iw.line() << "}";
				save_stream(rt_path + ".cpp", impl);

				netki_master << "#include \"netki/" << subpath << ".cpp\"\n";
			}
		}

		if (write_master)
			putki::save_stream(out_base + "/" + proj->module_name + "-netki-runtime-master.cpp", netki_master);
	}

	void build_netki_project_csharp(project *proj)
	{
		std::string out_base(proj->start_path);
		out_base.append("/_gen");
		std::string netki_base(out_base + "/netki_csharp");

		std::stringstream netki_master;
		indentedwriter hw(netki_master);
		
		std::stringstream netki_switch_parse;
		indentedwriter sp(netki_switch_parse);
		std::stringstream netki_switch_encode;
		indentedwriter enc(netki_switch_encode);
		
		hw.cont() << "// Netki generated code";
		hw.line() << "namespace netki";
		hw.line() << "{";
		hw.indent(1);
		
		bool write_master = false;

		for (int i=0; i!=proj->files.size(); i++)
		{
			parsed_file *pf = &proj->files[i];
			write_csharp_enum(pf, hw);
			
			for (int j=0; j!=pf->structs.size(); j++)
			{
				parsed_struct *s = &pf->structs[j];
				if (is_netki_struct(s))
				{
					write_struct_csharp(s, hw, sp, enc);
					write_master = true;
				}
			}
		}
		
		hw.indent(-1);
		hw.line();
		hw.line(1) << "public static class " << proj->loader_name << "Packets";
		hw.line(1) << "{";
		hw.line(2) << "public static bool Decode(Bitstream.Buffer bs, int type_id, out DecodedPacket pkt)";
		hw.line(2) << "{";
		hw.line(3) << "pkt.type_id = -1;";
		hw.line(3) << "pkt.packet = null;";
		hw.line(3) << "switch (type_id) {";
		hw.line() << netki_switch_parse.str();
		hw.line(3) << "}";
		hw.line(3) << "return false;";
		hw.line(2) << "}";
		hw.line(2) << "public static void Encode(Packet packet, Bitstream.Buffer buffer)";
		hw.line(2) << "{";
		hw.line(3) << "switch (packet.type_id) {";
		hw.line() << netki_switch_encode.str();
		hw.line(3) << "}";
		hw.line(2) << "}";
		hw.line(1) << "}";
		hw.line(0) << "}"; // namespace 

		if (write_master)
			putki::save_stream(out_base + "/" + proj->module_name + "-netki.cs", netki_master);
	}
	
	void build_netki_project(project *proj)
	{
		build_netki_project_c(proj);
		build_netki_project_csharp(proj);
	}
}
