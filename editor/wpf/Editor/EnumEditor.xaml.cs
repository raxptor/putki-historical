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
    public partial class EnumEditor : UserControl, TypeEditor
    {
        int m_idx;
        Putki.FieldHandler m_fh;
        Putki.MemInstance m_mi;

		public EnumEditor()
        {
            InitializeComponent();
        }

        public Control GetRoot()
        {
            return this;
        }

        public void OnConnect(ObjectEditor root)
        {
            // m_root = root;
        }

        public void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int arrayIndex)
        {
            m_fh = fi;
            m_mi = mi;
			m_idx = arrayIndex;
            fi.SetArrayIndex(arrayIndex);

			string val = m_fh.GetEnum(mi);
			for (int i=0;;i++)
			{
				String s = m_fh.GetEnumValueByIndex(i);
				if (s == null)
					break;
				if (s == val)
					m_combo.SelectedIndex = i;
				m_combo.Items.Add(s);
			}

			m_combo.SelectionChanged += delegate
			{
				m_fh.SetEnum(m_mi, m_fh.GetEnumValueByIndex(m_combo.SelectedIndex));
			};
        }

		public List<ObjectEditor.RowNode> GetChildRows()
        {
            return new List<ObjectEditor.RowNode>();
        }
    }
}
