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
    public partial class NewObject : Window
    {
        public Putki.TypeDefinition selected = null;
		public string path;

        List<Putki.TypeDefinition> items = new List<Putki.TypeDefinition>();		

        public NewObject()
        {
            InitializeComponent();

			for (int i = 0; ; i++)
			{
				Putki.TypeDefinition td = Putki.Sys.GetTypeByIndex(i);
				if (td == null)
					break;

				m_types.Items.Add(td.GetName());
				items.Add(td);
			}
			m_types.SelectedIndex = 0;
		}

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            //selected = items[m_list.SelectedIndex];
            this.DialogResult = true;
			selected = items[m_types.SelectedIndex];
			path = m_path.Text;
        }
    }
}
