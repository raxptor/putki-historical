using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CCGUI
{
	class UITiledFill
	{
		private UIRenderer.Texture m_tex_f, m_tex_r, m_tex_b, m_tex_br;
		private float[] m_xs = null;
		private float[] m_ys = null;


		public static UITiledFill CreateFill(UIRenderContext ctx, outki.Texture tex, outki.UIBitmapTilingSettings settings, float x0, float y0, float x1, float y1)
		{
			UITiledFill tf = new UITiledFill();

			float tileW = settings.UseLayoutScale ? settings.TileWidth * ctx.LayoutScale : settings.TileWidth;
			float tileH = settings.UseLayoutScale ? settings.TileHeight * ctx.LayoutScale : settings.TileHeight;

			float numX = (x1 - x0) / tileW;
			float numY = (y1 - y0) / tileH;

			int cX = (int)Math.Ceiling(numX) + 1;
			int cY = (int)Math.Ceiling(numY) + 1;

			tf.m_xs = new float[cX];
			tf.m_ys = new float[cY];

			for (int i = 0; i < cX - 1; i++)
				tf.m_xs[i] = (int)(x0 + i * tileW);

			for (int i = 0; i < cY - 1; i++)
				tf.m_ys[i] = (int)(y0 + i * tileH);

			tf.m_xs[cX - 1] = x1;
			tf.m_ys[cY - 1] = y1;

			float lastU1 = (tf.m_xs[cX - 1] - tf.m_xs[cX - 2]) / tileW;
			float lastV1 = (tf.m_ys[cY - 1] - tf.m_ys[cY - 2]) / tileH;

			tf.m_tex_f  = ctx.TextureManager.ResolveTexture(tex, ctx.LayoutScale, 0, 0, 1, 1);
			tf.m_tex_r  = ctx.TextureManager.ResolveTexture(tex, ctx.LayoutScale, 0, 0, lastU1, 1);
			tf.m_tex_b  = ctx.TextureManager.ResolveTexture(tex, ctx.LayoutScale, 0, 0, 1, lastV1);
			tf.m_tex_br = ctx.TextureManager.ResolveTexture(tex, ctx.LayoutScale, 0, 0, lastU1, lastV1);

			return tf;
		}

		private UITiledFill()
		{

		}

		public void Draw()
		{	
			for (int x = 0; x < m_xs.Length - 1; x++)
			{
				UIRenderer.Texture t = m_tex_f;
				if (x == m_xs.Length - 2)
					t = m_tex_r;

				for (int y = 0; y < m_ys.Length - 1; y++)
				{
					if (y == m_ys.Length - 2)
					{
						if (x == m_xs.Length - 2)
							t = m_tex_br;
						else
							t = m_tex_b;
					}

					UIRenderer.DrawTexture(t, m_xs[x], m_ys[y], m_xs[x + 1], m_ys[y + 1]);
				}
			}
		}
	}
}
