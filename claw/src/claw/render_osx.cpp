#include "appwindow.h"

#include <OpenGL/gl.h>

namespace claw
{
	namespace render
	{
		struct data
		{
		
		};

		data* create(appwindow::data *window)
		{
			data *d = new data();
			return d;
		}
				
		void destroy(data *d)
		{
			delete d;
		}

		void begin(data *d, bool clearcolor, bool cleardepth, unsigned int clear_color)
		{
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		
		void end(data *d)
		{
			glFlush();
		}
		
		void present(data *d)
		{
		
		}

		void solid_rect(data *d, float x0, float y0, float x1, float y1, unsigned int color)
		{
		
		}
	}
}