using System;

namespace CCGUI
{
	class UIFill
	{
		outki.UIFill m_data;
		UIRenderer.Texture[] m_tex = null;
		UITiledFill m_tiledFillRender = null;

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

				int p = 0;

				int[] us = new int[4] { 0, s9.MarginLeft, s9.texture.Width - s9.MarginLeft, s9.texture.Width };
				int[] vs = new int[4] { 0, s9.MarginTop, s9.texture.Height - s9.MarginBottom, s9.texture.Height };

				for (int y = 0; y < 3; y++)
				{
					int v0 = vs[y];
					int v1 = vs[y+1];

					for (int x = 0; x < 3; x++)
					{
						int u0 = us[x];
						int u1 = us[x+1];
						m_tex[p++] = rctx.TextureManager.ResolveTexture(s9.texture, rctx.LayoutScale, u0, v0, u1, v1, true);
					}
				}
			}
		}

		public void OnLayout(UIRenderContext rctx, float x0, float y0, float x1, float y1)
		{
			if (m_data is outki.UIBitmapFill)
			{
				outki.UIBitmapFill bf = m_data as outki.UIBitmapFill;
				if (bf.Tiling.EnableRepeatTiling)
					m_tiledFillRender = UITiledFill.CreateFill(rctx, bf.Texture, bf.Tiling, x0, y0, x1, y1);
				else
					m_tiledFillRender = null;
			}
		}

		public void Draw(UIRenderContext ctx, float x0, float y0, float x1, float y1)	
		{
			if (m_data == null)
				return;
			if (Putki.LiveUpdate.Update(ref m_data))
				ResolveTextures(ctx);

			switch (m_data._rtti_type)
			{
				case outki.UISolidFill.TYPE:
					{
						outki.UISolidFill sf = m_data as outki.UISolidFill;
						UIRenderer.DrawSolidRect(x0, y0, x1, y1, sf.color);
						break;
					}

				case outki.UIBitmapFill.TYPE:
					{
						if (m_tiledFillRender != null)
						{
							m_tiledFillRender.Draw();
						}
						else
						{

						}
						break;
					}

				case outki.UISlice9Fill.TYPE:
					{
						outki.UISlice9Fill s9 = m_data as outki.UISlice9Fill;
						
						x0 -= (float) Math.Floor(s9.ExpandLeft * ctx.LayoutScale);
						y0 -= (float) Math.Floor(s9.ExpandTop * ctx.LayoutScale);
						x1 += (float) Math.Floor(s9.ExpandRight * ctx.LayoutScale);
						y1 += (float) Math.Floor(s9.ExpandBottom * ctx.LayoutScale);

						float[] xs = new float[4] { x0, x0 + s9.MarginLeft, x1 - s9.MarginLeft, x1 };
						float[] ys = new float[4] { y0, y0 + s9.MarginTop, y1 - s9.MarginBottom, y1 };

						for (int y = 0; y < 3; y++)
						{
							if (ys[y + 1] <= ys[y])
								continue;

							for (int x = 0; x < 3; x++)
							{
								if (xs[x + 1] <= xs[x])
									continue;

								UIRenderer.DrawTexture(m_tex[3 * y + x], xs[x], ys[y], xs[x + 1], ys[y + 1]);
							}
						}
					}
					break;
			}
		}
	}
}