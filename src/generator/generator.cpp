#include "generator.h"

#include <putki/domains.h>
#include <iostream>

namespace putki
{
	void write_runtime_header(putki::parsed_file *file)
	{
		for (size_t i=0;i!=file->structs.size();i++)
		{
			putki::parsed_struct *s = &file->structs[i];
			if (!(s->domains & putki::DOMAIN_RUNTIME))
				continue;

			std::cout << "struct " << s->name << " {" << std::endl;
			std::cout << "};" << std::endl;
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
