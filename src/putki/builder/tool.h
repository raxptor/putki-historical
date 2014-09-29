#ifndef __PUTKI_TOOL_H__
#define __PUTKI_TOOL_H__

#include <putki/builder/db.h>

namespace putki
{
	enum CheckFlags
	{
		REQUIRE_RESOLVED    = 1,
		REQUIRE_HAS_PATHS   = 2
	};
	
	void verify_obj(db::data *primary, db::data *secondary, type_handler_i *th, instance_t obj, int check_flags, bool follow_aux_ptrs, bool follow_ptrs);
	void resolve_object_aux_pointers(db::data *db, const char *path);
	void clear_unresolved_pointers(db::data *db, type_handler_i *th, instance_t obj);
	bool is_valid_pointer(type_handler_i *th, const char *ptr_type);
}

#endif