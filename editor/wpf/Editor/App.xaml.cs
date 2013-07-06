using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
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
             //Putki.Sys.Load("C:\\gitproj\\putki\\test-project\\build\\testapp-data-dll.dll", "c:\\gitproj\\putki\\test-project\\data");
            Putki.Sys.Load("C:\\gitproj\\putki\\ui-example\\build\\ui-example-data-dll.dll", "c:\\gitproj\\putki\\ui-example\\data\\objs");

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
