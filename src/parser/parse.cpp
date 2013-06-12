#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <parser/parse.h>
#include <putki/domains.h>
#include <putki/sys/compat.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace putki
{
	void parse_header(const char *input, putki::parsed_struct *out)
	{
		char *dup = strdup(input);
		char *tok = strtok(dup, " ");

		out->name = "";
		out->domains = 0;
		out->domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
		out->is_type_root = false;

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
		out->name = "<invalid>";
		out->is_array =	false;
		out->domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;

		bool read_type = true;
		bool read_ptr_type = false;

		std::string type_unresolved;

		while (tok)
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
					
				std::cout << " Type is '" << type << "'" << std::endl;
				if (!strcmp(type.c_str(), "[no-out]"))
					out->domains = putki::DOMAIN_INPUT;
				else if (!strcmp(type.c_str(), "string"))
					out->type = putki::FIELDTYPE_STRING;
				else if (!strcmp(type.c_str(), "int") || !strcmp(type.c_str(), "u32"))
					out->type = putki::FIELDTYPE_INT32;
				else if (!strcmp(type.c_str(), "float"))
					out->type = putki::FIELDTYPE_FLOAT;
				else if (!strcmp(type.c_str(), "bool"))
					out->type = putki::FIELDTYPE_BOOL;
				else if (!strcmp(type.c_str(), "byte"))
					out->type = putki::FIELDTYPE_BYTE;
				else if (!strcmp(type.c_str(), "file"))
					out->type = putki::FIELDTYPE_FILE;
				else if (!strcmp(type.c_str(), "ptr"))
				{
					out->type = putki::FIELDTYPE_POINTER;
					read_ptr_type = true;
				}
				else
				{
					out->type = putki::FIELDTYPE_STRUCT_INSTANCE;
					out->ref_type = type;
				}
			}
			else if (read_ptr_type)
			{
				out->ref_type = tok;
				read_ptr_type = false;
			}
			else
			{
				out->name = tok;
			}

			tok = strtok(0, delim);
		}

		free(dup);
	}

	void parse(const char *in_path, const char *name, int type_id_start, parsed_file *out)
	{
		std::cout << "Compiling [" << name << "]" << std::endl;
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
		bool in_scope = false;

		std::string line;
		
		putki::parsed_struct datastruct;		

		while (getline(f, line))
		{
			if (line.empty())
				continue;
			if (line[0] == '#')
			{
				int space = line.find_first_of(' ');
				if (space != std::string::npos)
				{
					if (line.substr(0, space) == "#include")
					{
						out->includes.push_back(mypath + "/" + line.substr(space + 1, line.size() - space - 1));
					}
				}
				continue;
			}

			if (!in_struct)
			{				
				in_struct = true;
				parse_header(line.c_str(), &datastruct);
				std::cout << "reading datatype [" << datastruct.name << "]" << std::endl;
			}
			else
			{
				if (!in_scope)
				{
					if (line[0] == '{')
					{
						in_scope = true;
					}
				}
				else if (in_scope)
				{
					if (line[0] == '}')
					{
						in_scope = false;
						in_struct = false;
						datastruct.unique_id = type_id_start++;

						// postprocess
						if (!datastruct.parent.empty())
						{
							// add in parent automatically.
							parsed_field pf;
							pf.name = "parent";
							pf.type = FIELDTYPE_STRUCT_INSTANCE;
							pf.ref_type = datastruct.parent;
							pf.is_array = false;
							pf.domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
							datastruct.fields.insert(datastruct.fields.begin(), pf);
						}
						else if (datastruct.is_type_root)
						{
							parsed_field pf;
							pf.name = "_rtti_type";
							pf.type = FIELDTYPE_INT32;
							pf.is_array = false;
							pf.domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;
							datastruct.fields.insert(datastruct.fields.begin(), pf);
						}

						out->structs.push_back(datastruct);

						putki::parsed_struct tmp;
						datastruct = tmp;
					}
					else
					{
						std::cout << "field:" << line << std::endl;
						parsed_field pf;
						parse_field(line.c_str(), &pf);
						datastruct.fields.push_back(pf);
					}
				}
			}
		}

	}
}
