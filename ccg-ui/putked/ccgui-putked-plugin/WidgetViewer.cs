using System;
using PutkEd;

namespace ccguiputkedplugin
{
	[System.ComponentModel.ToolboxItem (true)]
	public class WidgetViewer : Gtk.DrawingArea
	{
		inki.UIWidget m_widget;

		public WidgetViewer(inki.UIWidget widget)
		{
			m_widget = widget;
		}

		protected override bool OnButtonPressEvent (Gdk.EventButton ev)
		{
			// Insert button press handling code here.
			return base.OnButtonPressEvent (ev);
		}

		protected override bool OnExposeEvent (Gdk.EventExpose ev)
		{
			base.OnExposeEvent (ev);

			// Insert drawing code here.
			Gdk.GC g = new Gdk.GC(GdkWindow);
			g.RgbFgColor = new Gdk.Color(0,0,100);
			g.RgbBgColor = new Gdk.Color(0,0,0);


			for (int i=0;i<m_widget.get_layers_count();i++)
			{
				inki.UIElementLayer layer = m_widget.get_layers(i);
				if (layer != null)
				{
					Console.WriteLine("  and it has " + layer.get_elements_count() + " elements");
					for (int j=0;j<layer.get_elements_count();j++)
					{
						inki.UIElement el = layer.resolve_elements(j);
						Console.WriteLine("    element[" + j + "] = " + el);
						if (el != null)
						{
							Console.WriteLine("    => type = " + el.m_mi.GetTypeName());
							inki.UIRect lr = el.get_layout();
							Gdk.Rectangle r = new Gdk.Rectangle((int)lr.get_x(), (int)lr.get_y(), (int)lr.get_width(), (int)lr.get_height());
							GdkWindow.DrawRectangle(g, false, r);
						}
					}

				}
			}




			GdkWindow.DrawLine(g, 0, 0, 100, 100);

			return true;
		}

		protected override void OnSizeAllocated (Gdk.Rectangle allocation)
		{
			base.OnSizeAllocated (allocation);
			// Insert layout code here.
		}

		protected override void OnSizeRequested (ref Gtk.Requisition requisition)
		{
			// Calculate desired size here.
			requisition.Height = 50;
			requisition.Width = 50;
		}
	}
}

