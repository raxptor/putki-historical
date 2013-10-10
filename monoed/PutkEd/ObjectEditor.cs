using System;
using System.Collections.Generic;
using Gtk;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class ObjectEditor : Gtk.Bin, TypeEditor
	{
		DLLLoader.MemInstance m_obj;
		public int m_arrayIndex = 0;

		public ObjectEditor()
		{
			this.Build();
		}

		public void OnConnect(AssetEditor root)
		{

		}

		public Widget GetRoot()
		{
			return this;
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int arrayIndex)
		{
			if (fi != null)
			{
				// member somewhere.
				// fi.SetArrayIndex(arrayIndex);
				m_obj = fi.GetStructInstance(mi);
			}
			else
			{
				m_obj = mi;
			}

			m_arrayIndex = arrayIndex;
		}

		public List<AssetEditor.RowNode> GetChildRows()
		{
			int f = 0;            
			List<AssetEditor.RowNode> l = new List<AssetEditor.RowNode>();
			while (true)
			{
				DLLLoader.PutkiField fh = m_obj.GetField(f);
				if (fh != null)
				{
					// if (fh.ShowInEditor())
					{
						AssetEditor.RowNode e = new AssetEditor.RowNode();
						e.mi = m_obj;
						e.fh = fh;
						e.editor = EditorCreator.MakeEditor(fh, fh.IsArray());
						e.editor.SetObject(m_obj, fh, 0);
						e.children = e.editor.GetChildRows();
						l.Add(e);
					}
					f++;
				}
				else
				{
					break;
				}
			}
			return l;
		}
	}
}

