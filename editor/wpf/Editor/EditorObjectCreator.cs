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
        public object Make(Putki.TypeDefinition typeDef)
        {
            CompilerParameters parms = new CompilerParameters();
            parms.GenerateExecutable = false;
            parms.GenerateInMemory = true;
            parms.IncludeDebugInformation = false;
            parms.ReferencedAssemblies.Add("System.dll");
            parms.ReferencedAssemblies.Add("System.Core.dll");
            parms.ReferencedAssemblies.Add("System.Data.dll");
            parms.ReferencedAssemblies.Add("Microsoft.CSharp.dll");
            parms.ReferencedAssemblies.Add("mscorlib.dll");
            Dictionary<string, string> compilerOptions = new Dictionary<string, string>();
            compilerOptions.Add("CompilerVersion", "v4.0");
            CodeDomProvider compiler = CSharpCodeProvider.CreateProvider("CSharp", compilerOptions);

            string source = "public class " + typeDef.GetName() + " {";

            source += "public dynamic PutkiObj;\n";

            for (int i = 0; i < 100; i++)
            {
                Putki.FieldHandler fi = typeDef.GetField(i);
                if (fi == null)
                    break;
                
                source += "public string " + fi.GetName() + " {\n";
                source += " get { return PutkiObj.GetType().GetField(" + i + ").GetString(PutkiObj); } \n";
                source += " set { PutkiObj.GetType().GetField(" + i + ").SetString(PutkiObj, value); } \n";
                source += "}\n";
            }

            source += "}";
            
            CompilerResults res = compiler.CompileAssemblyFromSource(parms, source);
            object newObject = res.CompiledAssembly.CreateInstance(typeDef.GetName());

            
            /*
            putki.Sys ps = new putki.Sys();
            putki.TypeDefinition td = ps.get_type_definition("Kalle");

            string n = td.get_name();
            */

            return newObject;
       } 
    }
}
