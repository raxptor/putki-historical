#include <iostream>
#include <fstream>
#include <sstream>

#include <putki/fieldtypes.h>

namespace putki
{

	struct header
	{
		std::string name;
	};

	struct field
	{
		putki::field_type type;
		std::string name;
	};

	void parse_header(const char *input, header *out)
	{
		out->name = "bajs";
	}

	void parse_field(const char *input, field *out)
	{
		out->type = putki::FIELDTYPE_INT32;
		out->name = "int32";
	}

	void compile(const char *in_path)
	{
		std::cout << "Compiling [" << in_path << "]" << std::endl;
		std::ifstream f(in_path);

		bool in_struct = false;
		bool in_scope = false;

		std::string line;
		header cur;

		while (getline(f, line))
		{
			if (line.empty())
				continue;
			if (line[0] == '#')
				continue;			

			if (!in_struct)
			{				
				in_struct = true;
				parse_header(line.c_str(), &cur);
				std::cout << "reading structure [" << cur.name << "]" << std::endl;
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
					}
					else
					{
						std::cout << "field:" << line << std::endl;
					}
				}
			}
		}
	}

}
