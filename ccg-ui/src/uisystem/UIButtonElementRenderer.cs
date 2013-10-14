namespace CCGUI
{
	class UIButtonElementRenderer : UIElementRenderer
	{
		outki.UIButtonElement m_element;

		UIFill m_f0, m_f1, m_f2;
		UIFont m_font;
		UIFont.FormattedText m_formattedText;
		UIRenderer.Texture m_normal, m_pressed;
		UITouchInteraction m_touchInteraction = new UITouchInteraction();

		public UIButtonElementRenderer(outki.UIButtonElement element)
		{
			m_element = element;
			if (m_element.Style != null)
			{
				m_f0 = new UIFill(m_element.Style.Normal);
				m_f1 = new UIFill(m_element.Style.Highlight);
				m_f2 = new UIFill(m_element.Style.Pressed);
				if (m_element.Style.FontStyle != null)
					m_font = new UIFont(m_element.Style.FontStyle.Font);
			}
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			if (m_f0 != null)
			{
				m_f0.ResolveTextures(rctx);
				m_f1.ResolveTextures(rctx);
				m_f2.ResolveTextures(rctx);
			}

			if (m_font != null)
				m_formattedText = m_font.FormatText(rctx, m_element.Text, m_element.Style.FontStyle.PixelSize);
			else
				m_formattedText = null;

			if (m_element.TextureNormal != null)
				m_normal = rctx.TextureManager.ResolveTexture(m_element.TextureNormal, rctx.LayoutScale, 0, 0, 1, 1);

			if (m_element.TexturePressed != null)
				m_pressed = rctx.TextureManager.ResolveTexture(m_element.TexturePressed, rctx.LayoutScale, 0, 0, 1, 1);
		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			if (UIElementLogic.Button(rctx.InputManager, this, layout.x0, layout.y0, layout.x1, layout.y1, m_touchInteraction))
			{
				// clicked.
				if (m_element.Event.Length > 0)
					rctx.EventHandler.OnEvent(m_element.Event);
			}

			// behold the prettiness.
			switch (UIElementLogic.GetButtonVisualState(rctx.InputManager, this, m_touchInteraction))
			{
				case UIElementLogic.ButtonVisualStates.NORMAL:
					UIRenderer.SetColor(new UIRenderer.RColor(1,1,1,0.5f));	
					if (m_f0 != null)
						m_f0.Draw(rctx, layout.x0, layout.y0, layout.x1, layout.y1);
					if (m_normal != null)
						UIRenderer.DrawTexture(m_normal, layout.x0, layout.y0, layout.x1, layout.y1);
					break;
				case UIElementLogic.ButtonVisualStates.MOUSEOVER:
					UIRenderer.SetColor(new UIRenderer.RColor(1,1,1,0.7f));
					if (m_f1 != null)
						m_f1.Draw(rctx, layout.x0, layout.y0, layout.x1, layout.y1);
					if (m_normal != null)
						UIRenderer.DrawTexture(m_normal, layout.x0, layout.y0, layout.x1, layout.y1);
					break;
				case UIElementLogic.ButtonVisualStates.PRESSED:
					UIRenderer.SetColor(new UIRenderer.RColor(1,1,1,1));
					if (m_f2 != null)
						m_f2.Draw(rctx, layout.x0, layout.y0, layout.x1, layout.y1);
					if (m_pressed != null)
						UIRenderer.DrawTexture(m_pressed, layout.x0, layout.y0, layout.x1, layout.y1);
					break;
			}
	
			UIRenderer.SetColor(new UIRenderer.RColor(1,1,1,1));
		
			if (m_font != null && m_formattedText != null)
				m_font.Render(rctx, (layout.x0 + layout.x1 - m_formattedText.x1) / 2, (layout.y0 + layout.y1 - (m_formattedText.y1 - m_formattedText.y0)) / 2 - m_formattedText.y0, m_formattedText);
		}
	}
}