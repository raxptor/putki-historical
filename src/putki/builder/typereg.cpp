#include "typereg.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <iostream>

namespace putki
{
	struct registry
	{
		std::map<std::string, type_handler_i*> handlers;
		std::map<int, type_handler_i*> by_number;
		std::vector<type_handler_i*> list;
	};

	namespace
	{
		registry *g_reg()
		{
			static registry r;
			return &r;
		}
	}

	struct depwalker_i::visited_set
	{
		std::set<void *> visited;
	};

	depwalker_i::depwalker_i()
	{
		_visited = 0;
	}

	depwalker_i::~depwalker_i()
	{
		delete _visited;
	}

	bool depwalker_i::pointer_pre_filter(instance_t *on)
	{
		bool ret = pointer_pre(on);
		if (ret)
		{
			if (!_visited)
			{
				_visited = new depwalker_i::visited_set();
				_visited->visited.insert(on);
				return true;
			}
			else if (_visited->visited.count(on))
			{
				return false;
			}
			else
			{
				_visited->visited.insert(on);
				return true;
			}
		}
		return false;
	}

	void typereg_init()
	{

	}

	void typereg_register(const char *type, type_handler_i *dt)
	{
		g_reg()->handlers[type] = dt;
		g_reg()->by_number[dt->id()] = dt;
		g_reg()->list.push_back(dt);
	}

	type_handler_i *typereg_get_handler(int type_id)
	{
		return g_reg()->by_number[type_id];
	}

	type_handler_i *typereg_get_handler_by_index(unsigned int idx)
	{
		if (idx < g_reg()->list.size()) {
			return g_reg()->list[idx];
		}
		return 0;
	}

	type_handler_i *typereg_get_handler(type_t t)
	{
		return g_reg()->handlers[t];
	}
}