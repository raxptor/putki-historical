#pragma once

#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/ccg-renderer.h>
#include <ccg-ui/uielement.h>
#include <ccg-ui/uiscreen.h>

namespace ccgui
{
	namespace uielement
	{
		void draw(renderinfo *rinfo, drawinfo *drawinfo)
		{
			switch (drawinfo->element->rtti_type_ref())
			{
				case outki::UIFillElement::TYPE_ID:
					break;
				case outki::UIWidgetElement::TYPE_ID:
					break;
				case outki::UIBitmapElement::TYPE_ID:
					{
						outki::UIBitmapElement *bmp = (outki::UIBitmapElement *) drawinfo->element;

						uiscreen::resolved_texture rt;
						if (uiscreen::resolve_texture(rinfo->screen, bmp->texture, &rt, 0, 0, 1, 1))
						{
							rinfo->backend->tex_rect(rt.texture, drawinfo->layout.x0, drawinfo->layout.y0, drawinfo->layout.x1, drawinfo->layout.y1, 
							                         rt.u0, rt.v0, rt.u1, rt.v1, 0xffffffff);
						}
					}
					break;
				default:
					break;
			}
		}
	}
}
