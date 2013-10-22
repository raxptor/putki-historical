#pragma once

#include <outki/types/ccg-ui/Elements.h>

namespace ccgui
{
	class render_api 
	{
		public:

			render_api()
			{

			}

			virtual ~render_api() 
			{

			}

			virtual void tex_rect(outki::Texture *texture, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color) = 0;

		private:
	};

}