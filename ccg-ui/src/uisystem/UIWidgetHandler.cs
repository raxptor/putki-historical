namespace CCGUI
{
	public interface UIWidgetHandler
	{
		UIWidgetRenderer CreateWidgetRenderer(outki.UIWidget widget);
		UIElementRenderer CreateRenderer(outki.UIElement element);
	}
}

