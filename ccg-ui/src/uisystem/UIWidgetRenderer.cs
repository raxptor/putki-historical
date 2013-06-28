namespace CCGUI
{
	public class UIWidgetRenderer : UIElementRenderer
	{
		struct ElementList
		{
			public outki.UIElement element;
			public UIElementLayout layout;
			public UIElementRenderer renderer;
		};

		outki.UIWidget m_data;
		ElementList[] m_rtData;

		public UIWidgetRenderer(outki.UIWidget widget, UIWidgetHandler handler)
		{
			m_data = widget;
			Setup(widget, handler);
		}

		public void Setup(outki.UIWidget widget, UIWidgetHandler handler)
		{
			int count = m_data.elements.Length;
			m_rtData = new ElementList[count];
			for (int i = 0; i < count; i++)
			{
				m_rtData[i].renderer = handler.CreateRenderer(m_data.elements[i]);
			}
		}

		public void LayoutElements(UIRenderContext rctx, float nsx0, float nsy0, float nsx1, float nsy1)
		{
			float expX = (nsx1 - nsx0) - m_data.width;
			float expY = (nsy1 - nsy0) - m_data.height;
			if (expX < 0) expX = 0;
			if (expY < 0) expY = 0;

			for (int i = 0; i < m_rtData.Length; i++)
			{
				outki.UIElement el = m_data.elements[i];

				UIElementLayout layout;
				layout.nsx0 = nsx0 + el.layout.x + el.expansion.x * expX;
				layout.nsy0 = nsy0 + el.layout.y + el.expansion.y * expY;
				layout.nsx1 = layout.nsx0 + el.layout.width + el.expansion.width * expX;
				layout.nsy1 = layout.nsy0 + el.layout.height + el.expansion.height * expY;

				layout.x0 = layout.nsx0 * rctx.LayoutScale + rctx.LayoutOffsetX;
				layout.y0 = layout.nsy0 * rctx.LayoutScale + rctx.LayoutOffsetY;
				layout.x1 = layout.nsx1 * rctx.LayoutScale + rctx.LayoutOffsetX;
				layout.y1 = layout.nsy1 * rctx.LayoutScale + rctx.LayoutOffsetY;
				
				m_rtData[i].element = el;
				m_rtData[i].layout = layout;
				m_rtData[i].renderer.OnLayout(rctx, ref layout);
			}
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			LayoutElements(rctx, elementLayout.nsx0, elementLayout.nsy0, elementLayout.nsx1, elementLayout.nsy1);
		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			for (int i = 0; i < m_rtData.Length; i++)
			{
				RenderSubElement(rctx, ref layout, m_rtData[i].element, m_rtData[i].renderer, ref m_rtData[i].layout);
			}
		}
	
		// This can be overriden for custom drawing.
		virtual public void RenderSubElement(UIRenderContext rctx, ref UIElementLayout myLayout, outki.UIElement element, UIElementRenderer renderer, ref UIElementLayout layout)
		{
			renderer.Render(rctx, ref layout);
		}
	}
}
