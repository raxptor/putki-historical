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
	/// Interaction logic for FileBrowser.xaml
	/// </summary>    
	/// 

	public class FileSelectedArgs : EventArgs
	{
		public string Path;
	}

	public partial class FileBrowser : UserControl
	{
		public event EventHandler FileSelected;

		public FileBrowser()
		{
			InitializeComponent();

			var itemProvider = new FileTree.ItemProvider();
			var items = itemProvider.GetItems("c:/gitproj/putki/claw/data", "");
			DataContext = items;


			m_treeView.SelectedItemChanged += SelectionChanged;
		}

		private void SelectionChanged(object sender, RoutedPropertyChangedEventArgs<Object> e)
		{
			//Perform actions when SelectedItem changes
			if (e.NewValue is FileTree.FileItem)
			{
				FileTree.FileItem fi = (FileTree.FileItem)e.NewValue;
				if (fi != null)
				{
					FileSelectedArgs fa = new FileSelectedArgs();
					fa.Path = fi.Path;
					FileSelected(this, fa);
				}
			}
		}

	}
}
