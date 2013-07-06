
namespace CCGUI
{
	public class UIDefaultWidgetHandler
	{
		public static UIElementRenderer CreateRootRenderer(outki.UIWidget widget, UIWidgetHandler handler)
		{
			return new UIWidgetRenderer(widget, handler);
		}
		
		public static UIElementRenderer CreateRenderer(outki.UIElement element, UIWidgetHandler handler)
		{
			switch (element._rtti_type)
			{
				case outki.UIWidgetElement.TYPE:
					return new UIWidgetRenderer(((outki.UIWidgetElement)element).widget, handler);
				case outki.UIBitmapElement.TYPE:
					return new UIBitmapElementRenderer((outki.UIBitmapElement)element);
				case outki.UITextElement.TYPE:
					return new UITextElementRenderer((outki.UITextElement)element);
				case outki.UIFillElement.TYPE:
					return new UIFillElementRenderer((outki.UIFillElement)element);
				default:
					return null;
			}
		}
	}
}