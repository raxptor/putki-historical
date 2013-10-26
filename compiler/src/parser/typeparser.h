#ifndef __PUTKI_PARSE_H__
#define __PUTKI_PARSE_H__

#include <putki/fieldtypes.h>

#include <sstream>
#include <vector>

namespace putki
{	
	struct parsed_field
	{
		parsed_field()
		{
			_WROTE_DLL_FIELD_INDEX = -1;
		}

		int domains;

		putki::field_type type;
		bool is_array;
		bool is_aux_ptr;
		bool show_in_editor;
		
		std::string name;
		std::string ref_type;
		std::string def_value;

		int _WROTE_DLL_FIELD_INDEX;
	};

	struct enum_value
	{
		std::string name;
		int value;
	};

	struct parsed_enum
	{
		std::string name;
		std::vector<enum_value> values;
	};

	struct parsed_struct
	{
		int domains;
		int unique_id;
		std::string name;
		std::vector<parsed_field> fields;
		std::string parent;
		std::string inline_editor;
		bool is_type_root;
		bool permit_as_auxptr;
		bool permit_as_asset;
	};

	struct parsed_file
	{
		std::string filename;
		std::string sourcepath;
		std::string modulename;
		std::string signature;
		std::vector<parsed_struct> structs;
		std::vector<parsed_enum> enums;
		std::vector<std::string> includes;
	};

	void parse(const char *in_path, const char *name, int *type_id_counter, parsed_file *out);
}

#endif
