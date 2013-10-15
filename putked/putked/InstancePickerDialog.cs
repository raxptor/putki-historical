using Gtk;
using System;
using System.Collections.Generic;

namespace PutkEd
{
	public partial class InstancePickerDialog : Gtk.Dialog
	{
		public string m_selectedPath;

		public InstancePickerDialog()
		{
			this.Build();
		}

		public void AddForTypes(string TypeName)
		{
			DLLLoader.Types want = null;
			foreach (DLLLoader.Types d in PutkEdMain.s_dataDll.GetTypes())
			{
				if (TypeName == d.Name)
					want = d;
			}

			// Path
			Gtk.CellRendererText r0 = new Gtk.CellRendererText();
			TreeViewColumn c0 = new TreeViewColumn("Path", r0);
			c0.AddAttribute(r0, "text", 0);
			m_assets.InsertColumn (c0, 0);

			// Type
			Gtk.CellRendererText r1 = new Gtk.CellRendererText();
			TreeViewColumn c1 = new TreeViewColumn("Type", r1);
			c1.AddAttribute(r1, "text", 1);
			m_assets.InsertColumn (c1, 1);

			Gtk.ListStore ls = new Gtk.ListStore (typeof(string), typeof(string));

			foreach (PutkEd.FileIndex.Entry entry in PutkEdMain.s_fileIndex.GetAssets())
			{
				foreach (DLLLoader.Types e in PutkEdMain.s_dataDll.GetTypes())
				{
					if (entry.DisplayType == e.Name && DLLLoader.HasParent(e, want))
					{
						ls.AppendValues(entry.AssetName, entry.DisplayType);
						break;
					}
				}
			}

			m_assets.Model = ls;
		}

		protected void OnOK (object sender, EventArgs e)
		{
			OnRowActivated(null, null);
		}

		protected void OnRowActivated (object o, RowActivatedArgs args)
		{
			TreeModel model;
			TreeIter iter;
			if (m_assets.Selection.GetSelected(out model, out iter))
			{	
				m_selectedPath = model.GetValue(iter, 0).ToString();
				Respond(0);
			}
		}
	}
}

