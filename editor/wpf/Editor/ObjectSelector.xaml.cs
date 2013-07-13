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
    public partial class ObjectSelector : Window
    {
        public string selected = null;
        List<string> items = new List<string>();

        public ObjectSelector()
        {
            InitializeComponent();
        }

        public void SelectByTypes(Putki.TypeDefinition compatible)
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

				// find all these assets. this should be provided by putki and not hacked like this.
				List<string> f = MainWindow.inst.GetFileTree().AssetsByType(td);
				foreach (string s in f)
				{
					m_list.Items.Add(s);
					items.Add(s);
				}
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
