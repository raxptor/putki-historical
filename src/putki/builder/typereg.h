#pragma once

#include <putki/runtime.h>

namespace putki
{
	typedef void* instance_t;
	typedef const char *type_t;

	struct i_field_desc
	{
		const char *name;
	};

	namespace parse { struct node; }
	namespace db { struct data; }
	struct sstream;

	struct load_resolver_i
	{
		virtual void resolve_pointer(instance_t *ptr, const char *path) = 0;
	};

	struct depwalker_i
	{
		// dodge cycles
		depwalker_i();
		virtual ~depwalker_i();
		struct visited_set;
		visited_set *_visited;
		bool pointer_pre_filter(instance_t *on, const char *ptr_type_name);

		void reset_visited();

		// pre descending into pointer.
		virtual bool pointer_pre(instance_t *on, const char *ptr_type_name) = 0;
	
		// after visited it.
		virtual void pointer_post(instance_t *on) { }; // post descending into pointer.
	};

	struct type_handler_i
	{
		// info
		virtual const char *name() = 0;
		virtual type_handler_i *parent_type() = 0;
		
		virtual int id() = 0; // unique type id
		virtual bool in_output() = 0;

		// instantiate / destruct.
		virtual instance_t alloc() = 0;
		virtual instance_t clone(instance_t source) = 0;
		virtual void free(instance_t) = 0;

		// reading / writing
		virtual void fill_from_parsed(parse::node *pn, instance_t target, load_resolver_i *resolver) = 0;
		virtual void write_json(putki::db::data *ref_source, instance_t source, putki::sstream & out, int indent) = 0;

		virtual char* write_into_buffer(runtime::descptr rt, instance_t source, char *beg, char *end) = 0;

		// recurse down and report all pointers
		virtual void walk_dependencies(instance_t source, depwalker_i *walker, bool traverseChildren, bool skipInputOnly = false, bool rttiDispatch = false) = 0;
	};

	// used by dll interface, forward decl here for getters.
	struct ext_field_handler_i;
	struct ext_type_handler_i;

	void typereg_init();
	void typereg_register(type_t, type_handler_i *dt);
	type_handler_i *typereg_get_handler(type_t);
	type_handler_i *typereg_get_handler(int type_id);
	type_handler_i *typereg_get_handler_by_index(unsigned int idx);
}
