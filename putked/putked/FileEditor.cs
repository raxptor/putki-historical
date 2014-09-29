using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class FileEditor : Gtk.Bin, TypeEditor
	{
		public FileEditor()
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
			m_text.Changed += delegate 
			{
				fi.SetArrayIndex(arrayIndex);
				fi.SetString(mi, m_text.Text);
				UpdateColor(fi.GetString(mi));


			};
			UpdateColor(m_text.Text);
		}

		public void UpdateColor(string path)
		{
			if (!System.IO.File.Exists(PutkEdMain.s_resPath + "/" + path))
			{
				m_text.ModifyBase(StateType.Normal, new Gdk.Color(200, 10, 10));
			}
			else
			{
				m_text.ModifyBase(StateType.Normal);
			}
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

