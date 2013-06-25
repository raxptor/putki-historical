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
    /// Interaction logic for FloatEditor.xaml
    /// </summary>
    public partial class FloatEditor : UserControl, TypeEditor
    {
        int m_idx = -1;
        Brush m_obrush;

        public FloatEditor()
        {
            InitializeComponent();
            m_obrush = m_tbox.Foreground;
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
            m_tbox.Text = fi.GetFloat(mi).ToString();
            m_tbox.TextChanged += delegate { 
                float o;
                if (Single.TryParse(m_tbox.Text, out o))
                {
                    fi.SetFloat(mi, o);
                    m_tbox.Foreground = m_obrush;
                }
                else
                {
                    m_tbox.Foreground = Brushes.Red;
                }
            };

            m_idx = arrayIndex;
        }

        public List<ObjectEditor.RowNode> GetChildRows()
        {
            return new List<ObjectEditor.RowNode>();
        }
    }
}
