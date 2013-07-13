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
    /// Interaction logic for PointerEditor.xaml
    /// </summary>
    public partial class PointerEditor : UserControl, TypeEditor
    {
        int m_idx;
        Putki.FieldHandler m_fh;
        Putki.MemInstance m_mi;
        Putki.MemInstance m_ptr_target;
        ObjectEditor m_root;

        public PointerEditor()
        {
            InitializeComponent();
        }

        public Control GetRoot()
        {
            return this;
        }

        public void OnConnect(ObjectEditor root)
        {
            m_root = root;
        }

        public void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int arrayIndex)
        {
            m_fh = fi;
            m_mi = mi;
            fi.SetArrayIndex(arrayIndex);

            String r = fi.GetPointer(mi);
            m_ptr_target = Putki.Sys.LoadFromDisk(r);
            if (m_ptr_target == null)
                m_tbox.Foreground = Brushes.DarkRed;

            m_tbox.Text = r;
            m_tbox.IsEnabled = false;
            m_idx = arrayIndex;
        }

        public List<ObjectEditor.RowNode> GetChildRows()
        {
            List<ObjectEditor.RowNode> rows = new List<ObjectEditor.RowNode>();
            if (m_fh.IsAuxPtr())
            {
                if (m_ptr_target != null)
                {
                    ObjectEditor.RowNode e = new ObjectEditor.RowNode();
                    e.mi = m_ptr_target;
                    e.fh = null;
                    e.row = new RowDefinition();
                    e.row.Height = GridLength.Auto;
                    e.editor = new ObjectEditor();
                    e.editor.SetObject(m_ptr_target, null, 0);
                    e.children = e.editor.GetChildRows();
                    rows.Add(e);
                }
            }
            return rows;
        }

        private void OnInstantiate(object sender, RoutedEventArgs e)
        {
			if (m_fh.IsAuxPtr())
			{
				TypeSelector ts = new TypeSelector();
				ts.Owner = MainWindow.inst;
				ts.FillWithTypes(m_fh.GetRefType());
				if (ts.ShowDialog() == true)
				{
					m_ptr_target = Putki.Sys.CreateAuxInstance(m_mi, ts.selected);
					m_fh.SetArrayIndex(m_idx);
					m_fh.SetPointer(m_mi, m_ptr_target.GetPath());
					m_root.OnStructureChanged();
				}
			}
			else
			{ 
				ObjectSelector ts = new ObjectSelector();
				ts.Owner = MainWindow.inst;
				ts.SelectByTypes(m_fh.GetRefType());
				if (ts.ShowDialog() == true)
				{
					m_fh.SetArrayIndex(m_idx);
					m_fh.SetPointer(m_mi, ts.selected);
					m_root.OnStructureChanged();
					m_tbox.Text = ts.selected;
				}
			}
        }

        private void OnNull(object sender, RoutedEventArgs e)
        {
            m_fh.SetArrayIndex(m_idx);
            m_fh.SetPointer(m_mi, "");
            m_root.OnStructureChanged();
        }
    }
}
