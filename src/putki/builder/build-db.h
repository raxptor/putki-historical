#pragma once

namespace putki
{
	namespace build_db
	{
		struct data;
		struct record;

		data* load(const char *path);

		void release(data *d);

		record *create_record(const char *input_path);
		void add_output(record *r, const char *output_path);
		void add_input_dependency(record *r, const char *dependency);	
		void commit_record(data *d, record *r);

		void copy_input_dependencies(record *copy_to, record *copy_from);
		void append_extra_outputs(record *target, record *source);

		const char *enum_outputs(record *r, unsigned int pos);
	}
}
