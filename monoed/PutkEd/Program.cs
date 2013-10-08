using System;
using System.IO;
using Gtk;

namespace PutkEd
{
	class MainClass
	{
		public static Loader s_loader = null;
		
		
		public static void Main (string[] args)
		{
			Directory.SetCurrentDirectory("c:\\gitproj\\putki\\ui-example");

			FileIndex fi = new FileIndex();
			fi.Load("data/objs");

			Application.Init();
			s_loader = new Loader();
			MainWindow win = new MainWindow();
			win.Show();
						
			Application.Run ();
		}
	}
}


