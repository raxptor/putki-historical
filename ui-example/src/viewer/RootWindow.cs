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
		CCGUI.UIInputState m_input;
		CCGUI.UIInputManager m_inputManager;

		Grid g = new Grid();

		public RootWindow(CCGUI.UIScreenRenderer scrn)
		{
			Background = Brushes.Transparent;
			Width = 900;
			Height = 700;
			Title = "CCG UI Viewer";
			m_screenRenderer = scrn;
			m_input = new CCGUI.UIInputState();
			m_inputManager = new CCGUI.UIInputManager();

			AddChild(g);
		}

		protected override void OnRender(DrawingContext drawingContext)
		{
			drawingContext.DrawRectangle(Brushes.Black, new Pen(Brushes.Black, 2), new Rect(0, 0, 100, 100));
			CCGUI.UIRenderer.dc = drawingContext;

			Point p = Mouse.GetPosition(this);
			m_input.MouseX = (float)p.X;
			m_input.MouseY = (float)p.Y;

			m_inputManager.BeginFrame(m_input);

			m_screenRenderer.Draw(0, 0, (float)g.ActualWidth, (float)g.ActualHeight, m_inputManager);

			m_input.MouseClicked = false;

			CCGUI.UIRenderer.dc = null;
		}

		protected override void OnMouseDown(MouseButtonEventArgs e)
		{
			m_input.MouseDown = true;
			InvalidateVisual();
		}

		protected override void OnMouseUp(MouseButtonEventArgs e)
		{
			m_input.MouseClicked = true;
			m_input.MouseDown = false;
			InvalidateVisual();
		}

		protected override void OnMouseMove(MouseEventArgs e)
		{
			base.OnMouseMove(e);
			InvalidateVisual();
		}
	}
}
