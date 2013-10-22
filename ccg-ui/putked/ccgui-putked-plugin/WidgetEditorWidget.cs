using System;
using PutkEd;
using System.Collections.Generic;

namespace ccguiputkedplugin
{
	[System.ComponentModel.ToolboxItem (true)]
	public partial class WidgetEditorWidget : Gtk.Bin
	{
		inki.UIWidget m_iWidget;

		List<DLLLoader.PutkiField> m_fields = null;

		public WidgetEditorWidget(DLLLoader.MemInstance mi) : base()
		{
			// Save both original & miWidget.
			// In putki these will be same pointers, really, but 
			// field set will be different!
			m_iWidget = DataHelper.CreatePutkEdObj(mi) as inki.UIWidget;

			this.Build ();

			m_title.Text = mi.GetPath();

			WidgetViewer wv = new WidgetViewer(m_iWidget);
			m_vbox.Add(wv);

			wv.SetSizeRequest((int)m_iWidget.get_width(), (int)m_iWidget.get_height());
		}
	}
}

