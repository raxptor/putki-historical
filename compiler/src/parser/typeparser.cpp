#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <buildconfigs.h>
#include <parser/typeparser.h>
#include <parser/crc32.h>
#include <putki/domains.h>
#include <putki/sys/compat.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace putki
{
	std::string trim(std::string str)
	{
		while (str.size() && (str[str.size()-1] == ' ' || str[str.size()-1] == '\t'))
			str.erase(str.size()-1, 1);
		while (str.size() && (str[0] == ' ' || str[0] == '\t'))
			str.erase(0,1);
		return str;
	}

	void parse_header(const char *input, putki::parsed_struct *out)
	{
		char *dup = strdup(input);
		char *tok = strtok(dup, " ");

		out->name = "";
		out->domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
		out->is_type_root = false;
		out->permit_as_asset = true;
		out->permit_as_auxptr = true;
		out->targets.push_back("putki");

		bool read_parent = false;

		while (tok)
		{
			if (read_parent)
			{
				out->parent = tok;
				read_parent = false;
			}
			else if (out->name.empty())
			{
				out->name = tok;
			}
			else if (!strcmp(tok, ":"))
			{
				read_parent = true;
			}
			else if (!strcmp(tok, "rtti"))
			{
				out->is_type_root = true;
			}
			else if (!strcmp(tok, "no-asset"))
			{
				out->permit_as_asset = false;
			}
			else if (!strcmp(tok, "no-auxptr"))
			{
				out->permit_as_auxptr = false;
			}
			else if (!strcmp(tok, "no-out"))
			{
				out->domains = putki::DOMAIN_INPUT;
			}
			else if (!strcmp(tok, "non-instantiable"))
			{
				out->permit_as_asset = false;
				out->permit_as_auxptr = false;
			}
			else if (tok[0] == '@')
			{
				if (tok[1] == '-')
					out->targets.clear();
				else
					out->targets.push_back(&tok[1]);
				std::cout << " added target [" << &tok[1] << "]" << std::endl;
			}
			else
			{
				std::cerr << "Invalid struct attribute '" << tok << "'" << std::endl;
			}

			tok = strtok(0, " ");
		}

		free(dup);
	}

	void parse_field(const char *input, putki::parsed_field *out)
	{
		const char *delim = " \t";
		char *dup = strdup(input);
		char *tok = strtok(dup, delim);

		out->type = putki::FIELDTYPE_INT32;
		out->name.clear();
		out->is_array = false;
		out->domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
		out->is_aux_ptr = false;
		out->show_in_editor = true;
		out->def_value = "";
		out->is_build_config = false;

		bool read_type = true;
		bool read_ref_type = false;
		bool read_def_value = false;

		std::string type_unresolved;

		while (tok)
		{
			if (!strcmp(tok, "[no-out]"))
			{
				out->domains = putki::DOMAIN_INPUT;
			}
			else if (!strcmp(tok, "[no-in]"))
			{
				out->domains = putki::DOMAIN_RUNTIME;
			}
			else if (!strcmp(tok, "[hidden]"))
			{
				out->show_in_editor = false;
			}
			else if (!strcmp(tok, "[build-config]"))
			{
				out->is_build_config = true;
				out->domains = putki::DOMAIN_INPUT;
				out->show_in_editor = false;
			}
			else
			{
				if (read_type)
				{
					read_type = false;

					std::string type = tok;
					if (strlen(tok) > 2 && type.substr(type.size() - 2, 2) == "[]")
					{
						type.erase(type.size() - 2, 2);
						out->is_array = true;
					}

					if (!strcmp(type.c_str(), "string")) {
						out->type = putki::FIELDTYPE_STRING;
					}
					else if (!strcmp(type.c_str(), "int") || !strcmp(type.c_str(), "s32")) {
						out->type = putki::FIELDTYPE_INT32;
					}
					else if (!strcmp(type.c_str(), "uint") || !strcmp(type.c_str(), "u32")) {
						out->type = putki::FIELDTYPE_UINT32;
					}
					else if (!strcmp(type.c_str(), "float")) {
						out->type = putki::FIELDTYPE_FLOAT;
					}
					else if (!strcmp(type.c_str(), "bool")) {
						out->type = putki::FIELDTYPE_BOOL;
					}
					else if (!strcmp(type.c_str(), "byte")) {
						out->type = putki::FIELDTYPE_BYTE;
					}
					else if (!strcmp(type.c_str(), "path"))
					{
						out->type = putki::FIELDTYPE_PATH;
						read_ref_type = true;
					}
					else if (!strcmp(type.c_str(), "file"))
					{
						out->type = putki::FIELDTYPE_FILE;
						out->domains = putki::DOMAIN_INPUT;
					}
					else if (!strcmp(type.c_str(), "ptr"))
					{
						out->type = putki::FIELDTYPE_POINTER;
						out->is_aux_ptr = false;
						read_ref_type = true;
					}
					else if (!strcmp(type.c_str(), "enum"))
					{
						out->type = putki::FIELDTYPE_ENUM;
						read_ref_type = true;
					}
					else if (!strcmp(type.c_str(), "auxptr"))
					{
						out->type = putki::FIELDTYPE_POINTER;
						out->is_aux_ptr = true;
						read_ref_type = true;
					}
					else
					{
						out->type = putki::FIELDTYPE_STRUCT_INSTANCE;
						out->ref_type = type;
					}
				}
				else if (read_ref_type)
				{
					out->ref_type = tok;
					read_ref_type = false;
				}
				else if (out->name.empty())
				{
					out->name = tok;
				}
				else if (!strcmp(tok, "="))
				{
					read_def_value = true;
				}
				else
				{
					if (read_def_value)
						out->def_value = tok;
				}
			}

			tok = strtok(0, delim);
		}

		free(dup);
	}
	
	void post_parse_struct(parsed_struct *tmp)
	{
		for (unsigned int i=0;i<tmp->fields.size();i++)
		{
			if (tmp->fields[i].is_build_config)
			{
				unsigned int inserted = 0;
				for (int j=0;;j++)
				{
					const char *name = get_build_config(j);
					if (!name)
					{
						break;
					}
					parsed_field pf = tmp->fields[i];
					pf.name = pf.name + name;
					pf.is_build_config = false;
					pf.show_in_editor = true;
					tmp->fields.insert(tmp->fields.begin()+i+1, pf);
					inserted++;
				}
				i += inserted;
			}	
		}
	}

	bool parse(const char *in_path, const char *name, parsed_file *out)
	{
		std::cout << "Parsing [" << name << "]" << std::endl;
		std::ifstream f(in_path);

		std::string fn(in_path);

		std::string mypath = name;
		int lpp = mypath.find_last_of('/');
		if (lpp != std::string::npos)
			mypath = mypath.substr(0, lpp);

		out->filename = "unknown";
		out->sourcepath = "unknown";

		std::string::size_type lp = fn.find_last_of("/");
		if (lp != std::string::npos)
		{
			std::string::size_type np = fn.find_last_of(".");
			if (np != std::string::npos)
			{
				out->filename = fn.substr(lp + 1, np - lp - 1);
				out->sourcepath = fn.substr(0, np);
			}
		}

		bool in_struct = false;
		bool in_enum = false;
		bool in_scope = false;

		std::string line;

		putki::parsed_struct datastruct;
		putki::parsed_enum enum_;

		std::string everything;

		while (getline(f, line))
		{
			// trim beginning
			while (!line.empty() && isspace(line[0]))
				line.erase(0, 1);

			// erase comments
			int comment = line.find_first_of('/');
			if (comment != std::string::npos && (comment + 1) < (int)line.size() && line[comment+1] == '/')
				line.erase(comment, line.size() - comment);

			if (line.empty())
				continue;

			everything.append(line);

			if (line[0] == '#' || line[0] == '%')
			{
				int space = line.find_first_of(' ');
				if (space != std::string::npos)
				{
					if (line.substr(0, space) == "#include")
						out->includes.push_back(mypath + "/" + line.substr(space + 1, line.size() - space - 1));
					else if (line.substr(0, space) == "%include")
						// cross project full path include.
						out->includes.push_back(line.substr(space + 1, line.size() - space - 1));
				}
				continue;
			}

			if (!in_struct && !in_enum)
			{
				if (!strcmp(line.substr(0, 4).c_str(), "enum"))
				{
					enum_.name = line.substr(5, line.size()-5);
					in_enum = true;
				}
				else
				{
					in_struct = true;
					parse_header(line.c_str(), &datastruct);
				}
			}
			else
			{
				if (!in_scope)
				{
					if (line[0] == '{')
						in_scope = true;
				}
				else if (in_scope)
				{
					if (line[0] == '}')
					{
						if (in_struct)
						{
							// postprocess
							if (!datastruct.parent.empty())
							{
								// add in parent automatically.
								parsed_field pf;
								pf.name = "parent";
								pf.type = FIELDTYPE_STRUCT_INSTANCE;
								pf.ref_type = datastruct.parent;
								pf.is_array = false;
								pf.is_aux_ptr = false;
								pf.domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
								pf.show_in_editor = true;
								pf.is_build_config = false;
								datastruct.fields.insert(datastruct.fields.begin(), pf);
							}
							else if (datastruct.is_type_root)
							{
								parsed_field pf;
								pf.name = "_rtti_type";
								pf.type = FIELDTYPE_INT32;
								pf.is_array = false;
								pf.is_aux_ptr = false;
								pf.domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
								pf.show_in_editor = false;
								pf.is_build_config = false;
								datastruct.fields.insert(datastruct.fields.begin(), pf);
							}
							
							post_parse_struct(&datastruct);
							out->structs.push_back(datastruct);

							putki::parsed_struct tmp;
							datastruct = tmp;
							in_struct = false;
						}
						else if (in_enum)
						{
							int cur_val = 0;
							for (unsigned int i=0; i<enum_.values.size(); i++)
							{
								if (enum_.values[i].value == -666)
									enum_.values[i].value = cur_val++;
								else
									cur_val = enum_.values[i].value + 1;
							}

							out->enums.push_back(enum_);
							putki::parsed_enum tmp;
							enum_ = tmp;
							in_enum = false;
						}

						in_scope = false;
					}
					else
					{
						if (in_struct)
						{
							std::string spc = trim(line);

							if (!spc.empty() && spc[0] == '@')
							{
								// special.
								int np = spc.find_first_of(' ');
								if (np != std::string::npos)
								{
									std::string prop = spc.substr(0, np);
									std::string val = spc.substr(np+1, spc.size()-np-1);
									if (prop == "@inline-editor")
										datastruct.inline_editor = val;
								}

							}
							else
							{
								parsed_field pf;
								parse_field(line.c_str(), &pf);
								datastruct.fields.push_back(pf);
							}
						}
						if (in_enum)
						{
							putki::enum_value ev;
							ev.value = -666;

							line = trim(line);
							if (line[line.size()-1] == ',')
								line.erase(line.size()-1,1);

							int eq = line.find_first_of('=');
							int end = line.size();

							ev.value = -666;
							if (eq != std::string::npos)
							{
								ev.value = atoi(line.substr(eq+1, line.size()-eq-1).c_str());
								end = eq;
							}

							ev.name = trim(line.substr(0, end));

							enum_.values.push_back(ev);
						}
					}
				}
			}
		}

		char tmp[128];
		sprintf(tmp, "=> Automatically generated by Putki Parser Build [%s @ %s] [V12] Source signature [%d]", __DATE__, __TIME__, crc32(0, everything.c_str(), everything.size()));
		out->signature = tmp;
		return true;
	}
}
