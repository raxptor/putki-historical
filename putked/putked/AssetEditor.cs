using System;
using System.Collections.Generic;
using Gtk;

namespace PutkEd
{
	public partial class AssetEditor : Gtk.Bin
	{
		ObjectEditor m_rootObj;
		List<RowNode> m_tree = new List<RowNode>();
		List<Widget> m_destroyList = new List<Widget>();

		public class RowNode
		{
			public DLLLoader.MemInstance mi;
			public DLLLoader.PutkiField fh;
			public TypeEditor editor;

			public List<RowNode> children = new List<RowNode>();
			public int arrayIndex = -1;
			public bool expanded = false;
		}

		public AssetEditor()
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
		}

		public void OnStructureChanged()
		{
			foreach (Widget w in m_destroyList)
				w.Destroy();
			m_destroyList.Clear();

			m_tree = m_rootObj.GetChildRows();
			LayoutAll();
		}

		public void LayoutAll()
		{
			foreach (Widget w in m_destroyList)
				m_propEd.Remove(w);

			m_destroyList.Clear();

			int rows = Layout(m_tree, 0, 0) + 1;

			int reqSize = rows * Looks.FieldHeight + 100;
			if (reqSize > 600)
				reqSize = 600;

			this.SetSizeRequest(Looks.PropEdWidth, reqSize);

			ShowAll();
		}

		public int Layout(List<RowNode> list, int rowIndex, int indent)
		{
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
					name.Text = "";//rn.mi.GetPath();
				}

				m_propEd.Put(name, 10 + indent * Looks.IndentWidth, y0 + 4);
				m_destroyList.Add(name);

				// 
				rn.editor.GetRoot().SetSizeRequest(Looks.PropEdWidth - 260, Looks.FieldHeight - 2);
				m_propEd.Put(rn.editor.GetRoot(), 200, y0 + 1);
				m_destroyList.Add(rn.editor.GetRoot());

				// If is is array entry
				if (rn.fh != null && rn.fh.IsArray())
				{
					Button b = new Button();

					if (rn.arrayIndex == -1)
					{
						b.Label = "+";
						b.Clicked += delegate { InsertClick(rn); };
					}
					else
					{
						b.Label = "-";
						b.Clicked += delegate { EraseClick(rn); };
					}

					b.SetSizeRequest(30, Looks.FieldHeight - 2);
					m_propEd.Put(b, Looks.PropEdWidth - 50, y0 + 1);
					m_destroyList.Add(b);
				}

				rn.editor.OnConnect(this);
				rowIndex = Layout(rn.children, rowIndex + 1, indent + 1);
			}
			return rowIndex;
		}

		public void InsertClick(RowNode node)
		{
			node.fh.SetArrayIndex(0);
			node.fh.ArrayInsert(node.mi);
			OnStructureChanged();
		}

		public void EraseClick(RowNode node)
		{
			node.fh.SetArrayIndex(node.arrayIndex);
			node.fh.ArrayErase(node.mi);
			OnStructureChanged();
		}
	}


}
