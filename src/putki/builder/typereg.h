#pragma once

#include <putki/runtime.h>
#include <cstring>

namespace putki
{
	typedef void* type_inst;
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
		virtual type_inst alloc() = 0;
		virtual void free(type_inst) = 0;
		virtual void fill_from_parsed(parse::node *pn, type_inst target) = 0;
		virtual char* write_into_buffer(putki::runtime rt, type_inst source, char *beg, char *end) = 0;
		virtual const char *name() = 0;
	};

	void typereg_init();
    void typereg_register(type_t, i_type_handler *dt);
	i_type_handler *typereg_get_handler(type_t);


}
