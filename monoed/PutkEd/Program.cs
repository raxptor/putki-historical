using System;
using System.IO;
using Gtk;

namespace PutkEd
{
	class MainClass
	{
		public static Loader s_loader = null;
		public static FileIndex s_fileIndex = null;
		public static DLLLoader s_dataDll = null;
		
		public static void Main (string[] args)
		{
			Directory.SetCurrentDirectory("c:\\gitproj\\putki\\ui-example");

			s_dataDll = new DLLLoader();
			s_dataDll.Load();


			s_fileIndex = new FileIndex();
			s_fileIndex.Load("data/objs");

			Application.Init();
			s_loader = new Loader();
			MainWindow win = new MainWindow();
			win.Show();
						
			Application.Run ();
		}
	}
}


