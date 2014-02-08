#pragma once

namespace putki
{
	namespace build_db
	{
		struct data;
		struct record;
		struct deplist;

		data* create(const char *path, bool load);
		void store(data *);

		void release(data *d);

		record *create_record(const char *input_path, const char *input_sig, const char *builder = 0);
		
		void set_builder(record *r, const char *builder);
		void add_output(record *r, const char *output_path, const char *builder);
		void add_input_dependency(record *r, const char *dependency);	
		void commit_record(data *d, record *r);

		void copy_input_dependencies(record *copy_to, record *copy_from);
		void append_extra_outputs(record *target, record *source);

		const char *enum_outputs(record *r, unsigned int pos);
		
		deplist* deplist_get(data *d, const char *path);
		const char *deplist_entry(deplist *d, unsigned int index);
		void deplist_free(deplist *);
	}
}
