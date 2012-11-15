#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{
	void write_runtime_header(putki::parsed_file *file, putki::runtime rt)
	{
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			if (rt == putki::RUNTIME_CPP_WIN32)
			{
				std::cout << "namespace outki {" << std::endl;
				std::cout << "	struct " << s->name << " {" << std::endl;

				for (size_t i=0;i<s->fields.size();i++)
				{
					std::cout << "		";
					switch (s->fields[i].type)
					{
						case FIELDTYPE_INT32:
							std::cout << "int ";
							break;
						case FIELDTYPE_POINTER:
							std::cout << s->fields[i].ptr_type << " *";
							break;
						case FIELDTYPE_STRING:
							std::cout << "const char *";
							break;
					}
					std::cout << s->fields[i].name << ";" <<std::endl;
				}

				std::cout << "	};" << std::endl;
				std::cout << "}" << std::endl;
			}
		}
	}

	

	void write_runtime_impl(putki::parsed_file *file)
	{

	}

	void write_meta_header(putki::parsed_file *file)
	{

	}

	void write_meta_impl(putki::parsed_file *file)
	{

	}
}
