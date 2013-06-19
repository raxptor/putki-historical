using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestLoad
{
    class Program
    {
        static void Main(string[] args)
        {
			Putki.Package p = Putki.Package.LoadFromFile("c:\\gitproj\\putki\\ccg-ui\\out\\csharp32\\packages\\static.pkg");


			outki.ProjectDescription dr = (outki.ProjectDescription)p.Resolve("description");
			Console.WriteLine("Welcome to " + dr.Name);
        }
    }
}
