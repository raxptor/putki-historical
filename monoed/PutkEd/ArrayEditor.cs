using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	public partial class ArrayEditor : Gtk.Bin, TypeEditor
	{
		DLLLoader.MemInstance m_mi;
		DLLLoader.PutkiField m_fh;

		public ArrayEditor()
		{
			this.Build();
		}

		public Widget GetRoot()
		{
			return this;
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int arrayIndex)
		{
			m_mi = mi;
			m_fh = fi;
			m_label.Text = "Array (" + 444 /*fi.GetArraySize(mi)*/ + " items)";
		}

		public void OnConnect(AssetEditor root)
		{

		}

		public List<AssetEditor.RowNode> GetChildRows()
		{
			List<AssetEditor.RowNode> list = new List<AssetEditor.RowNode>();
			int entries = 0; // m_fh.GetArraySize(m_mi);
			for (int i = 0; i < entries; i++)
			{
				AssetEditor.RowNode e = new AssetEditor.RowNode();
				e.mi = m_mi;
				e.fh = m_fh;
				e.editor = EditorCreator.MakeEditor(m_fh, false);
				e.editor.SetObject(m_mi, m_fh, i);
				e.children = e.editor.GetChildRows();
				e.arrayIndex = i;
				list.Add(e);
			}
			return list;
		}
	}
	}

