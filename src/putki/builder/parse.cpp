
#include "parse.h"

#define _CRT_SECURE_NO_WARNINGS 0

extern "C"
{
#include <jsmn/jsmn.h>
}

#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>

// #define PARSE_DEBUG(x) std::cout << x;
#define PARSE_DEBUG(x) {}

namespace putki
{
namespace parse
{
	struct data
	{
		node *root;
	};

	struct node
	{
		node()
		{
			key = 0;
		}

		jsmntok_t *token;

		std::string value;
		std::map<std::string, node*> object;
		std::vector<node*> arr;
		node *key;
		int children_left;
	};

	node *get_array_item(node *arr, unsigned int i)
	{
		if (!arr)
			return 0;

		if (i < arr->arr.size())
			return arr->arr[i];
		return 0;
	}

	node *get_object_item(node *obj, const char *field)
	{
		if (!obj)
			return 0;

		if (obj->object.count(field))
			return obj->object[field];
		return 0;
	}

	const char *get_value_string(node *node)
	{
		if (!node)
			return "<missing>";

		return node->value.c_str();
	}

	int get_value_int(node *node)
	{
		if (!node)
			return 0;

		return atoi(node->value.c_str());
	}

	float get_value_float(node *node)
	{
		if (!node)
			return 0;

		return (float) atof(node->value.c_str());
	}

	void print_tree(node *n, int ilevel)
	{
		for (int i=0;i<ilevel;i++)
			std::cout << " ";

		if (n->token->type == JSMN_OBJECT)
		{
			std::cout << "[OBJECT]" << std::endl;
			std::map<std::string, node*>::iterator i = n->object.begin();
			while (i != n->object.end())
			{
				for (int j=0;j<ilevel;j++)
					std::cout << " ";

				std::cout << i->first << " => " << std::endl;
				print_tree(i->second, ilevel + i->first.size() + 4);
				++i;
			}
		}
		else if (n->token->type == JSMN_ARRAY)
		{
			std::cout << "<array>" << std::endl;
			for (unsigned int i=0;i<n->arr.size();i++)
			{
				for (int j=0;j<ilevel;j++)
					std::cout << " ";

				std::cout << "[" << i << "] => " << std::endl;
				print_tree(n->arr[i], ilevel + 8);
			}
		}
		else
		{
			std::cout << n->value;
		}

		std::cout << std::endl;
	}


	data * parse(const char *full_path)
	{
		std::ifstream f(full_path);

		FILE *fp = ::fopen(full_path, "rb");
		if (!fp)
			return 0;

		char tmp[65536];
		int rd = fread(tmp, 1, 65000, fp);

		PARSE_DEBUG("Read " << rd << "bytes.." << std::endl);

		tmp[rd] = 0;

		jsmn_parser p;
		jsmn_init(&p);

		const unsigned int maxtok = 4096;
		jsmntok_t tok[maxtok];
		jsmnerr_t err = jsmn_parse(&p, tmp, tok, maxtok);
		if (err != JSMN_SUCCESS)
		{
			std::cout << "Parse failure!" << std::endl;
			return 0;
		}

		if (tok[0].type != JSMN_OBJECT)
		{
			std::cout << "First element is not object!" << std::endl;
			return 0;
		}

		std::stack<node*> pst;

		int loc = 0;

		node *root = 0;

		std::string org_string(tmp);

		do
		{
			node *top = pst.empty() ? 0 : pst.top();
			node *current = new node();

			if (!root)
				root = current;

			PARSE_DEBUG("Start parse on " << loc << " children=" << tok[loc].size << std::endl);

			current->token = &tok[loc];
			current->children_left = tok[loc].size;

			current->value = org_string.substr(current->token->start, current->token->end - current->token->start);

			if (top)
			{
				// insert into top .
				if (top->token->type == JSMN_OBJECT)
				{
					if (!top->key)
					{
						PARSE_DEBUG( "  Adding key " << current->value << " " << std::endl);
						top->key = current;
					}
					else
					{
						PARSE_DEBUG("  Completing value [" << top->key->value << "] = [" << current->value << "]" << std::endl);
						top->object[top->key->value] = current;
						top->key = 0;
					}
				}
				else if (top->token->type == JSMN_ARRAY)
				{
					top->arr.push_back(current);
				}
			
				top->children_left--;
			}

			if (current->token->type == JSMN_OBJECT || current->token->type == JSMN_ARRAY)
			{
				PARSE_DEBUG("STARTING OBJECT [" << current->value << "]" << std::endl);
				pst.push(current);			
			}
			else
			{
				current->value = org_string.substr(current->token->start, current->token->end - current->token->start);
			}


			while (!pst.empty() && pst.top()->children_left == 0)
			{
				PARSE_DEBUG("END OBJECT [" << top->value << "]" << std::endl);
				pst.pop();
			}

			loc++;

		} while (!pst.empty());

		PARSE_DEBUG("Parse success!" << std::endl);

		data *pd = new data;
		pd->root = root;
		return pd;
	}

	node *get_root(data *pd)
	{
		return pd->root;
	}

	void free(data *d)
	{
		delete d;
	}

}
}