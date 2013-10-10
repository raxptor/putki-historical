using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class IntEditor : Gtk.Bin, TypeEditor
	{
		public IntEditor()
		{
			this.Build();
		}

		public Widget GetRoot()
		{
			return this;
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int araryIndex)
		{
			m_tbox.Text = fi.GetInt32(mi).ToString();
			m_tbox.Changed += delegate {
				int o;
				if (Int32.TryParse(m_tbox.Text, out o))
				{
					m_tbox.ModifyBg(StateType.Normal, new Gdk.Color(222,22,22));
					Console.WriteLine("ok");
				}
				else
				{
					m_tbox.ResetRcStyles();
					Console.WriteLine("bad");
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

