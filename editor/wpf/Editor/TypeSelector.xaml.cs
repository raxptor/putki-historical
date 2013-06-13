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
using System.Windows.Shapes;

namespace Editor
{
    public partial class TypeSelector : Window
    {
        public Putki.TypeDefinition selected = null;
        List<Putki.TypeDefinition> items = new List<Putki.TypeDefinition>();

        public TypeSelector()
        {
            InitializeComponent();
        }

        public void FillWithTypes(Putki.TypeDefinition compatible)
        {
            for (int i=0;;i++)
            {
                Putki.TypeDefinition td = Putki.Sys.GetTypeByIndex(i);
                if (td == null)
                    break;

                // filter out anything but sub objects.
                if (compatible != null)
                {
                    bool match = td.GetName() == compatible.GetName();
                    if (!match)
                    {
                        Putki.TypeDefinition rh = td.GetParentType();
                        while (rh != null && !match)
                        {
                            match = (rh.GetName() == compatible.GetName());
                            rh = rh.GetParentType();
                        }
                    }

                    if (!match)
                        continue;

                }
                
                m_list.Items.Add(td.GetName());
                items.Add(td);
            }
            m_list.SelectedIndex = 0;

        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            selected = items[m_list.SelectedIndex];
            this.DialogResult = true;
        }
    }
}
