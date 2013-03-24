#pragma once

#include "typereg.h"
#include "database.h"

namespace putki
{
	struct transform_i
	{
		virtual void transform(type_inst i, db *d) = 0; 
	};

	void transform_register(type_t t, transform_i *f);
}
