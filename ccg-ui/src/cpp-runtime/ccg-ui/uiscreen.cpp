#pragma once

#include "uiscreen.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/uielement.h>
#include <vector>

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
			instance *inst = new instance();
			inst->backend = rapi;
			inst->data = screen;
			return inst;
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
					LIVE_UPDATE(&layer.elements[j]);
					outki::UIElement *element = layer.elements[j];
					if (!element)
						continue;

					uielement::renderinfo ri;
					ri.backend = d->backend;
					ri.screen = d;

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
			LIVE_UPDATE(&d->data);
			LIVE_UPDATE(&d->data->Root);

			do_widget(d, d->data->Root, x0, y0, x1, y1);
		}

		bool resolve_texture(instance *d, outki::Texture *texture, resolved_texture * out_resolved, float u0, float v0, float u1, float v1)
		{
			for (unsigned int i=0;i<d->data->Atlases_size;i++)
			{
				outki::Atlas *atlas = d->data->Atlases[i];
				if (atlas)
				{
					for (unsigned int j=0;j<atlas->Outputs_size;j++)
					{
						const outki::AtlasOutput *output = &atlas->Outputs[j];
						if (output->Scale != 1.0f)
							continue;

						for (unsigned int k=0;k<output->Entries_size;k++)
						{
							const outki::AtlasEntry *entry = &output->Entries[k];
							if (!strcmp(entry->id, texture->id))
							{
								out_resolved->u0 = entry->u0;
								out_resolved->v0 = entry->v0;
								out_resolved->u1 = entry->u1;
								out_resolved->v1 = entry->v1;
								out_resolved->texture = output->Texture;
								return true;
							}
						}
					}
				}
			}

			if (texture->Output)
			{
				out_resolved->u0 = 0;
				out_resolved->v0 = 0;
				out_resolved->u1 = 1;
				out_resolved->v1 = 1;
				out_resolved->texture = texture;
				return true;
			}

			return false;
		}

	}
}
