
#include "parse.h"
#include "log.h"

#include <jsmn/jsmn.h>

#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>

//#define PARSE_DEBUG(x) std::cout << x;
#define PARSE_DEBUG(x) {}

namespace putki
{
	namespace parse
	{
		struct data
		{
			node *root;
			std::vector<node*> allnodes;
			const char *alloc;
		};

		typedef std::unordered_map<std::string, node*> objmap_t;

		struct node
		{
			node()
			{
				key = 0;
				decoded = false;
			}

			jsmntok_t *token;

			char *value;
			int length;
			bool decoded;
			
			objmap_t object;
			std::vector<node*> arr;
			node *key;
			int children_left;
		};

		node *get_array_item(node *arr, unsigned int i)
		{
			if (!arr) {
				return 0;
			}

			if (i < arr->arr.size()) {
				return arr->arr[i];
			}
			return 0;
		}

		node *get_object_item(node *obj, const char *field)
		{
			if (!obj) {
				return 0;
			}
			
			objmap_t::iterator i = obj->object.find(field);
			if (i != obj->object.end())
				return i->second;

			return 0;
		}
		
		inline unsigned int unhex(char c)
		{
			if (c >= '0' && c <= '9')
				return c - '0';
			if (c >= 'a' && c <= 'f')
				return 10 + c - 'a';
			return 0;
		}

		const char *get_value_string(node *node)
		{
			if (!node || !node->length) {
				return "";
			}
			
			if (node->decoded)
				return node->value;
			
			// un-parse the \u0000
			char *str = node->value;
			char *out = node->value; // original null terminator
			
			bool shortened = false;
			int p = 0;
			while (p < (node->length-5))
			{
				if (str[p] == '\\' && str[p+1] == 'u')
				{
					if (str[p+2] != '0' || str[p+3] != '0')
					{
							APP_WARNING("Error in string \"" << node->value << "\". Will not properly decode this \\uXXXX encoded character because it contains character > 256, and only 8-bit values are supported. The escape sequences must be encodings of the original encoded string bytes.");
					}
					
					*out++ = (char)(16 * unhex(str[p+4]) + unhex(str[p+5]));
					p += 6;
					
					// shorten string..
					shortened = true;
				}
				else if (shortened)
				{
					// ..need to continue writing!
					*out++ = str[p++];
				}
				else
				{
					p++;
					out++;
				}
			}
			
			while (p < node->length)
				*out++ = str[p++];

			*out = 0;
			
			node->decoded = true;
			node->length = out - node->value;

			return node->value;
		}

		bool parse_stringencoded_byte_array(node *node, std::vector<unsigned char> & out)
		{
			if (!node)
				return true;

			if (node->length == 0)
				return false;

			if (!strcmp(node->value, "<empty>"))
				return true;

			// hexstream
			for (int i=0;i<node->length;i+=2)
				out.push_back(16 * (unsigned int)(node->value[i] - 'a') + (node->value[i+1] - 'a'));

			return true;
		}

		int get_value_int(node *node)
		{
			if (!node) {
				return 0;
			}

			return atoi(node->value);
		}

		float get_value_float(node *node)
		{
			if (!node) {
				return 0;
			}

			return (float) atof(node->value);
		}
		
		data * parse(const char *full_path)
		{
			FILE *fp = ::fopen(full_path, "rb");
			if (!fp) {
				return 0;
			}
			
			::fseek(fp, 0, SEEK_END);
			long size = ftell(fp); // get current file pointer
			::fseek(fp, 0, SEEK_SET);
			if (size < 0) {
				return 0;
			}

			char *tmp = new char[size+1];
			int rd = ::fread(tmp, 1, size, fp);
			::fclose(fp);

			PARSE_DEBUG("Read " << rd << "bytes.." << std::endl);
			
			tmp[rd] = 0;
			data *p = parse_json(tmp, rd);
			p->alloc = tmp;
			return p;
		}
			
		
		data * parse_json(char *json, int size)
		{
			jsmn_parser p;

			int maxtok = 1024;
			jsmntok_t *tok;
			
			tok = new jsmntok_t[maxtok];

			jsmn_init(&p);
			int ret = jsmn_parse(&p, json, size, tok, maxtok);
			if (ret == JSMN_ERROR_NOMEM)
			{
				jsmn_init(&p);
				ret = jsmn_parse(&p, json, size, 0, 0);
				if (ret > maxtok)
				{
					maxtok = ret;
					delete [] tok;
					tok = new jsmntok_t[maxtok];
					jsmn_init(&p);
					ret = jsmn_parse(&p, json, size, tok, maxtok);
				}
			}
			
			if (ret <= 0)
			{
				APP_ERROR("Failed to parse with jsmn err:" << ret);
				delete [] tok;
				return 0;
			}

			if (tok[0].type != JSMN_OBJECT)
			{
				delete [] tok;
				APP_ERROR("First element is not object");
				return 0;
			}

			std::stack<node*> pst;

			int loc = 0;

			node *root = 0;

			data *pd = new data;

			do
			{
				node *top = pst.empty() ? 0 : pst.top();
				node *current = new node();

				// for unalloc only.
				pd->allnodes.push_back(current);

				if (!root) {
					root = current;
				}

				PARSE_DEBUG("Start parse on " << loc << " children=" << tok[loc].size << std::endl);

				current->token = &tok[loc];
				current->children_left = tok[loc].size;

				// chop chop!
				current->value = &json[current->token->start];
				json[current->token->end] = 0;
				current->length = current->token->end - current->token->start;

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

				while (!pst.empty() && pst.top()->children_left == 0)
				{
					PARSE_DEBUG("END OBJECT [" << top->value << "]" << std::endl);
					pst.pop();
				}

				loc++;

			} while (!pst.empty());

			PARSE_DEBUG("Parse success!" << std::endl);
			delete [] tok;

			pd->root = root;
			pd->alloc = 0;
			return pd;
		}

		node *get_root(data *pd)
		{
			return pd->root;
		}

		void free(data *d)
		{
			for (unsigned i=0; i<d->allnodes.size(); i++)
				delete d->allnodes[i];
			delete [] d->alloc;
			delete d;
		}

	}
}
