using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using putki;

namespace Editor
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        App()
        {
            putki.Sys apa = new putki.Sys();
            apa.load("C:\\gitproj\\putki\\test-project\\build\\testapp-data-dll.dll");

            for (int i=0;;i++)
            {
                putki.TypeDefinition th = apa.get_type_by_index(i);
                if (th == null)
                    break;

                System.Console.WriteLine("Loaded type [" + th.GetName() + "]");
            }
        }
        
    }
}
