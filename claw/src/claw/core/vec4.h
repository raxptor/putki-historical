#pragma once

namespace claw
{
	struct vec4f
	{
		vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
		{
			
		};

		float x, y, z, w;
	};

}