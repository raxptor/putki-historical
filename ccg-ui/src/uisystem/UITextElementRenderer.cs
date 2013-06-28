namespace CCGUI
{
	class UITextElementRenderer : UIElementRenderer
	{
		outki.UITextElement m_element;
		UIFont m_font;

		public UITextElementRenderer(outki.UITextElement element)
		{
			m_element = element;
			m_font = new UIFont(m_element.font);
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
		//	m_texture = rctx.TextureManager.ResolveTexture(m_element.texture, rctx.LayoutScale, 0, 0, 1, 1);
		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			m_font.Render(rctx, layout.x0, layout.y0, "Putki!", (int)(100 * rctx.LayoutScale));
			//UIRenderer.Rect(x0, y0, x1, y1);
			/*
			if (m_texture != null)
			{
				UIRenderer.DrawTexture(m_texture, layout.x0, layout.y0, layout.x1, layout.y1);
			}
			else
			{

			}
			 */
		}
	}
}