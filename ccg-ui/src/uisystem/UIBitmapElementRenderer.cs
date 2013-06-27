namespace CCGUI
{
	class UIBitmapElementRenderer : UIElementRenderer
	{
		outki.UIBitmapElement m_element;
		UIRenderer.Texture m_texture;

		public UIBitmapElementRenderer(outki.UIBitmapElement element)
		{
			m_element = element;
			m_texture = UIRenderer.ResolveTexture(m_element.texture);
		}

		public void Render(UIRenderContext rctx, float x0, float y0, float x1, float y1)
		{
			//UIRenderer.Rect(x0, y0, x1, y1);
			UIRenderer.DrawTexture(m_texture, x0, y0, x1, y1);
		}
	}
}