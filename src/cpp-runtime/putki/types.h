#pragma once

//////////////////////////////////////////////////////////////////////////////
// Here are the typedefs needed to compile the generated code for the runtime

namespace outki
{
	typedef void* instance_t;
	typedef void (*reg_ptr_t)(instance_t *ptr);

	struct depwalker_i
	{
		reg_ptr_t pointer;
	};
}
