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
    /// Interaction logic for ArrayEditor.xaml
    /// </summary>
    public partial class ArrayEditor : UserControl, TypeEditor
    {
        Putki.MemInstance m_mi;
        Putki.FieldHandler m_fh;

        public ArrayEditor()
        {
            InitializeComponent();
        }

        public Control GetRoot()
        {
            return this;
        }

        public void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int arrayIndex)
        {
            m_mi = mi;
            m_fh = fi;
        }

        public List<ObjectEditor.RowNode> GetChildRows()
        {
            List<ObjectEditor.RowNode> list = new List<ObjectEditor.RowNode>();
            int entries = m_fh.GetArraySize(m_mi);
            for (int i = 0; i < entries; i++)
            {
                ObjectEditor.RowNode e = new ObjectEditor.RowNode();
                e.mi = m_mi;
                e.fh = m_fh;
                e.row = new RowDefinition();
                e.row.Height = GridLength.Auto;
                e.editor = EditorCreator.MakeEditor(m_fh, false);
                e.editor.SetObject(m_mi, m_fh, i);
                e.children = e.editor.GetChildRows();
                e.arrayIndex = i;
                list.Add(e);
            }
            return list;
        }
    }
}
