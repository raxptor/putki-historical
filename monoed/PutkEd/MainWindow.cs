using System;
using Gtk;

public partial class MainWindow : Gtk.Window
{	

	public class Item
	{
		public string Path;
	}

	public MainWindow() : base (Gtk.WindowType.Toplevel)
	{
		Build();

		// Path
		Gtk.CellRendererText r0 = new Gtk.CellRendererText();
		TreeViewColumn c0 = new TreeViewColumn("Path", r0);
		c0.AddAttribute(r0, "text", 0);
		m_fileTree.InsertColumn (c0, 0);
		
		// Type
		Gtk.CellRendererText r1 = new Gtk.CellRendererText();
		TreeViewColumn c1 = new TreeViewColumn("Type", r1);
		c1.AddAttribute(r1, "text", 1);
		m_fileTree.InsertColumn (c1, 1);

		Gtk.ListStore ls = new Gtk.ListStore (typeof(string), typeof(string));

		foreach (PutkEd.FileIndex.Entry e in PutkEd.MainClass.s_fileIndex.GetAssets())
		{
			ls.AppendValues(e.AssetName, e.DisplayType);
		}
		
		m_fileTree.Model = ls;

		ShowAll();
	}

	protected void OnDeleteEvent(object sender, DeleteEventArgs a)
	{
		Application.Quit();
		a.RetVal = true;
	}

	protected void OnEditAsset (object sender, EventArgs e)
	{
		TreeModel model;
		TreeIter iter;

		if (m_fileTree.Selection.GetSelected(out model, out iter))
		{		
			string Path = model.GetValue(iter, 0).ToString();
			PutkEd.DLLLoader.MemInstance mi = PutkEd.MainClass.s_dataDll.DiskLoad(Path);
			if (mi != null)
			{
				PutkEd.AssetEditor ae = new PutkEd.AssetEditor(mi);
				ae.Show();
			}
		}
	}
}
