
// This file has been generated by the GUI designer. Do not modify.
namespace PutkEd
{
	public partial class Vec4Editor
	{
		private global::Gtk.HBox hbox2;
		private global::Gtk.Label m_lX;
		private global::Gtk.Entry m_eX;
		private global::Gtk.Label m_lY;
		private global::Gtk.Entry m_eY;
		private global::Gtk.Label m_lZ;
		private global::Gtk.Entry m_eZ;
		private global::Gtk.Label m_lW;
		private global::Gtk.Entry m_eW;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget PutkEd.Vec4Editor
			global::Stetic.BinContainer.Attach (this);
			this.Name = "PutkEd.Vec4Editor";
			// Container child PutkEd.Vec4Editor.Gtk.Container+ContainerChild
			this.hbox2 = new global::Gtk.HBox ();
			this.hbox2.Name = "hbox2";
			this.hbox2.Spacing = 6;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_lX = new global::Gtk.Label ();
			this.m_lX.Name = "m_lX";
			this.m_lX.LabelProp = global::Mono.Unix.Catalog.GetString ("label1");
			this.hbox2.Add (this.m_lX);
			global::Gtk.Box.BoxChild w1 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_lX]));
			w1.Position = 0;
			w1.Expand = false;
			w1.Fill = false;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_eX = new global::Gtk.Entry ();
			this.m_eX.CanFocus = true;
			this.m_eX.Name = "m_eX";
			this.m_eX.IsEditable = true;
			this.m_eX.InvisibleChar = '●';
			this.hbox2.Add (this.m_eX);
			global::Gtk.Box.BoxChild w2 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_eX]));
			w2.Position = 1;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_lY = new global::Gtk.Label ();
			this.m_lY.Name = "m_lY";
			this.m_lY.LabelProp = global::Mono.Unix.Catalog.GetString ("label2");
			this.hbox2.Add (this.m_lY);
			global::Gtk.Box.BoxChild w3 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_lY]));
			w3.Position = 2;
			w3.Expand = false;
			w3.Fill = false;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_eY = new global::Gtk.Entry ();
			this.m_eY.CanFocus = true;
			this.m_eY.Name = "m_eY";
			this.m_eY.IsEditable = true;
			this.m_eY.InvisibleChar = '●';
			this.hbox2.Add (this.m_eY);
			global::Gtk.Box.BoxChild w4 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_eY]));
			w4.Position = 3;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_lZ = new global::Gtk.Label ();
			this.m_lZ.Name = "m_lZ";
			this.m_lZ.LabelProp = global::Mono.Unix.Catalog.GetString ("label3");
			this.hbox2.Add (this.m_lZ);
			global::Gtk.Box.BoxChild w5 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_lZ]));
			w5.Position = 4;
			w5.Expand = false;
			w5.Fill = false;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_eZ = new global::Gtk.Entry ();
			this.m_eZ.CanFocus = true;
			this.m_eZ.Name = "m_eZ";
			this.m_eZ.IsEditable = true;
			this.m_eZ.InvisibleChar = '●';
			this.hbox2.Add (this.m_eZ);
			global::Gtk.Box.BoxChild w6 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_eZ]));
			w6.Position = 5;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_lW = new global::Gtk.Label ();
			this.m_lW.Name = "m_lW";
			this.m_lW.LabelProp = global::Mono.Unix.Catalog.GetString ("label4");
			this.hbox2.Add (this.m_lW);
			global::Gtk.Box.BoxChild w7 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_lW]));
			w7.Position = 6;
			w7.Expand = false;
			w7.Fill = false;
			// Container child hbox2.Gtk.Box+BoxChild
			this.m_eW = new global::Gtk.Entry ();
			this.m_eW.CanFocus = true;
			this.m_eW.Name = "m_eW";
			this.m_eW.IsEditable = true;
			this.m_eW.InvisibleChar = '●';
			this.hbox2.Add (this.m_eW);
			global::Gtk.Box.BoxChild w8 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.m_eW]));
			w8.Position = 7;
			this.Add (this.hbox2);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}
