using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class Vec4Editor : Gtk.Bin, TypeEditor
	{
		DLLLoader.MemInstance m_mi;
		DLLLoader.PutkiField[] m_fh = {null, null, null, null};
		Entry[] m_tb = {null, null, null, null};
		Label[] m_lb = {null, null, null, null};
		int m_idx;

		public Vec4Editor()
		{
			this.Build();

			m_tb[0] = m_eX;
			m_tb[1] = m_eY;
			m_tb[2] = m_eZ;
			m_tb[3] = m_eW;
			m_lb[0] = m_lX;
			m_lb[1] = m_lY;
			m_lb[2] = m_lZ;
			m_lb[3] = m_lW;
		}

		public void OnConnect(AssetEditor root)
		{

		}

		public Widget GetRoot()
		{
			return this;
		}

		public void Bind(int i)
		{
			m_lb[i].Text = m_fh[i].GetName();

			if (m_fh[i].GetFieldType() == 7) // float
				m_tb[i].Text = m_fh[i].GetFloat(m_mi).ToString();
			else if ((int)m_fh[i].GetFieldType() == 1) // byte
				m_tb[i].Text = m_fh[i].GetByte(m_mi).ToString();

			m_tb[i].Changed += delegate
			{
				if (m_fh[i].GetFieldType() == 7) // float
				{
					float o;
					if (Single.TryParse(m_tb[i].Text, out o))
					{
						m_fh[i].SetFloat(m_mi, o);
						m_tb[i].ModifyBase(StateType.Normal);
					}
					else
					{
						m_tb[i].ModifyBase(StateType.Normal, new Gdk.Color(200, 10, 10));
					}
				}
				if (m_fh[i].GetFieldType() == 1) // byte
				{
					byte o;
					if (Byte.TryParse(m_tb[i].Text, out o))
					{
						m_fh[i].SetByte(m_mi, o);
						m_tb[i].ModifyBase(StateType.Normal);
					}
					else
					{
						m_tb[i].ModifyBase(StateType.Normal, new Gdk.Color(200, 10, 10));
					}
				}

			};
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField  fi, int arrayIndex)
		{
			fi.SetArrayIndex(arrayIndex);
			m_mi = fi.GetStructInstance(mi);

			for (int i=0;i<4;i++)
			{
				m_fh[i] = m_mi.GetField(i);
				Bind(i);
			}

			m_idx = arrayIndex;
		}

		public List<AssetEditor.RowNode> GetChildRows()
		{
			return new List<AssetEditor.RowNode>();
		}
	}
}

