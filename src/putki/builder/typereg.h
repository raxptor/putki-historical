#pragma once

#include <putki/runtime.h>
#include <cstring>
#include <iostream>

namespace putki
{
	typedef void* instance_t;
    typedef const char *type_t;

	struct i_field_desc
	{
		const char *name;
	};

    inline bool type_eq(type_t a, type_t b)
    {
        return !strcmp(a, b);
    }

	namespace parse { struct node; }
	namespace db { struct data; }

	struct load_resolver_i
	{
		virtual void resolve_pointer(instance_t *ptr, const char *path) = 0;
	};
	
	struct depwalker_i
	{
		virtual bool pointer_pre(instance_t *on) = 0;  // pre descending into pointer.
		virtual void pointer_post(instance_t *on) = 0; // post descending into pointer.
 	};
	
	struct type_handler_i
	{
		// info
		virtual const char *name() = 0;
		virtual int id() = 0; // unique type id
	
		// instantiate / destruct.
		virtual instance_t alloc() = 0;
		virtual instance_t clone(instance_t source) = 0;
		virtual void free(instance_t) = 0;
		
		// reading / writing
		virtual void fill_from_parsed(parse::node *pn, instance_t target, load_resolver_i *resolver) = 0;
		virtual void write_json(putki::db::data *ref_source, instance_t source, std::ostream & out, int indent) = 0;

		virtual char* write_into_buffer(putki::runtime rt, instance_t source, char *beg, char *end) = 0;

		// recurse down and report all pointers
		virtual void walk_dependencies(instance_t source, depwalker_i *walker, bool traverseChildren) = 0;
	};

	// used by dll interface, forward decl here for getters.
	struct ext_field_handler_i;
	struct ext_type_handler_i;

	void typereg_init();
	void typereg_register(type_t, type_handler_i *dt);
	type_handler_i *typereg_get_handler(type_t);


}
