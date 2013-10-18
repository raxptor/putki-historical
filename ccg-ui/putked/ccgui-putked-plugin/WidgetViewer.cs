using System;
using PutkEd;

namespace ccguiputkedplugin
{
	[System.ComponentModel.ToolboxItem (true)]
	public class WidgetViewer : Gtk.DrawingArea
	{
		DLLLoader.MemInstance m_mi;

		public WidgetViewer(DLLLoader.MemInstance mi)
		{
			m_mi = mi;
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

