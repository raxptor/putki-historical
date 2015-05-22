#ifndef __PUTKI_PARSE_H__
#define __PUTKI_PARSE_H__

#include <putki/fieldtypes.h>

#include "buildconfigs.h"

#include <sstream>
#include <vector>

namespace putki
{
	struct parsed_struct;
	struct parsed_enum;

	struct parsed_field
	{
		parsed_field()
		{
			_WROTE_DLL_FIELD_INDEX = -1;
			resolved_ref_struct = 0;
			resolved_ref_enum = 0;
		}

		int domains;

		putki::field_type type;
		bool is_array;
		bool is_aux_ptr;
		bool is_build_config;
		
		bool show_in_editor;

		std::string name;
		std::string ref_type;
		std::string def_value;

		parsed_struct *resolved_ref_struct;
		parsed_enum *resolved_ref_enum;

		int _WROTE_DLL_FIELD_INDEX; // remove
	};

	struct enum_value
	{
		std::string name;
		int value;
	};

	struct parsed_enum
	{
		std::string name;
		std::string loadername;
		std::vector<enum_value> values;
	};

	struct parsed_struct
	{
		int domains;
		int unique_id;
		std::string name;
		std::string loadername;
		std::vector<parsed_field> fields;
		std::string parent;
		std::string inline_editor;
		std::vector<std::string> targets;
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

	bool parse(const char *in_path, const char *name, parsed_file *out);
}

#endif
