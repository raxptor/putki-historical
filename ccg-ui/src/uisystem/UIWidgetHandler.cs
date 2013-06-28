namespace CCGUI
{
	public interface UIWidgetHandler
	{
		UIElementRenderer CreateRootRenderer(outki.UIWidget widget);	
		UIElementRenderer CreateRenderer(outki.UIElement element);
	}
}

