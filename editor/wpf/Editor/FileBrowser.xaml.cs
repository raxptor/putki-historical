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

		public Dictionary<string, Putki.TypeDefinition> m_dataPaths = null;

		public FileBrowser()
		{
			InitializeComponent();	
			m_treeView.SelectedItemChanged += SelectionChanged;

			LoadFromDisk();
		}

		public List<string> AssetsByType(Putki.TypeDefinition def)
		{
			List<string> n = new List<string>();
			foreach (string name in m_dataPaths.Keys)
			{
				if (m_dataPaths[name].GetName() == def.GetName())
					n.Add(name);
			}
			return n;
		}

		public void Add(FileTree.Item item)
		{
			FileTree.DirectoryItem di = item as FileTree.DirectoryItem;
			if (di != null)
			{
				foreach (FileTree.Item i in di.Items)
					Add(i);
				return;
			}

			Putki.MemInstance mi = Putki.Sys.LoadFromDisk(item.Path);
			if (mi != null)
				m_dataPaths.Add(mi.GetPath(), mi.GetType());			
		}

		public void LoadFromDisk()
		{
			m_dataPaths = new Dictionary<string, Putki.TypeDefinition>();

			var itemProvider = new FileTree.ItemProvider();
			var items = itemProvider.GetItems("c:/gitproj/ccg-cybots/data/objs", "");

			foreach (FileTree.Item i in items)
				Add(i);

			DataContext = items;
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
