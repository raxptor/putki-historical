#include "resolve.h"
#include "treeparser.h"

#include <iostream>

namespace putki
{
	bool resolve_parse(grand_parse *parse, resolved_parse *output)
	{
		int type_id = 0;
		
		for (int i=0;i!=parse->projects.size();i++)
		{
			// round to next hundreds
			type_id = 100 * ((type_id + 99) / 100);
			
			project *proj = &parse->projects[i];
			for (int f=0;f!=proj->files.size();f++)
			{
				parsed_file *pf = &proj->files[f];
				for (int s=0;s!=pf->structs.size();s++)
				{
					parsed_struct *str = &pf->structs[s];
					
					str->loadername = proj->loader_name;
					
					if (str->name.empty())
					{
						std::cerr << "Struct with empty name in file " << pf->filename << std::endl;
						return false;
					}

					StructMapT::iterator si = output->structs.find(str->name);
					if (si != output->structs.end())
					{
						std::cerr << "There exists more than one struct with the name [" << str->name << "]" << std::endl;
						return false;
					}
					
					str->unique_id = ++type_id;
					output->structs.insert(StructMapT::value_type(str->name, str));
				}

				for (int s=0;s!=pf->enums.size();s++)
				{
					parsed_enum *en = &pf->enums[s];
					if (en->name.empty())
					{
						std::cerr << "Enum with empty name in file " << pf->filename << std::endl;
						return false;
					}
					
					en->loadername = proj->loader_name;

					EnumMapT::iterator si = output->enums.find(en->name);
					if (si != output->enums.end())
					{
						std::cerr << "There exists more than one enum with the name [" << en->name << "]" << std::endl;
						return false;
					}

					output->enums.insert(EnumMapT::value_type(en->name, en));
				}
			}
		}

		std::cout << "Resolved with " << output->enums.size() << " enums and " << output->structs.size() << " structs" << std::endl;

		// cross resolve all structs
		StructMapT::iterator k = output->structs.begin();
		while (k != output->structs.end())
		{
			parsed_struct *s = (k++)->second;
			for (int f=0;f!=s->fields.size();f++)
			{
				parsed_field *pf = &s->fields[f];

				pf->resolved_ref_struct = 0;

				if (pf->type == FIELDTYPE_POINTER || pf->type == FIELDTYPE_STRUCT_INSTANCE)
				{
					if (pf->ref_type.empty())
					{
						std::cerr << "Field " << s->name << "." << pf->name << " has empty reftype." << std::endl;
						return false;
					}

					StructMapT::iterator lu = output->structs.find(pf->ref_type);
					if (lu == output->structs.end())
					{
						std::cerr << "Field " << s->name << "." << pf->name << " has unkown reftype [" << pf->ref_type << "]" << std::endl;
						return false;
					}

					pf->resolved_ref_struct = lu->second;
				}

				if (pf->type == FIELDTYPE_ENUM)
				{
					EnumMapT::iterator lu = output->enums.find(pf->ref_type);
					if (lu == output->enums.end())
					{
						std::cerr << "Field " << s->name << "." << pf->name << " has unkown (enum) reftype [" << pf->ref_type << "]" << std::endl;
						return false;
					}

					pf->resolved_ref_enum = lu->second;
				}
			}
		}
		return true;
	}
}

