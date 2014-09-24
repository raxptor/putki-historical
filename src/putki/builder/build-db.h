#pragma once

#include <putki/builder/log.h>

namespace putki
{
	namespace db
	{
		struct data;
	}
	
	namespace build_db
	{
		struct data;
		struct record;
		struct deplist;

		data* create(const char *path, bool load);
		void store(data *);

		void release(data *d);

		record *create_record(const char *input_path, const char *input_sig, const char *builder = 0);
		
		record *find(data *d, const char *output_path);
		const char *get_pointer(record *r, unsigned int index);
		const char *get_type(record *r);
		const char *get_signature(record *r);
		const char *get_builder(record *r);
		const char *get_parent(record *r);

		bool copy_existing(data *d, record *target, const char *path);

		void set_builder(record *r, const char *builder);
		void set_parent(record *r, const char *parent);

		void add_output(record *r, const char *output_path, const char *builder);
		void add_input_dependency(record *r, const char *dependency, const char *signature=0);
		void add_external_resource_dependency(record *r, const char *filepath, const char *signature);
		void insert_metadata(data *data, db::data *db, const char *path);
		void record_log(record *r, LogType type, const char *msg);
		void flush_log(record *r);
		
		void commit_record(data *d, record *r);

		void copy_input_dependencies(record *copy_to, record *copy_from);
		void merge_input_dependencies(record *target, record *source);

		void append_extra_outputs(record *target, record *source);

		const char *enum_outputs(record *r, unsigned int pos);

		deplist* inputdeps_get(data *d, const char *path);

		deplist* deplist_get(data *d, const char *path);

		const char *deplist_entry(deplist *d, unsigned int index);
		bool deplist_is_external_resource(deplist *d, unsigned int index);
		const char *deplist_path(deplist *d, unsigned int index);
		const char *deplist_signature(deplist *d, unsigned int index);

		void deplist_free(deplist *);
	}
}
