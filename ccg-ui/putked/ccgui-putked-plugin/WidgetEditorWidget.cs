using System;
using PutkEd;
using System.Collections.Generic;

namespace ccguiputkedplugin
{
	[System.ComponentModel.ToolboxItem (true)]
	public partial class WidgetEditorWidget : Gtk.Bin
	{
		DLLLoader.MemInstance m_miOriginal;
		DLLLoader.MemInstance m_miWidget;

		List<DLLLoader.PutkiField> m_fields = null;

		public WidgetEditorWidget(DLLLoader.MemInstance mi) : base()
		{
			// Save both original & miWidget.
			// In putki these will be same pointers, really, but 
			// field set will be different!
			m_miOriginal = mi;
			m_miWidget = DataHelper.CastUp(mi, DLLLoader.GetTypeByName("UIWidget"));

			this.Build ();

			m_title.Text = mi.GetPath();

			WidgetViewer wv = new WidgetViewer(mi);
			m_vbox.Add(wv);

			object w = DataHelper.Get(m_miWidget, "width");
			object h = DataHelper.Get(m_miWidget, "height");

			Console.WriteLine("Widget is " + w + "x" + h);

			wv.SetSizeRequest(Convert.ToInt32(w), Convert.ToInt32(h));
		}
	}
}

