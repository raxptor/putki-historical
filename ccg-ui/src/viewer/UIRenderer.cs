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

namespace CCGUI
{
	public class UIRenderer
	{
		public static DrawingContext dc = null;

		public class Texture
		{
			public ImageSource img;
		};

		public static void Rect(float x0, float y0, float x1, float y1)
		{
			dc.DrawRectangle(Brushes.Red, new Pen(), new Rect(x0, y0, x1 - x0, y1 - y0));
		}

		public static void DrawTexture(Texture tex, float x0, float y0, float x1, float y1)
		{
			dc.DrawImage(tex.img, new Rect(x0, y0, x1 - x0, y1 - y0));
		}

		public static Texture ResolveTexture(outki.Texture tex)
		{
			Texture tmp = new Texture();

			outki.TextureOutputPng png = (outki.TextureOutputPng) tex.Output;
			if (png != null)
			{
				tmp.img = new BitmapImage(new Uri(png.PngPath, UriKind.Relative));
			}
			return tmp;
		}
	}
}