#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{

	std::string cs_type_name(putki::parsed_field *f)
	{
		switch (f->type)
		{
			case FIELDTYPE_INT32:
				return "int";
			case FIELDTYPE_BYTE:
				return "byte";
			case FIELDTYPE_FLOAT:
				return "float";
			case FIELDTYPE_BOOL:
				return "bool";
			case FIELDTYPE_POINTER:
			case FIELDTYPE_STRUCT_INSTANCE:
			case FIELDTYPE_ENUM:
				return f->ref_type;
			case FIELDTYPE_FILE:
			case FIELDTYPE_STRING:
				return "string";
			default:
				return "nada";
		}		
	}

	std::string cs_type_size_expr(putki::parsed_field *f)
	{
		switch (f->type)
		{
			case FIELDTYPE_INT32:
				return "4";
			case FIELDTYPE_BYTE:
				return "1";
			case FIELDTYPE_POINTER:
				return "4";
			case FIELDTYPE_BOOL:
				return "1";
			case FIELDTYPE_STRUCT_INSTANCE:
				return f->ref_type + ".SIZE";
			case FIELDTYPE_FILE:
			case FIELDTYPE_STRING:
				return "4";
			default:
				return "1000000";
		}		
	}

	void write_csharp_runtime_class(putki::parsed_file *file, putki::indentedwriter out, putki::indentedwriter switch_case_out, putki::indentedwriter switch_case_resolve)
	{
		out.line() << "// Generated by Putki compiler";
		out.line();
		out.line() << "namespace outki";
		out.line() << "{";
		out.indent(1);

		out.line() << "// Enums";

		for (size_t i=0;i<file->enums.size();i++)
		{
			putki::parsed_enum *e = &file->enums[i];
			out.line() << "public enum " << e->name;
			out.line() << "{";
			for (size_t j=0;j<e->values.size();j++)
			{				
				if (j > 0)
					out.cont() << ",";
				out.line(1) << e->values[j].name << " = " << e->values[j].value;
			}
			out.line() << "};";
		}
				
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			std::string expr_size_add = "";
			
			out.line() << "public class " << s->name;
			
			// c# gets real parenting since we can't fool around with pointer casts.
			if (!s->parent.empty())
			{
				out.cont() << " : " << s->parent;
				expr_size_add.append(" + " + s->parent + ".SIZE");
			}

			out.line() << "{";

			
			out.indent(1);

			int size = 0;

			out.line() << "// Fields";

			for (size_t i=0;i<s->fields.size();i++)
			{
				putki::parsed_field *f = &s->fields[i];
				if (!(f->domains & putki::DOMAIN_RUNTIME))
					continue;
				if (!strcmp(f->name.c_str(), "parent"))
					continue;

				if (s->fields[i].is_array)
				{
					size += 8; // ptr & count
				}
				else
				{
					switch (s->fields[i].type)
					{
					case FIELDTYPE_INT32:
					case FIELDTYPE_FLOAT:
					case FIELDTYPE_ENUM:
						size += 4;
						break;
					case FIELDTYPE_BYTE:
						size++;
						break;
					case FIELDTYPE_POINTER:
						size += 4;
						break;					
					case FIELDTYPE_BOOL:
						size += 1;
						break;
					case FIELDTYPE_STRUCT_INSTANCE:
						size += 0;
						expr_size_add = " + " + s->fields[i].ref_type + ".SIZE";
						break;
					case FIELDTYPE_FILE:
					case FIELDTYPE_STRING:
						size += 4;
						break;
					}
				}

				out.line() << "public " << cs_type_name(&s->fields[i]);

				if (f->is_array)
					out.cont() << "[]";

				out.cont() << " " << f->name << ";";

				if (s->fields[i].type == FIELDTYPE_POINTER)
				{
					if (f->is_array)
						out.line() << "int[] _ptr_slot_" << s->fields[i].name << "; // temp for load/resolve step";
					else
						out.line() << "int _ptr_slot_" << s->fields[i].name << "; // temp for load/resolve step";
				}

				out.line();
				
			}

			out.line() << "// Generated constants";

			std::string new_hide_str = "";
			if (!s->parent.empty())
				new_hide_str = "new ";

			out.line() << new_hide_str << "public const int SIZE = " << size << expr_size_add <<";";

			out.line();
			out.line() << "// Generated functions";

			// Load from package
			out.line() << new_hide_str << "public static " << s->name << " LoadFromPackage(Putki.PackageReader reader, Putki.PackageReader aux)";
			out.line() << "{";
			out.line(1) << s->name << " tmp = new " << s->name << "();";
			out.line(1) << "tmp.ParseFromPackage_" << s->name << "(reader, aux);";
			out.line(1) << "return tmp;";
			out.line() << "}";
			
			out.line();
			out.line() << "public void ParseFromPackage_" << s->name << "(Putki.PackageReader reader, Putki.PackageReader aux)";
			out.line() << "{";
			out.indent(1);
			
			if (!s->parent.empty())
			{
				out.line() << "ParseFromPackage_" << s->parent << "(reader, aux);";
			}

			out.line() << "// Load from byte buffer";
			for (size_t i=0;i<s->fields.size();i++)
			{
				putki::parsed_field *f = &s->fields[i];
				if (!(f->domains & putki::DOMAIN_RUNTIME))
					continue;
				if (!strcmp(f->name.c_str(), "parent"))
					continue;
				
				std::string field_ref = f->name;
				std::string ptr_slot_ref = std::string("_ptr_slot_") + f->name;
				std::string content_reader = "reader";

				if (s->fields[i].is_array)
				{
					out.line() << "// array reader";
					out.line() << "{";
					out.indent(1);
					out.line() << "reader.ReadInt32(); // where the pointer would go in c++";
					out.line() << "int count = reader.ReadInt32();";
					if (s->fields[i].type == FIELDTYPE_POINTER)
						out.line() << ptr_slot_ref << " = new int[count];";

					out.line() << "Putki.PackageReader arrAux = aux.CloneAux(0);";
					out.line() << "aux.Skip(count * " << cs_type_size_expr(&s->fields[i]) << ");";
					out.line() << field_ref << " = new " << cs_type_name(&s->fields[i]) << "[count];";
					out.line() << "for (int i=0;i<count;i++)";
					out.line() << "{";

					content_reader = "arrAux";
					field_ref = field_ref + "[i]";
					ptr_slot_ref = ptr_slot_ref + "[i]";
					out.indent(1);
				}

				switch (s->fields[i].type)
				{
					case FIELDTYPE_INT32:
						out.line() << field_ref <<  " = " << content_reader << ".ReadInt32();";
						break;
					case FIELDTYPE_BYTE:
						out.line() << field_ref << " = " << content_reader << ".ReadByte();";
						break;
					case FIELDTYPE_BOOL:
						out.line() << field_ref << " = " << content_reader << ".ReadByte() != 0;";
						break;
					case FIELDTYPE_FLOAT:
						out.line() << field_ref << " = " << content_reader << ".ReadFloat();";
						break;
					case FIELDTYPE_ENUM:
						out.line() << field_ref << " = (" << s->fields[i].ref_type << ")" << content_reader << ".ReadInt32();";
						break;
					case FIELDTYPE_POINTER:
						out.line() << ptr_slot_ref << " = " << content_reader << ".ReadInt32();";
						//out.cont() << f->ref_type;
						break;
					case FIELDTYPE_STRUCT_INSTANCE:
						out.line() << field_ref << " = " << s->fields[i].ref_type << ".LoadFromPackage(" << content_reader << ", aux);";
						break;
					case FIELDTYPE_FILE:
					case FIELDTYPE_STRING:
						{
							out.line() << field_ref << " = aux.ReadString(" << content_reader << ".ReadInt32());";
						}
						break;
				}

				if (s->fields[i].is_array)
				{
					out.indent(-1);
					out.line() << "} // loop";
					out.indent(-1);
					out.line() << "} // blah";
				}
			}

			out.indent(-1);
			out.line() << "}";

			// Resolve pointers
			out.line();
			out.line() << new_hide_str << "public void ResolveFromPackage(Putki.Package pkg)";
			out.line() << "{";
			out.indent(1);
			
			out.line() << "// Load from byte buffer";
			for (size_t i=0;i<s->fields.size();i++)
			{
				putki::parsed_field *f = &s->fields[i];
				if (!(f->domains & putki::DOMAIN_RUNTIME))
					continue;
				if (s->fields[i].type != FIELDTYPE_POINTER &&  s->fields[i].type != FIELDTYPE_STRUCT_INSTANCE)
					continue;
				if (!strcmp(f->name.c_str(), "parent"))
					continue;

				std::string field_ref = f->name;
				std::string ptr_slot_ref = std::string("_ptr_slot_") + f->name;

				if (s->fields[i].is_array)
				{
					out.line() << "// array reader";
					out.line() << "for (int i=0;i<" << field_ref << ".Length;i++)";
					out.line() << "{";
					
					field_ref = field_ref + "[i]";
					ptr_slot_ref = ptr_slot_ref + "[i]";
					out.indent(1);
				}

				switch (s->fields[i].type)
				{
					case FIELDTYPE_POINTER:
						out.line() << field_ref << " = (" << s->fields[i].ref_type << ") pkg.ResolveSlot(" << ptr_slot_ref << ");";
						break;
					case FIELDTYPE_STRUCT_INSTANCE:
						out.line() << field_ref << ".ResolveFromPackage(pkg);";
						break;
					default:
						break;
				}

				if (s->fields[i].is_array)
				{
					out.indent(-1);
					out.line() << "} // loop";
				}
			}
			
			out.indent(-1);
			out.line() << "}";

			// class
			out.indent(-1);
			out.line() << "}";

			switch_case_out.line() << "case " << s->unique_id << ":";
			switch_case_out.line() << "{";
			switch_case_out.line() << "	Putki.PackageReader aux = reader.CloneAux(" << s->name << ".SIZE);";
			switch_case_out.line() << "	object o = " << s->name << ".LoadFromPackage(reader, aux);";
			switch_case_out.line() << "	reader.MoveTo(aux);";
			switch_case_out.line() << "	return o;";
			switch_case_out.line() << "}";

			switch_case_resolve.line() << "case " << s->unique_id << ": ((" << s->name << ")obj).ResolveFromPackage(pkg); break;";

			out.line();
		}

		out.indent(-1);
		out.line() << "} // namespace outki";
	}

}
