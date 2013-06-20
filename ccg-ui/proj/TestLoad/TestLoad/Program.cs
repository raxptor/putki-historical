using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace TestLoad
{
    class Program
    {
        static void Main(string[] args)
        {
			Putki.Package p = Putki.PackageLoader.FromBytes(File.ReadAllBytes("c:\\gitproj\\putki\\ccg-ui\\out\\csharp32\\packages\\static.pkg"));


			outki.ProjectDescription dr = (outki.ProjectDescription)p.Resolve("description");
			Console.WriteLine("Welcome to " + dr.Name);

			outki.UIWidget w = (outki.UIWidget)p.Resolve("widget");
			Console.WriteLine("Widget id is " + w.width);

			outki.Font f = (outki.Font)p.Resolve("fonts/calibri");
			Console.WriteLine("Font is " + f);


        }
    }
}
