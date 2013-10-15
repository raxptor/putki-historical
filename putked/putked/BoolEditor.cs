using System;
using System.Collections.Generic;
using Gtk;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class BoolEditor : Gtk.Bin, TypeEditor
	{
		public int m_arrayIndex;

		public BoolEditor()
		{
			this.Build();
		}

		public Widget GetRoot()
		{
			return this;
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int arrayIndex)
		{
			m_arrayIndex = arrayIndex;

			fi.SetArrayIndex(m_arrayIndex);
			Console.WriteLine("Iti si " + fi.GetBool(mi));
			m_checkbox.Active = fi.GetBool(mi);
			m_checkbox.Clicked += delegate(object sender, EventArgs e)
			{
				fi.SetArrayIndex(m_arrayIndex);
				fi.SetBool(mi, m_checkbox.Active);
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

