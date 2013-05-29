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
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Dynamic;
using System.Reflection; 
using System.CodeDom.Compiler; 
using Microsoft.CSharp;

namespace Editor
{
    class EditorObjectCreator
    {
        public object Make()
        {
            CompilerParameters parms = new CompilerParameters();
            parms.GenerateExecutable = false;
            parms.GenerateInMemory = true;
            parms.IncludeDebugInformation = false;
            parms.ReferencedAssemblies.Add("System.dll");
            parms.ReferencedAssemblies.Add("System.Core.dll");
            parms.ReferencedAssemblies.Add("System.Data.dll");
            parms.ReferencedAssemblies.Add("mscorlib.dll");
            Dictionary<string, string> compilerOptions = new Dictionary<string, string>();
            compilerOptions.Add("CompilerVersion", "v4.0");
            CodeDomProvider compiler = CSharpCodeProvider.CreateProvider("CSharp", compilerOptions);

            string source = "class Banan { public string lalle { get; set; } }";
            
            CompilerResults res = compiler.CompileAssemblyFromSource(parms, source);
            object newObject = res.CompiledAssembly.CreateInstance("Banan");

            /*
            putki.Sys ps = new putki.Sys();
            putki.TypeDefinition td = ps.get_type_definition("Kalle");

            string n = td.get_name();
            */

            return newObject;
       } 
    }
}
