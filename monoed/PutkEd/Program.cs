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
			Directory.SetCurrentDirectory("/Users/dannilsson/git/lilwiz");
			// Directory.SetCurrentDirectory("c:\\gitproj\\putki\\ui-example");

			s_loader = new Loader();

			s_dataDll = new DLLLoader();
			s_dataDll.Load(s_loader.m_configOpts["datadll"], s_loader.m_configOpts["obj"]);

			s_fileIndex = new FileIndex();
			s_fileIndex.Load(s_loader.m_configOpts["obj"]);

			Application.Init();

			MainWindow win = new MainWindow();
			win.Show();
						
			Application.Run ();
		}
	}
}


