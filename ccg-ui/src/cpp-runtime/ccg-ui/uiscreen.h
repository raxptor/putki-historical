#pragma once

#include <outki/types/ccg-ui/Screen.h>
#include <ccg-ui/ccg-renderer.h>

namespace ccgui
{
	namespace uiscreen
	{
		struct resolved_texture
		{
			outki::Texture *texture;
			float u0, v0, u1, v1;
		};

		struct instance;

		instance * create(outki::UIScreen *screen, render_api *rapi);
		void draw(instance *d, float x0, float y0, float x1, float y1);
		void free(instance *r);

		bool resolve_texture(instance *d, outki::Texture *texture, resolved_texture * out_resolved, float u0, float v0, float u1, float v1);
	}
}
