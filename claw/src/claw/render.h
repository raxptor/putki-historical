#ifndef __CLAW_RENDER_H___
#define __CLAW_RENDER_H___

#include <claw/appwindow.h>

namespace claw
{
	namespace render
	{
		struct data;

		data* create(appwindow::data *window);
		void destroy(data *);

		void begin(data *d, bool clearcolor, bool cleardepth, unsigned int clear_color);
		void end(data *d);
		void present(data *d);

		void solid_rect(data *d, float x0, float y0, float x1, float y1, unsigned int color);
	}
}

#endif