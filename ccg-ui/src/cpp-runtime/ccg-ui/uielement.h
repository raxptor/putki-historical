#pragma once

#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/ccg-renderer.h>

namespace ccgui
{
	namespace uielement
	{
		struct layoutinfo
		{
			float x0, y0, x1, y1;
		};

		struct renderinfo
		{
			ccgui::render_api *backend;
		};

		struct drawinfo
		{
			layoutinfo layout;
			outki::UIElement *element;
			//
			// + handler
		};

		void draw(renderinfo *rinfo, drawinfo *drawinfo);
	}
}
