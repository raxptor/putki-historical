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
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
        EditorObjectCreator eoc = new EditorObjectCreator();
        Putki.MemInstance lastObject = null;

		public MainWindow()
		{
			InitializeComponent();
			m_propertyGrid.ShowSearchBox = false;

			m_fileBrowser.FileSelected += OnFileSelected;            
		}

		public void OnFileSelected(object sender, EventArgs a)
		{
            if (lastObject != null)
                Putki.Sys.SaveObject(lastObject);

			FileSelectedArgs fsa = (FileSelectedArgs)a;

            Putki.MemInstance mi = Putki.Sys.LoadFromDisk(fsa.Path);

            dynamic objectProxy = eoc.Make(mi.GetType());
            objectProxy.PutkiObj = mi;

            m_propertyGrid.SelectedObject = objectProxy;

            lastObject = mi;
		}
	}
}
