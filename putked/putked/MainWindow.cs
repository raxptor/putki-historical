using System;
using System.Collections.Generic;
using PutkEd;
using Gtk;

public partial class MainWindow : Gtk.Window
{	
	public class Item
	{
		public string Path;
	}

	public struct PluginButtons
	{
		public EditorPlugin plugin;
		public Gtk.Button button;
	};


	private Gtk.TreeModelFilter m_modelFilter;
	private List<PluginButtons> m_pluginButtons =new List<PluginButtons>();

	public MainWindow() : base (Gtk.WindowType.Toplevel)
	{
		Build();

		// Path
		Gtk.CellRendererText r0 = new Gtk.CellRendererText();
		TreeViewColumn c0 = new TreeViewColumn("Path", r0);
		c0.AddAttribute(r0, "text", 0);
		c0.MinWidth = 300;
		m_fileTree.InsertColumn (c0, 0);

		// Type
		Gtk.CellRendererText r1 = new Gtk.CellRendererText();
		TreeViewColumn c1 = new TreeViewColumn("Type", r1);
		c1.AddAttribute(r1, "text", 1);
		m_fileTree.InsertColumn (c1, 1);

		foreach (EditorPlugin p in PutkEdMain.s_plugins)
		{
			AddPluginButton(p);
		}

		this.WidthRequest = 800;

		m_searchFilter.Changed += delegate
		{
			m_modelFilter.Refilter();
		};

		m_searchFilter.Activate();

		ScanFiles();

		m_statusText.Text = DLLLoader.GetStatus();
	}


	public void AddPluginButton(EditorPlugin p)
	{
		Button b = new Button(p.GetDescription());
		vbox3.Add(b);
		Gtk.Box.BoxChild bc = (Box.BoxChild)vbox3[b];
		bc.Fill = false;
		bc.Expand = false;
		b.Sensitive = false;

		b.Clicked += delegate(object sender, EventArgs e)
		{
			OpenAssetEditor(p);
		};

		PluginButtons bt;
		bt.plugin = p;
		bt.button = b;
		m_pluginButtons.Add(bt);
	}

	public void ScanFiles()
	{
		PutkEdMain.s_fileIndex.Reload();

		Gtk.ListStore ls = new Gtk.ListStore (typeof(string), typeof(string));
		foreach (FileIndex.Entry e in PutkEdMain.s_fileIndex.GetAssets())
		{
			ls.AppendValues(e.AssetName, e.DisplayType);
		}
			
		m_modelFilter = new Gtk.TreeModelFilter (ls, null);
		m_modelFilter.VisibleFunc = new Gtk.TreeModelFilterVisibleFunc(FilterFiles);
		m_fileTree.Model = m_modelFilter;

		ShowAll();
	}

	private bool FilterFiles(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		if (m_searchFilter.Text == "")
			return true;

		string fn = model.GetValue(iter, 0).ToString();
		string type = model.GetValue(iter, 1).ToString();

		return fn.IndexOf(m_searchFilter.Text) > -1 || type.IndexOf(m_searchFilter.Text) > -1;
	}

	protected void OnDeleteEvent(object sender, DeleteEventArgs a)
	{
		Application.Quit();
		a.RetVal = true;
	}

	private void OpenAssetEditor(DLLLoader.MemInstance mi, EditorPlugin plugin)
	{
		if (mi != null)
		{
			if (plugin == null)
			{
				AssetEditor ae = new AssetEditor();
				ae.SetObject(mi);
				ae.Show();
			}
			else
			{
				if (plugin.CanEditType(mi.GetPutkiType()))
					plugin.LaunchEditor(mi);
			}
		}
	}

	private void OpenAssetEditor(EditorPlugin plugin)
	{
		TreeModel model;
		TreeIter iter;

		if (m_fileTree.Selection.GetSelected(out model, out iter))
		{		
			string Path = model.GetValue(iter, 0).ToString();
			DLLLoader.MemInstance mi = DLLLoader.MemInstance.Load(Path);
			OpenAssetEditor(mi, plugin);
		}
	}

	// Clicked edit button
	protected void OnEditAsset (object sender, EventArgs e)
	{
		OpenAssetEditor(null);
	}

	protected void OnNewAsset (object sender, EventArgs e)
	{
		NewAssetDialog nad = new NewAssetDialog();
		if (nad.Run() == 0)
		{
			ScanFiles();
		}
		nad.Destroy();
	}

	protected void OnRowActivated (object o, RowActivatedArgs args)
	{
		OnEditAsset(o, args);
	}

	protected void OnReloadIndex(object sender, EventArgs e)
	{
		ScanFiles();
	}

	protected void OnCursorChanged (object sender, EventArgs e)
	{
		DLLLoader.MemInstance mi = null;
		TreeModel model;
		TreeIter iter;
		if (m_fileTree.Selection.GetSelected(out model, out iter))
		{		
			string Path = model.GetValue(iter, 0).ToString();
			mi = DLLLoader.MemInstance.Load(Path);
		}

		foreach (PluginButtons pb in m_pluginButtons)
		{
			pb.button.Sensitive = (mi != null && pb.plugin.CanEditType(mi.GetPutkiType()));
		}
	}

}
