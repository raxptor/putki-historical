using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class TextFieldEditor : Gtk.Bin, TypeEditor
	{
		public TextEditor m_editor;

		public TextFieldEditor()
		{
			this.Build();
		}

		public Widget GetRoot()
		{
			return this;
		}

		public void OnConnect(AssetEditor root)
		{

		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int arrayIndex)
		{
			m_openEditor.Clicked += delegate
			{
				if (m_editor != null)
				{
					m_editor.Destroy();
				}

				m_editor = new TextEditor();
				fi.SetArrayIndex(arrayIndex);

				m_editor.SetObject(fi.GetStructInstance(mi), fi.GetStructInstance(mi).GetField(0), arrayIndex);
				m_editor.Present();
			};
		}

		public List<AssetEditor.RowNode> GetChildRows()
		{
			return new List<AssetEditor.RowNode>();
		}
	}
}

