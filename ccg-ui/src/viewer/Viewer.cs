using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace ViewerApp
{
    class Viewer
    {
		[STAThread]
        static void Main(string[] args)
        {
			Putki.Package p = Putki.PackageLoader.FromBytes(File.ReadAllBytes("packages\\static.pkg"));


			outki.UIScreen screen = (outki.UIScreen)p.Resolve("screens/ingame");

			RootWindow w = new RootWindow(new CCGUI.UIScreenRenderer(screen));
			w.ShowDialog();


			/*
			outki.ProjectDescription dr = (outki.ProjectDescription)p.Resolve("description");
			Console.WriteLine("Welcome to " + dr.Name);

			outki.UIWidget w = (outki.UIWidget)p.Resolve("widget");
			Console.WriteLine("Widget id is " + w.width);

			outki.Font f = (outki.Font)p.Resolve("fonts/calibri");
			Console.WriteLine("Font is " + f);
				
			outki.Atlas a = (outki.Atlas)p.Resolve("atlas1");
			Console.WriteLine("Atlas 1 is " + a);
			 */
		}
	}
}

	