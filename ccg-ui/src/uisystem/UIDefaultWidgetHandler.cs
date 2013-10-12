
namespace CCGUI
{
	public class UIDefaultWidgetHandler
	{
		public static UIWidgetRenderer CreateWidgetRenderer(outki.UIWidget widget, UIWidgetHandler handler)
		{
			return new UIWidgetRenderer(widget, handler);
		}
		
		public static UIElementRenderer CreateElementRenderer(outki.UIElement element, UIWidgetHandler handler)
		{
			switch (element._rtti_type)
			{
				case outki.UIWidgetElement.TYPE:
					return handler.CreateWidgetRenderer(((outki.UIWidgetElement)element).widget);
				case outki.UIBitmapElement.TYPE:
					return new UIBitmapElementRenderer((outki.UIBitmapElement)element);
				case outki.UITextElement.TYPE:
					return new UITextElementRenderer((outki.UITextElement)element);
				case outki.UIFillElement.TYPE:
					return new UIFillElementRenderer((outki.UIFillElement)element);
				case outki.UIButtonElement.TYPE:
					return new UIButtonElementRenderer((outki.UIButtonElement)element);
				default:
					return null;
			}
		}
	}
}