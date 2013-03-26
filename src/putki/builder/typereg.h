#pragma once

#include <putki/runtime.h>
#include <cstring>

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
	
	struct i_type_handler
	{
		// instantiate / destruct.
		virtual instance_t alloc() = 0;
		virtual void free(instance_t) = 0;
		virtual void fill_from_parsed(parse::node *pn, instance_t target) = 0;
		virtual char* write_into_buffer(putki::runtime rt, instance_t source, char *beg, char *end) = 0;
		virtual const char *name() = 0;
	};

	void typereg_init();
    void typereg_register(type_t, i_type_handler *dt);
	i_type_handler *typereg_get_handler(type_t);


}
