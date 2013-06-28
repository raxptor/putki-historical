using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CCGUI
{
	public class UIFont
	{
		outki.Font m_data;

		public struct FormattedGlyph
		{
			public float x, y;
			public float u0, v0, u1, v1;
			public float w, h;
		}

		public class FormattedText
		{
			public FormattedGlyph[] glyphs;
			public UIRenderer.Texture texture;
		};
		
		public UIFont(outki.Font data)
		{
			m_data = data;
		}

		public FormattedText FormatText(UIRenderContext ctx, string text, int pixelSize)
		{
			FormattedText fmt = new FormattedText();
			fmt.glyphs = new FormattedGlyph[text.Count()];

			outki.FontOutput f = null;
			double minDiff = 10000;

			foreach (outki.FontOutput fo in m_data.Outputs)
			{
				double diff = Math.Abs(1 - (float)fo.PixelSize / (float)pixelSize);
				if (diff < minDiff)
				{
					minDiff = diff;
					f = fo;
				}
			}

			float scaling = (float)pixelSize / (float)f.PixelSize;
			

			fmt.texture = ctx.TextureManager.ResolveTexture(f.OutputTexture, 1.0f, 0, 0, 1, 1);

			int pen = 0;

			for (int i = 0; i < text.Length; i++)
			{
				int gl = (int) text[i];

				fmt.glyphs[i].u0 = 0;
				fmt.glyphs[i].v0 = 0;
				fmt.glyphs[i].u1 = 0;
				fmt.glyphs[i].v1 = 0;

				if (i > 1)
				{
					int left = text[i - 1];
					int right = gl;

					for (int k=0;k<f.KerningCharL.Count();k++)
						if (f.KerningCharL[k] == left && f.KerningCharR[k] == right)
						{
							pen += f.KerningOfs[k];
							break;
						}
				}

				foreach (outki.FontGlyph fgl in f.Glyphs)
				{
					fmt.glyphs[i].w = -666;

					if (fgl.glyph == gl)
					{
						fmt.glyphs[i].u0 = fgl.u0;
						fmt.glyphs[i].v0 = fgl.v0;
						fmt.glyphs[i].u1 = fgl.u1;
						fmt.glyphs[i].v1 = fgl.v1;

						fmt.glyphs[i].x = (scaling * ((pen + fgl.bearingX) >> 6));
						fmt.glyphs[i].y = (scaling * ((0 + fgl.bearingY) >> 6));

						fmt.glyphs[i].w = fgl.pixelWidth * scaling;
						fmt.glyphs[i].h = fgl.pixelHeight * scaling;

						pen += fgl.advance;
						break;
					}
				}
			}

			return fmt;
		}

		public void Render(UIRenderContext ctx, float x0, float y0, string text, int size)
		{
			FormattedText ft = FormatText(ctx, text, size);
			for (int i = 0; i < ft.glyphs.Count(); i++)
			{
				if (ft.glyphs[i].w == -666)
					continue;

				float xp = x0 + ft.glyphs[i].x;
				float yp = y0 + ft.glyphs[i].y;
				UIRenderer.DrawTextureUV(ft.texture, xp, yp, xp + ft.glyphs[i].w, yp + ft.glyphs[i].h, ft.glyphs[i].u0, ft.glyphs[i].v0, ft.glyphs[i].u1, ft.glyphs[i].v1);
			}
		}
	}
}
