#pragma once

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
	
	struct i_type_handler
	{
		// instantiate / destruct.
		virtual type_inst alloc() = 0;
		virtual void free(type_inst) = 0;

		// read/write
//		virtual type_inst parse(const char *str, unsigned int len) = 0;
//		virtual char* write_into_blob(type_inst d, char *beg, char *end) = 0;
	};

	void typereg_init();
    void typereg_register(type_t, i_type_handler *dt);
}
