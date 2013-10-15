using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class EnumEditor : Gtk.Bin, TypeEditor
	{
		public EnumEditor()
		{
			this.Build();
		}

		public Widget GetRoot()
		{
			return this;
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int arrayIndex)
		{
			fi.SetArrayIndex(arrayIndex);

			string val = fi.GetEnum(mi);
			for (int i=0;;i++)
			{
				String s = fi.GetEnumPossibility(i);
				if (s == null)
					break;
				m_combo.AppendText(s);

				if (val == s)
				{
					TreeIter it;
					if (m_combo.Model.IterNthChild(out it, i))
						m_combo.SetActiveIter(it);
				}
			}

			m_combo.Changed += delegate
			{
				fi.SetArrayIndex(arrayIndex);
				fi.SetEnum(mi, m_combo.ActiveText);
			};
		}

		public List<AssetEditor.RowNode> GetChildRows()
		{
			return new List<AssetEditor.RowNode>();
		}

		public void OnConnect(AssetEditor root)
		{
		}
	}
}

