using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class FloatEditor : Gtk.Bin, TypeEditor
	{
		public FloatEditor()
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
			m_tbox.Text = fi.GetFloat(mi).ToString();
			m_tbox.Changed += delegate {
				float o;
				fi.SetArrayIndex(arrayIndex);
				if (Single.TryParse(m_tbox.Text, out o))
				{
					fi.SetFloat(mi, o);
					m_tbox.ModifyBase(StateType.Normal);
				}
				else
				{
					m_tbox.ModifyBase(StateType.Normal, new Gdk.Color(200, 10, 10));
				}
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

