#pragma once

//////////////////////////////////////////////////////////////////////////////
// Here are the typedefs needed to compile the generated code for the runtime

namespace outki
{
	typedef void* instance_t;

	struct depwalker_i
	{
		virtual void pointer(instance_t *ptr) = 0;
	};

	typedef unsigned int u32;
}


