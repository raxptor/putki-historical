#include <vector>
#include <string>

namespace {
	std::vector<std::string> configs;
}

void add_build_config(const char *name)
{
	configs.push_back(name);
}

const char* get_build_config(unsigned int index)
{
	if (!index)
		return "Default";
	
	index--;
	
	if (index < configs.size())
		return configs[index].c_str();
	return 0;
}
