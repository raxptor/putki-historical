#pragma once

#include "uiscreen.h"

#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/uielement.h>

namespace ccgui
{
	namespace uiscreen
	{

		struct instance
		{
			outki::UIScreen *data;
			render_api *backend;
		};

		instance * create(outki::UIScreen *screen, render_api *rapi)
		{
			instance *i = new instance();
			i->backend = rapi;
			i->data = screen;
			return i;
		}

		void free(instance *r)
		{
			delete r;
		}

		void do_widget(instance *d, outki::UIWidget *widget, float x0, float y0, float x1, float y1)
		{
			float expx = (x1 - x0) - widget->width;
			float expy = (y1 - y0) - widget->height;
			if (expx < 0) expx = 0;
			if (expy < 0) expy = 0;


			unsigned int layers = widget->layers_size;
			for (unsigned int i=0;i!=layers;i++)
			{
				const outki::UIElementLayer & layer = widget->layers[i];
				for (unsigned int j=0;j!=layer.elements_size;j++)
				{
					outki::UIElement *element = layer.elements[j];
					if (!element)
						continue;

					uielement::renderinfo ri;
					ri.backend = d->backend;

					uielement::drawinfo di;
					di.element = element;

					uielement::layoutinfo& li = di.layout;
					li.x0 = element->layout.x + expx * element->expansion.x;
					li.y0 = element->layout.y + expy * element->expansion.y;
					li.x1 = li.x0 + element->layout.width  + expx * element->expansion.width;
					li.y1 = li.y0 + element->layout.height + expy * element->expansion.height;

					uielement::draw(&ri, &di);
				}
			}
		}

		void draw(instance *d, float x0, float y0, float x1, float y1)
		{
			do_widget(d, d->data->Root, x0, y0, x1, y1);
		}

	}
}
