
// This file has been generated by the GUI designer. Do not modify.
namespace PutkEd
{
	public partial class AssetEditor
	{
		private global::Gtk.VBox vbox1;
		
		private global::Gtk.ScrolledWindow scrolledwindow1;
		
		private global::Gtk.Fixed m_propEd;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget PutkEd.AssetEditor
			global::Stetic.BinContainer.Attach (this);
			this.WidthRequest = 850;
			this.Name = "PutkEd.AssetEditor";
			// Container child PutkEd.AssetEditor.Gtk.Container+ContainerChild
			this.vbox1 = new global::Gtk.VBox ();
			this.vbox1.Name = "vbox1";
			this.vbox1.Spacing = 6;
			// Container child vbox1.Gtk.Box+BoxChild
			this.scrolledwindow1 = new global::Gtk.ScrolledWindow ();
			this.scrolledwindow1.CanFocus = true;
			this.scrolledwindow1.Name = "scrolledwindow1";
			this.scrolledwindow1.ShadowType = ((global::Gtk.ShadowType)(1));
			// Container child scrolledwindow1.Gtk.Container+ContainerChild
			global::Gtk.Viewport w1 = new global::Gtk.Viewport ();
			w1.ShadowType = ((global::Gtk.ShadowType)(0));
			// Container child GtkViewport.Gtk.Container+ContainerChild
			this.m_propEd = new global::Gtk.Fixed ();
			this.m_propEd.Name = "m_propEd";
			this.m_propEd.HasWindow = false;
			w1.Add (this.m_propEd);
			this.scrolledwindow1.Add (w1);
			this.vbox1.Add (this.scrolledwindow1);
			global::Gtk.Box.BoxChild w4 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.scrolledwindow1]));
			w4.Position = 0;
			this.Add (this.vbox1);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Show ();
		}
	}
}
