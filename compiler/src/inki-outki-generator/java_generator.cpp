#include "generator.h"

#include <putki/domains.h>
#include <iostream>
#include <cstring>

namespace putki
{

	std::string java_type_name(putki::parsed_field *f)
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
			case FIELDTYPE_PATH:
				return "String";
			default:
				return "nada";
		}
	}

	std::string java_type_size_expr(putki::parsed_field *f)
	{
		switch (f->type)
		{
			case FIELDTYPE_FLOAT:
			case FIELDTYPE_ENUM:
			case FIELDTYPE_INT32:
				return "4";
			case FIELDTYPE_BYTE:
				return "1";
			case FIELDTYPE_POINTER:
				return "2";
			case FIELDTYPE_BOOL:
				return "1";
			case FIELDTYPE_STRUCT_INSTANCE:
				return f->ref_type + ".SIZE";
			case FIELDTYPE_FILE:
			case FIELDTYPE_STRING:
			case FIELDTYPE_PATH:
				return "2";
			default:
				return "1000000";
		}
	}
	
	void write_java_enum(putki::parsed_file *file, putki::indentedwriter out)
	{
		out.line() << "// Enums";
		for (size_t i=0; i<file->enums.size(); i++)
		{
			putki::parsed_enum *e = &file->enums[i];
			out.line() << "public enum " << e->name;
			out.line() << "{";
			for (size_t j=0; j<e->values.size(); j++)
			{
				if (j > 0)
					out.cont() << ",";
				out.line(1) << e->values[j].name << "(" << e->values[j].value << ")";
			}
			out.line() << "};";
		}
	}

	void write_java_inki_class(putki::parsed_file *file, putki::indentedwriter out)
	{
		for (size_t i=0; i<file->enums.size(); i++)
		{
			putki::parsed_enum *e = &file->enums[i];
			out.line() << "public enum " << e->name;
			out.line() << "{";
			for (size_t j=0; j<e->values.size(); j++)
			{
				if (j > 0)
					out.cont() << ",";
				out.line(1) << e->values[j].name << "(" << e->values[j].value << ")";
			}
			out.cont() << ";";
			out.line(1) << "private int v;";
			out.line(1) << "private " << e->name << "(int value) { v = value; };";
			out.line(1) << "public int getValue() { return v; }";
			out.line() << "};";
		}
		
		for (size_t i=0;i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];

			std::string expr_size_add = "";

			out.line() << "public static class " << s->name;

			if (!s->parent.empty())
				out.cont() << " extends " << s->parent;
			else
				out.cont() << " implements putked.ProxyObject";

			const char *newifparent = s->parent.empty() ? "" : "";

			out.line() << "{";
			out.line(1) << newifparent << "public Interop.MemInstance m_mi;";
			out.line();

			out.indent(1);


			if (!s->parent.empty())
				out.line() << "@Override";
				
			out.line() << "public void connect(Interop.MemInstance mi)";
			out.line() << "{";
			if (!s->parent.empty())
				out.line(1) << "super.connect(get" << s->name << "Parent(mi).m_mi);";
			out.line(1) << "// Should really check here!";
			out.line(1) << "m_mi = mi;";
			out.line() << "}";
			out.line();

			for (size_t i=0; i<s->fields.size(); i++)
			{
				putki::parsed_field *f = &s->fields[i];
				if (!(f->domains & putki::DOMAIN_INPUT))
					continue;

				const int dllindex = s->fields[i]._WROTE_DLL_FIELD_INDEX;
				if (dllindex == -1)
				{
					out.line();
					out.line() << "// !!! field " << s->fields[i].name << " was not written to dll handler";
					out.line();
					continue;
				}

				const char *args = "()";
				const char *args_set0 = "";

				////////////////////
				// First we do get.


				const char firstletter = s->fields[i].name[0];

				const char *getpfx = "get";
				const char *setpfx = "set";
				const char *resolvepfx = "Resolve";
				const char *sizepostfx = "Size";
				const char *pushpostfx = "PushBack";
				const char *erasepostfx = "Erase";

				if (firstletter >= 'a' && firstletter < 'z')
				{
					getpfx = "get_";
					setpfx = "set_";
					resolvepfx = "resolve_";
					sizepostfx = "_size";
					pushpostfx = "_push_back";
					erasepostfx = "_erase";
				}

				if (s->fields[i].is_array)
				{
					args = "(int arrayIndex)";
					args_set0 = "int arrayIndex, ";
					out.line() << "public int " << getpfx << s->fields[i].name << sizepostfx << "() { return m_mi.getField(" << dllindex << ").getArraySize(m_mi); }";
					out.line();

					out.line() << "public void " << s->fields[i].name << pushpostfx << "() { m_mi.getField(" << dllindex << ").arrayInsert(m_mi); }";
					out.line();
					out.line() << "public void " << s->fields[i].name << erasepostfx << "(int index) { m_mi.getField(" << dllindex << ").setArrayIndex(index); m_mi.getField(" << dllindex << ").arrayErase(m_mi); }";
					out.line();
				}


				std::string get_name = std::string(getpfx) + s->fields[i].name;
				std::string set_name = std::string(setpfx) + s->fields[i].name;

				if (get_name == "get__rtti_type")
					get_name = "get_rtti_type";

				if (get_name == "get_parent")
				{
					// SUPER DELUXE HACK TO WRITE STATIC MEMBER
					get_name = std::string("get") + s->name + "Parent";
					out.line() << "static ";
					args = "(Interop.MemInstance m_mi)";
				}

				switch (s->fields[i].type)
				{
					case FIELDTYPE_INT32:
						out.line() << "public int " << get_name << args;
						break;
					case FIELDTYPE_STRING:
					case FIELDTYPE_FILE:
					case FIELDTYPE_PATH:
						out.line() << "public String " <<get_name << args;
						break;
					case FIELDTYPE_POINTER:
						out.line() << "public String " <<get_name << args;
						break;
					case FIELDTYPE_STRUCT_INSTANCE:
						out.line() << "public " << s->fields[i].ref_type << " " << get_name << args;
						break;
					case FIELDTYPE_FLOAT:
						out.line() << "public float " << get_name << args;
						break;
					default:
						out.line() << "public void  " << get_name << args;
						break;
				}

				out.line() << "{";
				out.indent(1);

				if (s->fields[i].is_array)
					out.line() << "m_mi.getField(" << dllindex << ").setArrayIndex(arrayIndex);";

				switch (s->fields[i].type)
				{
					case FIELDTYPE_INT32:
						out.line() << "return m_mi.getField(" << dllindex << ").getInt32(m_mi);";
						break;
					case FIELDTYPE_STRING:
					case FIELDTYPE_FILE:
					case FIELDTYPE_POINTER:
					case FIELDTYPE_PATH:
						out.line() << "return m_mi.getField(" << dllindex << ").getString(m_mi);";
						break;
					case FIELDTYPE_STRUCT_INSTANCE:
						out.line() << "Interop.MemInstance ml = m_mi.getField(" << dllindex << ").getStructInstance(m_mi);";
						out.line() << "if (ml == null) return null;";
						out.line() << s->fields[i].ref_type << " p = new " << s->fields[i].ref_type << "();";
						out.line() << "p.connect(ml);";
						out.line() << "return p;";
						break;
					case FIELDTYPE_FLOAT:
						out.line() << "return m_mi.getField(" << dllindex << ").getFloat(m_mi);";
						break;
					default:
						out.line() << "// nothing";
						break;
				}

				out.indent(-1);
				out.line() << "}";
				out.line();

				// SET

				switch (s->fields[i].type)
				{
					case FIELDTYPE_INT32:
						out.line() << "public void " << set_name << "(" << args_set0 << "int value)";
						break;
					case FIELDTYPE_STRING:
					case FIELDTYPE_FILE:
					case FIELDTYPE_POINTER:
					case FIELDTYPE_PATH:
						out.line() << "public void " << set_name << "(" << args_set0 << "String value)";
						break;
					case FIELDTYPE_FLOAT:
						out.line() << "public void " << set_name << "(" << args_set0 << "float value)";
						break;
					default:
						out.line() << "public void  " << set_name << "(" << args_set0 << "int dummy)";
						break;
				}

				out.line() << "{";
				out.indent(1);

				if (s->fields[i].is_array)
					out.line() << "m_mi.getField(" << dllindex << ").setArrayIndex(arrayIndex);";

				switch (s->fields[i].type)
				{
					case FIELDTYPE_INT32:
						out.line() << "m_mi.getField(" << dllindex << ").setInt32(m_mi, value);";
						break;
					case FIELDTYPE_STRING:
					case FIELDTYPE_FILE:
					case FIELDTYPE_POINTER:
					case FIELDTYPE_PATH:
						out.line() << "m_mi.getField(" << dllindex << ").setString(m_mi, value);";
						break;
					case FIELDTYPE_FLOAT:
						out.line() << "m_mi.getField(" << dllindex << ").setFloat(m_mi, value);";
						break;
					default:
						out.line() << "// nothing";
						break;
				}

				out.indent(-1);
				out.line() << "}";


				if (s->fields[i].type == FIELDTYPE_POINTER)
				{
					out.line();
					out.line() << "public " << s->fields[i].ref_type << " " << resolvepfx << s->fields[i].name << args;
					out.line() << "{";

					if (s->fields[i].is_array)
						out.line(1) << "m_mi.getField(" << dllindex << ").setArrayIndex(arrayIndex);";

					out.line(1) << "Interop.MemInstance ml = Interop.s_wrap.load(m_mi.getField(" << dllindex << ").getPointer(m_mi));";
					out.line(1) << "return ml != null ? (" << s->fields[i].ref_type << ") DataHelper.createPutkEdObj(ml) : null;";
					out.line() << "}";
				}

				out.line();
			}
			
			
			

			out.line() << "// Generated constants";

			std::string new_hide_str = "";
			/*
			if (!s->parent.empty())
				new_hide_str = "new ";
			*/
			out.line() << new_hide_str << "public final int TYPE = " << s->unique_id << ";";

			out.indent(-1);
			out.line() << "}";

			out.line();
		}
		
	}
	
	void write_java_proxy_creator(putki::parsed_file *file, putki::indentedwriter out)
	{
		for (size_t i=0;i!=file->structs.size(); i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			out.line() << "if (type.equals(\"" << s->name << "\"))";
			out.line(1) << "return new " << s->name << "();";
		}
	}
	
}
