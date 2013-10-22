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

			virtual void bind_texture(outki::Texture *texture) = 0;

		private:
	};

}