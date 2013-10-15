using System;

namespace ccguiputkedplugin
{
	public partial class WidgetEditorWindow : Gtk.Window
	{
		public WidgetEditorWindow () : 
				base(Gtk.WindowType.Toplevel)
		{
			this.Build ();
		}
	}
}

