#ifndef __PUTKI_DLL_INTERNAL_H__
#define __PUTKI_DLL_INTERNAL_H__

#include <putki/builder/typereg.h>
#include "dllinterface.h"

namespace putki
{
	struct type_handler_i;
	struct ext_type_handler_i;

	struct mem_instance_real : public mem_instance
	{
		type_handler_i *th;
		ext_type_handler_i *eth;
		instance_t inst;
		char *path;
	};

	void add_ext_type_handler(const char *type, ext_type_handler_i *i);
	ext_type_handler_i *get_ext_type_handler_by_index(unsigned int i);
	ext_type_handler_i *get_ext_type_handler_by_name(const char *name);

}

#endif

