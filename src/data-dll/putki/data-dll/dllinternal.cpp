#include "dllinternal.h"

#include <string>
#include <map>
#include <vector>

namespace putki
{
	namespace
	{
		std::map<std::string, ext_type_handler_i* > typehandlers;
		std::vector<std::string> typelist;
	}

	void add_ext_type_handler(const char *type, ext_type_handler_i *i)
	{
		if (!typehandlers.count(type))
		{
			typelist.push_back(type);
			typehandlers[type] = i;
		}
	}

	ext_type_handler_i *get_ext_type_handler_by_index(unsigned int i)
	{
		if (i < typelist.size()) {
			return typehandlers[typelist[i]];
		}
		return 0;
	}

	ext_type_handler_i *get_ext_type_handler_by_name(const char *name)
	{
		std::map<std::string, ext_type_handler_i* >::iterator i = typehandlers.find(name);
		if (i != typehandlers.end()) {
			return i->second;
		}
		return 0;
	}
}
