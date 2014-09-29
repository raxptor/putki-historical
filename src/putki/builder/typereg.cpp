#include "typereg.h"
#include "log.h"

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

	void depwalker_i::reset_visited()
	{
		if (_visited)
			_visited->visited.clear();
	}

	depwalker_i::depwalker_i()
	{
		_visited = 0;
	}

	depwalker_i::~depwalker_i()
	{
		delete _visited;
	}

	bool depwalker_i::pointer_pre_filter(instance_t *on, const char *ptr_type)
	{
		bool ret = pointer_pre(on, ptr_type);
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
		if (!g_reg()->by_number[type_id])
		{
			APP_ERROR("Type id " << type_id << " has no handler");
		}
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
		if (!g_reg()->handlers[t])
		{
			APP_ERROR("Type " << t << " has no handler")
		}
	
		return g_reg()->handlers[t];
	}
}