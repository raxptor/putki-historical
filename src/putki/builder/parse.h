#pragma once

#include <putki/builder/typereg.h>
#include <vector>

namespace putki
{
	namespace parse
	{
		struct data;
		struct node;

		data * parse(const char *full_path);
		void free(data *d);

		node *get_root(data*);

		node *get_array_item(node *arr, unsigned int i);
		node *get_object_item(node *obj, const char *field);
		const char *get_value_string(node *node);
		int get_value_int(node *node);
		float get_value_float(node *node);

		bool parse_hexstream_bytes(node *node, std::vector<unsigned char> & out);
	}
}
