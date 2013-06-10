#pragma once

//////////////////////////////////////////////////////////////////////////////
// Here are the typedefs needed to compile the generated code for the runtime

namespace putki
{
	typedef void* instance_t;

	struct depwalker_i
	{
		virtual bool pointer_pre(instance_t *ptr) { return true; }
		virtual void pointer_post(instance_t *ptr) = 0;
	};

	typedef unsigned int u32;
}


