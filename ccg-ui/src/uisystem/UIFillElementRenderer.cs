namespace CCGUI
{
	class UIFillElementRenderer : UIElementRenderer
	{
		outki.UIFillElement m_element;
		UIFill m_fill;

		public UIFillElementRenderer(outki.UIFillElement element)
		{
			m_element = element;
			m_fill = new UIFill(element.fill);
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			m_fill.ResolveTextures(rctx);
		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			m_fill.Draw(rctx, layout.x0, layout.y0, layout.x1, layout.y1);
		}
	}
}
