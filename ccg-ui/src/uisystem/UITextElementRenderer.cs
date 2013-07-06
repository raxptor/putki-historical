namespace CCGUI
{
	class UITextElementRenderer : UIElementRenderer
	{
		outki.UITextElement m_element;
		UIFont m_font;
		UIFont.FormattedText m_fmted;
		float m_x0, m_y0;

		public UITextElementRenderer(outki.UITextElement element)
		{
			m_element = element;
			m_font = new UIFont(m_element.font);
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			m_fmted = m_font.FormatText(rctx, m_element.Text, m_element.pixelSize);

			switch (m_element.HorizontalAlignment)
			{
				case outki.UIHorizontalAlignment.UIHorizontalAlignment_Center:
					m_x0 = (elementLayout.x0 + elementLayout.x1 - (m_fmted.x1 - m_fmted.x0)) / 2 - m_fmted.x0;
					break;
				case outki.UIHorizontalAlignment.UIHorizontalAlignment_Right:
					m_x0 = elementLayout.x1 - m_fmted.x1;
					break;
				default: // left align
					m_x0 = elementLayout.x0 - m_fmted.x0;
					break;
			}

			switch (m_element.VerticalAlignment)
			{
				case outki.UIVerticalAlignment.UIVerticalAlignment_Top:
					m_y0 = elementLayout.y0 - m_fmted.facey0;
					break;
				case outki.UIVerticalAlignment.UIVerticalAlignment_Bottom:
					m_y0 = elementLayout.y1 - m_fmted.facey1;
					break;
				default: // center
					// glyph actual sizes method
					m_y0 = (elementLayout.y0 + elementLayout.y1 - (m_fmted.y1 - m_fmted.y0)) / 2 - m_fmted.y0;
					break;
			}

		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			m_font.Render(rctx, m_x0, m_y0, m_fmted);
		}
	}
}