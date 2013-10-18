using System;
using PutkEd;

namespace ccguiputkedplugin
{
	public partial class WidgetEditWindow : Gtk.Window
	{
		public WidgetEditWindow(DLLLoader.MemInstance mi) : base(Gtk.WindowType.Toplevel)
		{
			this.Build();
			Add(new WidgetEditorWidget(mi));
			ShowAll();
		}
	}
}

