using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Drawing;

namespace CCGUI
{
	public class UIRenderer
	{
		public static DrawingContext dc = null;

		public class Texture
		{
			public ImageSource img;
			public BitmapImage bmp;
		};

		public class RColor
		{
			public RColor(float R, float G, float B, float A)
			{
				r = R;
				g = G;
				b = B;
				a = A;
			}
			float r, g, b, a;
		}

		public static void Rect(float x0, float y0, float x1, float y1)
		{
			dc.DrawRectangle(Brushes.Red, new Pen(), new Rect(x0, y0, x1 - x0, y1 - y0));
		}

		private static Color ConvertColor(outki.UIColor color)
		{
			return Color.FromArgb(color.a, color.r, color.g, color.b);
		}

		public static void SetColor(RColor r)
		{

		}

		public static void DrawGradientRect(float x0, float y0, float x1, float y1, outki.UIColor tl, outki.UIColor tr, outki.UIColor bl, outki.UIColor br)
		{

		}

		public static void DrawSolidRect(float x0, float y0, float x1, float y1, outki.UIColor color)
		{
			Brush b = new SolidColorBrush(ConvertColor(color));
			dc.DrawRectangle(b, new Pen(), new Rect(x0, y0, x1 - x0, y1 - y0));
		}

		public static void DrawTexture(Texture tex, float x0, float y0, float x1, float y1)
		{
			dc.DrawImage(tex.img, new Rect(x0, y0, x1 - x0, y1 - y0));
		}

		public static void DrawTextureUV(Texture tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1)
		{
			if (u0 == u1 || v0 == v1)
				return;

			Texture tmp = new Texture();
			tmp.bmp = tex.bmp;
			tmp.img = new CroppedBitmap(tex.bmp, new Int32Rect((int)(u0 * tmp.bmp.Width), (int)(v0 * tmp.bmp.Height), (int)((u1 - u0) * tmp.bmp.Width), (int)((v1 - v0) * tmp.bmp.Height)));
			dc.DrawImage(tmp.img, new Rect(x0, y0, x1 - x0, y1 - y0));
		}

		public static Texture ResolveTextureUV(outki.Texture tex, float u0, float v0, float u1, float v1)
		{
			Texture tmp = new Texture();

			outki.TextureOutputPng png = (outki.TextureOutputPng)tex.Output;
			if (png != null)
			{
				tmp.bmp = new BitmapImage(new Uri(png.PngPath, UriKind.Relative));
				tmp.img = new CroppedBitmap(tmp.bmp, new Int32Rect((int)(u0 * tmp.bmp.Width), (int)(v0 * tmp.bmp.Height), (int)((u1 - u0) * tmp.bmp.Width), (int)((v1 - v0) * tmp.bmp.Height)));
			}
			return tmp;
		}

		public static Texture ResolveTexture(outki.Texture tex)
		{
			return ResolveTextureUV(tex, 0, 0, 1, 1);
		}
	}
}