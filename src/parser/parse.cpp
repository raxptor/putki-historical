#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <parser/parse.h>
#include <putki/domains.h>

namespace putki
{
	void parse_header(const char *input, putki::parsed_struct *out)
	{
		char *dup = strdup(input);
		char *tok = strtok(dup, " ");

		out->name = "";
		out->domains = 0;
		out->domains = putki::DOMAIN_RUNTIME | putki::DOMAIN_INPUT;

		while (tok)
		{
			if (out->name.empty())
			{
				out->name = tok;
			}
			else
			{
				std::cout << "flag is '" << tok << "'\n";
			}
			tok = strtok(0, " ");
		}

		free(dup);
	}

	void parse_field(const char *input, putki::parsed_field *out)
	{
		out->type = putki::FIELDTYPE_STRING;
		out->name = "name";		
	}

	void parse(const char *in_path, parsed_file *out)
	{
		std::cout << "Compiling [" << in_path << "]" << std::endl;
		std::ifstream f(in_path);

		bool in_struct = false;
		bool in_scope = false;

		std::string line;
		
		putki::parsed_struct datastruct;		

		while (getline(f, line))
		{
			if (line.empty())
				continue;
			if (line[0] == '#')
				continue;			

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
