#ifndef __PUTKI_PARSER_RESOLVE_H__
#define __PUTKI_PARSER_RESOLVE_H__

#include "treeparser.h"

#include <map>

namespace putki
{

	typedef std::map<std::string, parsed_struct* > StructMapT;
	typedef std::map<std::string, parsed_enum* > EnumMapT;

	struct resolved_parse
	{
		StructMapT structs;
		EnumMapT enums;
	};

	bool resolve_parse(grand_parse *parse, resolved_parse *output);
}

#endif