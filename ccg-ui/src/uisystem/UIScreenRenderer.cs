namespace CCGUI
{
	public class UIScreenRenderer
	{
		outki.UIScreen m_screen;

		UIWidgetRenderer m_rootRenderer;

		public UIScreenRenderer(outki.UIScreen screen)
		{
			m_screen = screen;
			m_rootRenderer = new UIWidgetRenderer(m_screen.Root);
		}

		public void Draw(float x0, float y0, float x1, float y1)
		{
			UIRenderContext rctx = new UIRenderContext();
			m_rootRenderer.Render(rctx, x0, y0, x1, y1);
		}
	}

}