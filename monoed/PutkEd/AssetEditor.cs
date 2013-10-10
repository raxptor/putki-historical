using System;
using System.Collections.Generic;
using Gtk;

namespace PutkEd
{
	public partial class AssetEditor : Gtk.Window
	{
		ObjectEditor m_rootObj;
		List<RowNode> m_tree = new List<RowNode>();

		public class RowNode
		{
			public DLLLoader.MemInstance mi;
			public DLLLoader.PutkiField fh;
			public TypeEditor editor;

			public List<RowNode> children = new List<RowNode>();
			public int arrayIndex = -1;
		}

		public AssetEditor() : base (Gtk.WindowType.Toplevel)
		{
			Build();
			ShowAll();
		}

		public void SetObject(DLLLoader.MemInstance mi)
		{
			// m_label.Visibility = Visibility.Collapsed; // nly for structs.
			m_rootObj = new ObjectEditor();
			m_rootObj.SetObject(mi, null, 0);

			OnStructureChanged();
			this.Title = mi.GetTypeName() + " editor";
		}

		public void OnStructureChanged()
		{
			m_tree = m_rootObj.GetChildRows();
			int rows = Layout(m_tree, 0, 0) + 1;

			this.SetSizeRequest(Looks.PropEdWidth, rows * Looks.FieldHeight + 100);

			ShowAll();
		}

		public int Layout(List<RowNode> list, int rowIndex, int indent)
		{
			Console.WriteLine("Layouting " + list.Count + " items row " + rowIndex);
			int idx = 0;
			foreach (RowNode rn in list)
			{
				if (rn.fh != null && rn.fh.GetName() == "parent")
				{
					rowIndex = Layout(rn.children, rowIndex, indent);
					continue;
				}

				int y0 = 5 + rowIndex * Looks.FieldHeight;

				Label name = new Label();

				if (rn.fh != null)
				{
					if (rn.fh.IsArray() && !(rn.editor is ArrayEditor))
						name.Text = rn.fh.GetName() + "[" + (idx++) + "]";
					else
						name.Text = rn.fh.GetName();
				}
				else
				{
					name.Text = rn.mi.GetPath();
				}

				m_propEd.Put(name, 10 + indent * Looks.IndentWidth, y0);

				// 
				rn.editor.GetRoot().SetSizeRequest(Looks.PropEdWidth - 180, Looks.FieldHeight);
				m_propEd.Put(rn.editor.GetRoot(), 95, y0);

				// If is is array entry
				if (rn.fh != null && rn.fh.IsArray())
				{
					Button b = new Button();

					if (rn.arrayIndex == -1)
					{
						b.Label = "Add";
						//						b.Click += delegate { InsertClick(rn); };
					}
					else
					{
						b.Label = "Del";
						//						b.Click += delegate { EraseClick(rn); };
					}

					b.SetSizeRequest(70, Looks.FieldHeight);
					m_propEd.Put(b, Looks.PropEdWidth - 80, y0);
				}

				rn.editor.OnConnect(this);
				rowIndex = Layout(rn.children, rowIndex + 1, indent + 1);
			}
			return rowIndex;
		}

	}


}
