namespace CCGUI
{
	class UIWidgetRenderer : UIElementRenderer
	{
		struct ElementList
		{
			public float x0, y0, x1, y1;
			public UIElementRenderer renderer;
		};

		outki.UIWidget m_data;
		ElementList[] m_rtData;

		public UIWidgetRenderer(outki.UIWidget widget)
		{
			m_data = widget;

			Setup(widget);
		}

		public void Setup(outki.UIWidget widget)
		{
			int count = m_data.elements.Length;
			m_rtData = new ElementList[count];
			for (int i = 0; i < count; i++)
			{
				m_rtData[i].renderer = CreateRenderer(m_data.elements[i]);
			}
			Layout(0, 0, m_data.width, m_data.height);
		}

		public void Layout(float x0, float y0, float x1, float y1)
		{
			float expX = (x1 - x0) - m_data.width;
			float expY = (y1 - y0) - m_data.height;
			if (expX < 0) expX = 0;
			if (expY < 0) expY = 0;

			for (int i = 0; i < m_rtData.Length; i++)
			{
				outki.UIElement el = m_data.elements[i];
				m_rtData[i].x0 = el.layout.x + el.expansion.x * expX;
				m_rtData[i].y0 = el.layout.y + el.expansion.y * expY;
				m_rtData[i].x1 = m_rtData[i].x0 + el.layout.width + el.expansion.width * expX;
				m_rtData[i].y1 = m_rtData[i].y0 + el.layout.height + el.expansion.height * expY;
			}
		}

		public UIElementRenderer CreateRenderer(outki.UIElement element)
		{
			switch (element._rtti_type)
			{
				case outki.UIWidgetElement.TYPE:
					return new UIWidgetRenderer(((outki.UIWidgetElement)element).widget);
				case outki.UIBitmapElement.TYPE:
					return new UIBitmapElementRenderer((outki.UIBitmapElement)element);
				default:
					return null;
			}
		}

		public void Render(UIRenderContext rctx, float x0, float y0, float x1, float y1)
		{
			Layout(x0, y0, x1, y1);
			for (int i = 0; i < m_rtData.Length; i++)
			{
				if (m_rtData[i].renderer != null)
					m_rtData[i].renderer.Render(rctx, m_rtData[i].x0, m_rtData[i].y0, m_rtData[i].x1, m_rtData[i].y1);
			}
		}
	}
}
