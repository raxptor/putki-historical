﻿using System;
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
        Putki.MemInstance lastObject = null;
        public static MainWindow inst = null;

		public MainWindow()
		{
            inst = this;
			InitializeComponent();

			m_fileBrowser.FileSelected += OnFileSelected;
		}

		public FileBrowser GetFileTree()
		{
			return m_fileBrowser;
		}

		public void OnFileSelected(object sender, EventArgs a)
		{
			FileSelectedArgs fsa = (FileSelectedArgs)a;

            Putki.MemInstance mi = Putki.Sys.LoadFromDisk(fsa.Path);

            m_objectEditor.SetObject(mi);

            lastObject = mi;
		}

        private void Button_Click_Save(object sender, RoutedEventArgs e)
        {
            if (lastObject != null)
            {
                Putki.Sys.SaveObject(lastObject);
            }
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            if (lastObject != null)
            {
                Putki.Sys.MemBuildAsset(lastObject.GetPath());
            }
        }

		private void Button_Click_NewObject(object sender, RoutedEventArgs e)
		{
			NewObject ts = new NewObject();
			ts.Owner = MainWindow.inst;
			if (ts.ShowDialog() == true)
			{
				Putki.MemInstance mi = Putki.Sys.CreateInstance(ts.path, ts.selected);
				Putki.Sys.SaveObject(mi);
				m_fileBrowser.LoadFromDisk();
			}
		}
	}
}
