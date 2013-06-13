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
    /// Interaction logic for TextEditor.xaml
    /// </summary>
    public partial class TextEditor : UserControl, TypeEditor
    {
        int m_idx = 0;

        public TextEditor()
        {
            InitializeComponent();
        }

        public Control GetRoot()
        {
            return this;
        }

        public void OnConnect(ObjectEditor root)
        {

        }

        public void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int arrayIndex)
        {            
            fi.SetArrayIndex(arrayIndex);
            m_tbox.Text = fi.GetString(mi);
            m_tbox.TextChanged += delegate { fi.SetString(mi, m_tbox.Text); };

            m_idx = arrayIndex;
        }

        public List<ObjectEditor.RowNode> GetChildRows()
        {
            return new List<ObjectEditor.RowNode>();
        }
    }
}
