namespace CCGUI
{
	class UIBitmapElementRenderer : UIElementRenderer
	{
		outki.UIBitmapElement m_element;
		UIRenderer.Texture m_texture;

		public UIBitmapElementRenderer(outki.UIBitmapElement element)
		{
			m_element = element;
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			if (m_element.texture != null)
				m_texture = rctx.TextureManager.ResolveTexture(m_element.texture, rctx.LayoutScale, 0, 0, 1, 1);
		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			//UIRenderer.Rect(x0, y0, x1, y1);
			if (m_texture != null)
			{
				UIRenderer.DrawTexture(m_texture, layout.x0, layout.y0, layout.x1, layout.y1);
			}
			else
			{

			}
		}
	}
}