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
		AssetEditor m_ae;

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
			m_ptr_target = r.Length > 0 ? DLLLoader.MemInstance.Load(r) : null;
			if (m_ptr_target == null)
			{
				m_tbox.ModifyBase(StateType.Insensitive, new Gdk.Color(200, 10, 10));
			}
			else
			{
				m_tbox.ModifyBase(StateType.Insensitive);
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
			m_ae = root;
		}

		protected void OnStarClicked (object sender, EventArgs e)
		{
			if (m_fh.IsAuxPtr())
			{
				NewInstanceDialog nid = new NewInstanceDialog();
				nid.FillWithParentsTo(m_fh.GetRefType());
				if (nid.Run() == 0)
				{
					DLLLoader.MemInstance n = m_mi.CreateAux(nid.m_selectedType);
					m_fh.SetArrayIndex(m_idx);
					m_fh.SetPointer(m_mi, n.GetPath());
					m_ae.OnStructureChanged();
				}
				nid.Destroy();
			}
			else
			{
				InstancePickerDialog ipd = new InstancePickerDialog();
				ipd.AddForTypes(m_fh.GetRefType());
				if (ipd.Run() == 0)
				{
					m_fh.SetArrayIndex(m_idx);
					m_fh.SetPointer(m_mi, ipd.m_selectedPath);
					m_ae.OnStructureChanged();
				}
				ipd.Destroy();
				// Find
			}
		}

		protected void OnClearClicked (object sender, EventArgs e)
		{
			m_fh.SetArrayIndex(m_idx);
			m_fh.SetPointer(m_mi, "");
			m_ae.OnStructureChanged();
		}
	

	}
}

