#ifndef __TREEPARSER_H__
#define __TREEPARSER_H__

#include "typeparser.h"

#include <string>
#include <vector>
#include <map>

namespace putki
{
	struct project
	{
		std::string start_path;
		std::string base_path;
		std::string full_path;
		std::string output_path;
		std::string module_name;
		std::string loader_name;
		std::vector<parsed_file> files;
		std::vector<std::string> deps, build_configs;
		std::string parse_error;
	};

	struct grand_parse
	{
		std::vector<project> projects;
	};

	bool parse_all_with_deps(grand_parse *output);
}

#endif
