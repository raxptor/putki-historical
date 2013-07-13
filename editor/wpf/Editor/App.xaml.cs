using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.IO;
using Putki;

namespace Editor
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        App()
        {

			foreach (string line in File.ReadLines("putki-editor.config"))
			{

			}

             //Putki.Sys.Load("C:\\gitproj\\putki\\test-project\\build\\testapp-data-dll.dll", "c:\\gitproj\\putki\\test-project\\data");
			Putki.Sys.Load("C:\\gitproj\\ccg-cybots\\build\\ui-cybots-data-dll.dll", "c:\\gitproj\\ccg-cybots\\data\\objs");

            for (int i=0;;i++)
            {
                Putki.TypeDefinition th = Putki.Sys.GetTypeByIndex(i);
                if (th == null)
                    break;

                System.Console.WriteLine("Loaded type [" + th.GetName() + "]");
            }
        }
        
    }
}
