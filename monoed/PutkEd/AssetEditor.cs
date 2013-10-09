using System;
using Gtk;

namespace PutkEd
{
	public partial class AssetEditor : Gtk.Window
	{
		DLLLoader.MemInstance m_mi;

		public AssetEditor(DLLLoader.MemInstance instance) : base(Gtk.WindowType.Toplevel)
		{
			m_mi = instance;
			this.Build();

			this.Title = m_mi.GetTypeName() + " editor";

			
			// Path
			Gtk.CellRendererText r0 = new Gtk.CellRendererText();
			TreeViewColumn c0 = new TreeViewColumn("Field", r0);
			c0.AddAttribute(r0, "text", 0);
			m_props.InsertColumn (c0, 0);

			// Type
			Gtk.CellRendererText r1 = new Gtk.CellRendererText();
			TreeViewColumn c1 = new TreeViewColumn("Value", r1);
			c1.AddAttribute(r1, "text", 1);
			m_props.InsertColumn (c1, 1);

			Gtk.ListStore ls = new Gtk.ListStore (typeof(string), typeof(string));


			for (int i=0;i<10000;i++)
			{
				DLLLoader.PutkiField pf = m_mi.GetField(i);
				if (pf != null)
				{
					ls.AppendValues(pf.GetName(), "korv");
				}
			}

			m_props.Model = ls;
		}
	}
}
