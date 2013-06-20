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
    public partial class FileEditor : UserControl, TypeEditor
    {
        int m_idx;
        Putki.FieldHandler m_fh;
        Putki.MemInstance m_mi;
        ObjectEditor m_root;

        public FileEditor()
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
            m_tbox.Text = fi.GetString(mi);
            m_tbox.IsEnabled = false;
            m_idx = arrayIndex;
        }

        public List<ObjectEditor.RowNode> GetChildRows()
        {
            return new List<ObjectEditor.RowNode>();
        }

        private void OnBrowse(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog fd = new System.Windows.Forms.OpenFileDialog();
            if (fd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                m_fh.SetArrayIndex(m_idx);
                m_fh.SetString(m_mi, fd.FileName);
                m_tbox.Text = fd.FileName;
            }
        }

        private void OnNull(object sender, RoutedEventArgs e)
        {
            m_fh.SetArrayIndex(m_idx);
            m_fh.SetString(m_mi, "");
            m_root.OnStructureChanged();
        }
    }
}
