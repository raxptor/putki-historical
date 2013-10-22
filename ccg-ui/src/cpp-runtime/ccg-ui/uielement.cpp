#pragma once

#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/ccg-renderer.h>
#include <ccg-ui/uielement.h>

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
						rinfo->backend->bind_texture(bmp->texture);
					}
					break;
				default:
					break;
			}
		}
	}
}
