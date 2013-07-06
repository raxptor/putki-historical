using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor
{
    /// <summary>
    /// Interaction logic for Vec4Editor.xaml
    /// </summary>
    public partial class Vec4Editor : UserControl, TypeEditor
    {
        int m_idx = -1;
        Brush m_obrush;
		Putki.MemInstance m_mi;
		Putki.FieldHandler[] m_fh = {null,null,null,null};
		TextBox[] m_tb = { null, null, null, null };
		Label[] m_lb = { null, null, null, null };

        public Vec4Editor()
        {
            InitializeComponent();
            m_obrush = m_tbox_x.Foreground;
        }

        public Control GetRoot()
        {
            return this;
        }

        public void OnConnect(ObjectEditor root)
        {

        }

		public void Bind(int i)
		{
			m_lb[i].Content = m_fh[i].GetName();

			if ((int)m_fh[i].GetType() == 7) // float
				m_tb[i].Text = m_fh[i].GetFloat(m_mi).ToString();
			else if ((int)m_fh[i].GetType() == 1) // byte
				m_tb[i].Text = m_fh[i].GetByte(m_mi).ToString();

			m_tb[i].TextChanged += delegate
			{
				if ((int)m_fh[i].GetType() == 7) // float
				{
					float o;
					if (Single.TryParse(m_tb[i].Text, out o))
					{
						m_fh[i].SetFloat(m_mi, o);
						m_tb[i].Foreground = m_obrush;
					}
					else
					{
						m_tb[i].Foreground = Brushes.Red;
					}
				}
				else if ((int)m_fh[i].GetType() == 1) // byte
				{
					byte o;
					if (Byte.TryParse(m_tb[i].Text, out o))
					{
						m_fh[i].SetByte(m_mi, o);
						m_tb[i].Foreground = m_obrush;
					}
					else
					{
						m_tb[i].Foreground = Brushes.Red;
					}
				}

			};
		}

        public void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int arrayIndex)
        {
            fi.SetArrayIndex(arrayIndex);
			m_mi = fi.GetStructInstance(mi);

			m_tb[0] = m_tbox_x;
			m_tb[1] = m_tbox_y;
			m_tb[2] = m_tbox_z;
			m_tb[3] = m_tbox_w;


			m_lb[0] = m_label_x;
			m_lb[1] = m_label_y;
			m_lb[2] = m_label_z;
			m_lb[3] = m_label_w;

			for (int i=0;i<4;i++)
			{
				string n = m_mi.GetType().GetName();
				string fn = m_mi.GetType().GetField(i).GetName();
				m_fh[i] = m_mi.GetType().GetField(i);
				Bind(i);
			}

/*

			*/
            m_idx = arrayIndex;
        }

        public List<ObjectEditor.RowNode> GetChildRows()
        {
            return new List<ObjectEditor.RowNode>();
        }
    }
}
