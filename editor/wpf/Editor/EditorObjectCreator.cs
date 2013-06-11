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

        private static string MakeClassDef(Putki.TypeDefinition typeDef, HashSet<string> ignoreClasses)
        {
            string source = "";
            string prepend = "";
            string hookup = "";

            source += "public class " + typeDef.GetName() + " {";
            source += "public dynamic PutkiObj;\n";            

            for (int i = 0; i < 100; i++)
            {
                Putki.FieldHandler fi = typeDef.GetField(i);
                if (fi == null)
                    break;

                switch ((int)fi.GetType())
                {
                    case 1: // byte
                        {
                            source += "[Xceed.Wpf.Toolkit.PropertyGrid.Attributes.PropertyOrder(" + i + ")]";
                            source += "public int " + fi.GetName() + " {\n";
                            source += " get { return PutkiObj.GetType().GetField(" + i + ").GetByte(PutkiObj); } \n";
                            source += " set { PutkiObj.GetType().GetField(" + i + ").SetByte(PutkiObj, value); } \n";
                            source += "}\n";
                            break;
                        }

                    case 2:
                        {
                            source += "[Xceed.Wpf.Toolkit.PropertyGrid.Attributes.PropertyOrder(" + i + ")]";
                            source += "public string " + fi.GetName() + " {\n";
                            source += " get { return PutkiObj.GetType().GetField(" + i + ").GetString(PutkiObj); } \n";
                            source += " set { PutkiObj.GetType().GetField(" + i + ").SetString(PutkiObj, value); } \n";
                            source += "}\n";
                            break;
                        }
                    case 3:
                        {
                            source += "[Xceed.Wpf.Toolkit.PropertyGrid.Attributes.PropertyOrder(" + i + ")]";
                            source += "public string " + fi.GetName() + " {\n";
                            source += " get { return PutkiObj.GetType().GetField(" + i + ").GetPointer(PutkiObj); } \n";
                            source += " set { PutkiObj.GetType().GetField(" + i + ").SetPointer(PutkiObj, value); } \n";
                            source += "}\n";
                            break;
                        }
                    case 4:
                        {
                            Putki.TypeDefinition refType = fi.GetRefType();
                            if (!ignoreClasses.Contains(refType.GetName()))
                            {
                                ignoreClasses.Add(refType.GetName());
                                prepend = MakeClassDef(refType, ignoreClasses) + prepend;
                            }

                            source += "[ExpandableObject]\n";
                            source += "public " + refType.GetName() + " " + fi.GetName() + " { get; set; } \n";
                            hookup += fi.GetName() + " = new " + refType.GetName() + "();\n";
                            hookup += fi.GetName() + ".Setup(PutkiObj.GetType().GetField(" + i + ").GetStructInstance(PutkiObj));\n";
                            break;
                        }
                }
            }

            source += "public void Setup(dynamic po)\n";
            source += "{\n";            
            source += "    PutkiObj = po;\n";
            source += hookup;
            source += "}\n";
            source += "}\n";
            return prepend + "\n" + source;
        }

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
            parms.ReferencedAssemblies.Add("Xceed.Wpf.Toolkit.dll");
            parms.ReferencedAssemblies.Add("Editor.exe");
            Dictionary<string, string> compilerOptions = new Dictionary<string, string>();
            compilerOptions.Add("CompilerVersion", "v4.0");
            CodeDomProvider compiler = CSharpCodeProvider.CreateProvider("CSharp", compilerOptions);    
            
            string source = "";
            source += "using System.Collections.Generic;\n";
            source += "using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;";
            source += MakeClassDef(typeDef, new HashSet<string>());

            System.Console.WriteLine(source);
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
