#pragma once

#include <outki/types/ccg-ui/Screen.h>
#include <ccg-ui/ccg-renderer.h>

namespace ccgui
{
	namespace uiscreen
	{
		struct instance;
		instance * create(outki::UIScreen *screen, render_api *rapi);
		void draw(instance *d, float x0, float y0, float x1, float y1);
		void free(instance *r);
	}
}
