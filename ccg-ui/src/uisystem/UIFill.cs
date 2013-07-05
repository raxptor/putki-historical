namespace CCGUI
{
	class UIFill
	{
		outki.UIFill m_data;
		UIRenderer.Texture[] m_tex = null;

		public UIFill(outki.UIFill data)
		{
			m_data = data;
		}

		public void ResolveTextures(UIRenderContext rctx)
		{
			if (m_data is outki.UISlice9Fill)
			{
				outki.UISlice9Fill s9 = (outki.UISlice9Fill) m_data;
				m_tex = new UIRenderer.Texture[9];
				m_tex[0] = rctx.TextureManager.ResolveTexture(s9.texture, rctx.LayoutScale, 0, 0, 1, 1);
			}
		}


		public void Draw(float x0, float y0, float x1, float y1)
		{
			switch (m_data._rtti_type)
			{
				case outki.UISolidFill.TYPE:
					break;
				case outki.UISlice9Fill.TYPE:
					break;
			}
		}
	}
}