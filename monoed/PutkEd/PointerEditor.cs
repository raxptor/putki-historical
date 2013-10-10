using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class PointerEditor : Gtk.Bin, TypeEditor
	{
		int m_idx;
		DLLLoader.MemInstance m_mi, m_ptr_target;
		DLLLoader.PutkiField m_fh;

		public PointerEditor()
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
			m_idx = arrayIndex;

			fi.SetArrayIndex(arrayIndex);

			String r = fi.GetPointer(mi);
			m_ptr_target = DLLLoader.MemInstance.LoadFromDisk(r);
			if (m_ptr_target == null)
			{
				m_tbox.ModifyBase(StateType.Normal, new Gdk.Color(200, 10, 10));
			}
			else
			{
				m_tbox.ModifyBase(StateType.Normal);
			}

			m_tbox.Text = r;
		}

		public List<AssetEditor.RowNode> GetChildRows()
		{
			List<AssetEditor.RowNode> rows = new List<AssetEditor.RowNode>();
			if (m_fh.IsAuxPtr())
			{
				if (m_ptr_target != null)
				{
					AssetEditor.RowNode e = new AssetEditor.RowNode();
					e.mi = m_ptr_target;
					e.fh = null;
					e.editor = new ObjectEditor();
					e.editor.SetObject(m_ptr_target, null, 0);
					e.children = e.editor.GetChildRows();
					rows.Add(e);
				}
			}
			return rows;
		}

		public void OnConnect(AssetEditor root)
		{
		}

	}
}

