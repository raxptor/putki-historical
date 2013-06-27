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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ViewerApp
{
	public class RootWindow : Window
	{
		CCGUI.UIScreenRenderer m_screenRenderer;
		Grid g = new Grid();

		public RootWindow(CCGUI.UIScreenRenderer scrn)
		{
			Background = Brushes.Transparent;
			Width = 900;
			Height = 700;
			Title = "CCG UI Viewer";
			m_screenRenderer = scrn;

			AddChild(g);
		}

		protected override void OnRender(DrawingContext drawingContext)
		{
			drawingContext.DrawRectangle(Brushes.Black, new Pen(Brushes.Black, 2), new Rect(0, 0, 100, 100));

			CCGUI.UIRenderer.dc = drawingContext;
						
			m_screenRenderer.Draw(0, 0, (float)g.ActualWidth, (float)g.ActualHeight);
			CCGUI.UIRenderer.dc = null;
		}
	}
}
