#pragma once

//////////////////////////////////////////////////////////////////////////////
// Here are the typedefs needed to compile the generated code for the runtime

namespace putki
{
	typedef void* instance_t;

	struct depwalker_i
	{
		bool pointer_pre_filter(instance_t *ptr)
		{
			// todo, build in cycle dodging here
			return pointer_pre(ptr);
		}
		virtual bool pointer_pre(instance_t *ptr) {
			return true;
		}
		virtual void pointer_post(instance_t *ptr) = 0;
	};

	typedef unsigned int u32;
}


