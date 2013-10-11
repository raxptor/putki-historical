
// This file has been generated by the GUI designer. Do not modify.
public partial class MainWindow
{
	private global::Gtk.HBox hbox2;
	private global::Gtk.VBox vbox4;
	private global::Gtk.ScrolledWindow GtkScrolledWindow;
	private global::Gtk.TreeView m_fileTree;
	private global::Gtk.HBox hbox6;
	private global::Gtk.Button button7;
	private global::Gtk.Button button1;
	private global::Gtk.VBox vbox3;
	private global::Gtk.Button button3;

	protected virtual void Build ()
	{
		global::Stetic.Gui.Initialize (this);
		// Widget MainWindow
		this.Name = "MainWindow";
		this.Title = global::Mono.Unix.Catalog.GetString ("KraPutki");
		this.WindowPosition = ((global::Gtk.WindowPosition)(4));
		this.BorderWidth = ((uint)(6));
		// Container child MainWindow.Gtk.Container+ContainerChild
		this.hbox2 = new global::Gtk.HBox ();
		this.hbox2.Name = "hbox2";
		this.hbox2.Spacing = 6;
		// Container child hbox2.Gtk.Box+BoxChild
		this.vbox4 = new global::Gtk.VBox ();
		this.vbox4.Name = "vbox4";
		this.vbox4.Spacing = 6;
		// Container child vbox4.Gtk.Box+BoxChild
		this.GtkScrolledWindow = new global::Gtk.ScrolledWindow ();
		this.GtkScrolledWindow.Name = "GtkScrolledWindow";
		this.GtkScrolledWindow.ShadowType = ((global::Gtk.ShadowType)(1));
		// Container child GtkScrolledWindow.Gtk.Container+ContainerChild
		this.m_fileTree = new global::Gtk.TreeView ();
		this.m_fileTree.CanFocus = true;
		this.m_fileTree.Name = "m_fileTree";
		this.m_fileTree.EnableSearch = false;
		this.m_fileTree.SearchColumn = 0;
		this.GtkScrolledWindow.Add (this.m_fileTree);
		this.vbox4.Add (this.GtkScrolledWindow);
		global::Gtk.Box.BoxChild w2 = ((global::Gtk.Box.BoxChild)(this.vbox4 [this.GtkScrolledWindow]));
		w2.Position = 0;
		// Container child vbox4.Gtk.Box+BoxChild
		this.hbox6 = new global::Gtk.HBox ();
		this.hbox6.Name = "hbox6";
		this.hbox6.Spacing = 6;
		// Container child hbox6.Gtk.Box+BoxChild
		this.button7 = new global::Gtk.Button ();
		this.button7.CanFocus = true;
		this.button7.Name = "button7";
		this.button7.UseUnderline = true;
		this.button7.Label = global::Mono.Unix.Catalog.GetString ("Reload Index");
		this.hbox6.Add (this.button7);
		global::Gtk.Box.BoxChild w3 = ((global::Gtk.Box.BoxChild)(this.hbox6 [this.button7]));
		w3.Position = 0;
		w3.Expand = false;
		w3.Fill = false;
		// Container child hbox6.Gtk.Box+BoxChild
		this.button1 = new global::Gtk.Button ();
		this.button1.CanFocus = true;
		this.button1.Name = "button1";
		this.button1.UseUnderline = true;
		this.button1.Label = global::Mono.Unix.Catalog.GetString ("New Asset");
		this.hbox6.Add (this.button1);
		global::Gtk.Box.BoxChild w4 = ((global::Gtk.Box.BoxChild)(this.hbox6 [this.button1]));
		w4.Position = 1;
		w4.Expand = false;
		w4.Fill = false;
		this.vbox4.Add (this.hbox6);
		global::Gtk.Box.BoxChild w5 = ((global::Gtk.Box.BoxChild)(this.vbox4 [this.hbox6]));
		w5.Position = 1;
		w5.Expand = false;
		w5.Fill = false;
		this.hbox2.Add (this.vbox4);
		global::Gtk.Box.BoxChild w6 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.vbox4]));
		w6.Position = 0;
		// Container child hbox2.Gtk.Box+BoxChild
		this.vbox3 = new global::Gtk.VBox ();
		this.vbox3.Name = "vbox3";
		this.vbox3.Spacing = 6;
		// Container child vbox3.Gtk.Box+BoxChild
		this.button3 = new global::Gtk.Button ();
		this.button3.CanFocus = true;
		this.button3.Name = "button3";
		this.button3.UseUnderline = true;
		this.button3.Label = global::Mono.Unix.Catalog.GetString ("Edit");
		this.vbox3.Add (this.button3);
		global::Gtk.Box.BoxChild w7 = ((global::Gtk.Box.BoxChild)(this.vbox3 [this.button3]));
		w7.Position = 0;
		w7.Expand = false;
		w7.Fill = false;
		this.hbox2.Add (this.vbox3);
		global::Gtk.Box.BoxChild w8 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.vbox3]));
		w8.Position = 1;
		w8.Expand = false;
		w8.Fill = false;
		this.Add (this.hbox2);
		if ((this.Child != null)) {
			this.Child.ShowAll ();
		}
		this.DefaultWidth = 416;
		this.DefaultHeight = 472;
		this.Show ();
		this.DeleteEvent += new global::Gtk.DeleteEventHandler (this.OnDeleteEvent);
		this.m_fileTree.RowActivated += new global::Gtk.RowActivatedHandler (this.OnRowActivated);
		this.button7.Clicked += new global::System.EventHandler (this.OnReload);
		this.button1.Clicked += new global::System.EventHandler (this.OnNewAsset);
		this.button3.Clicked += new global::System.EventHandler (this.OnEditAsset);
	}
}
