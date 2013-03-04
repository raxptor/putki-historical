#ifndef __PUTKI_PARSE_H__
#define __PUTKI_PARSE_H__

#include <putki/fieldtypes.h>

#include <sstream>
#include <vector>

namespace putki
{	
	struct parsed_field
	{
		putki::field_type type;
		bool is_array;

		std::string name;	
		std::string ref_type;
	};

	struct parsed_struct
	{
		int domains;
		std::string name;
		std::vector<parsed_field> fields;
	};

	struct parsed_file
	{
		std::vector<parsed_struct> structs;
	};

	void parse(const char *in_path, parsed_file *out);
}

#endif
