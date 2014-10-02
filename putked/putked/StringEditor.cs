using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class StringEditor : Gtk.Bin, TypeEditor
	{
		public StringEditor()
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
			m_text.Text = fi.GetString(mi);
			m_text.Changed += delegate {
				fi.SetArrayIndex(arrayIndex);
				fi.SetString(mi, m_text.Text);
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

