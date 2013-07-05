#include "build-db.h"

#include <string>
#include <vector>
#include <iostream>

namespace putki
{
	namespace build_db
	{
		struct data
		{
			std::string path;
		};
	
		struct record
		{
			std::string source_path;
			std::vector<std::string> input_dependencies;
			std::vector<std::string> outputs;
		};

		data* load(const char *path)
		{
			data *d = new data();
			d->path = path;
			return d;
		}

		void release(data *d)
		{
			delete d;
		}

		record *create_record(const char *input_path)
		{
			record *r = new record();
			r->source_path = input_path;
			return r;
		}

		void add_output(record *r, const char *output_path)
		{
			r->outputs.push_back(output_path);
		}

		void add_input_dependency(record *r, const char *dependency)
		{
			r->input_dependencies.push_back(dependency);
		}

		void copy_input_dependencies(record *copy_to, record *copy_from)
		{
			copy_to->input_dependencies = copy_from->input_dependencies;
		}

		void append_extra_outputs(record *target, record *source)
		{
			for (unsigned int i=0;i<source->outputs.size();i++)
			{
				if (source->outputs[i] != source->source_path)
					target->outputs.push_back(source->outputs[i]);
			}
		}

		const char *enum_outputs(record *r, unsigned int pos)
		{
			if (pos < r->outputs.size())
				return r->outputs[pos].c_str();
			return 0;
		}

		void commit_record(data *d, record *r)
		{
			/*
			std::cout << " # Committing record for output [" << r->source_path << "]"  << std::endl;
			std::cout << "      input dependencies (" << r->input_dependencies.size() << "): "  << std::endl;
			for (unsigned int i=0;i<r->input_dependencies.size();i++)
				std::cout << "         " << r->input_dependencies[i] << std::endl;

			*/
			delete r;
		}
	}
}