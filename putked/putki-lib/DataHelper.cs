using System;
using System.Collections.Generic;

namespace PutkEd
{
	public interface ProxyObject
	{
		void Connect(DLLLoader.MemInstance mi);
	}

    public class ResolveCache
    {
        // These cached objects actually don't contain any data, besides the MemInstance which actually
        // points to a c++ struct somewhere, which actually holds the data.
        private static Dictionary<string, object> s_c = new Dictionary<string, object>();

        public object Resolve(string path)
        {
            if (s_c.ContainsKey(path))
                return s_c[path];

            DLLLoader.MemInstance mi = DLLLoader.DiskLoad(path, true);
            if (mi == null)
            {
                s_c.Add(path, mi);
                return null;
            }

            object pobj = DataHelper.CreatePutkEdObj(mi);
            s_c.Add(path, pobj);
            return pobj;        
        }
    }



	[AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
	public class PutkedProxyDescriptor : System.Attribute
	{
		//////////////////////////////////////////////////////////////////////////
		public PutkedProxyDescriptor(string name)
		{
			PutkiName = name;
		}

		public string PutkiName;
	}

	public static class DataHelper
	{	
		// Returns object of type 'parentType' or null
		public static DLLLoader.MemInstance CastUp(DLLLoader.MemInstance mi, DLLLoader.Types parentType)
		{
			if (mi.GetTypeName() == parentType.Name)
				return mi;

			for (int i = 0;true;i++)
			{
				DLLLoader.PutkiField pf = mi.GetField(i);
				if (pf == null)
					return null;
				if (pf.GetName() == "parent")
					return CastUp(pf.GetStructInstance(mi), parentType);
			}
		}

		public static object Get(DLLLoader.MemInstance mi, string FieldName)
		{
			for (int i = 0;true;i++)
			{
				DLLLoader.PutkiField pf = mi.GetField(i);
				if (pf == null)
				{
					Console.WriteLine("Error. Field [" + FieldName + "] not found");
					return null;
				}

				switch (pf.GetFieldType())
				{
					case 3: return pf.GetPointer(mi);
					case 6: return pf.GetBool(mi);
					case 7: return pf.GetFloat(mi);
					case 8: return pf.GetEnum(mi);
					case 0: return pf.GetInt32(mi);
					default: Console.WriteLine("Get: Unhandled field type " + pf.GetFieldType() + " for field name " + FieldName);
					break;
				}
			}
		}

		public static ProxyObject CreatePutkEdObj(DLLLoader.MemInstance mi)
		{
			if (mi == null)
				return null;

			string targetName = mi.GetTypeName();

			Type ti = typeof(ProxyObject);
			foreach (System.Reflection.Assembly asm in AppDomain.CurrentDomain.GetAssemblies())
			{
				foreach (Type t in asm.GetTypes())
				{
					if (ti.IsAssignableFrom(t) && !t.IsInterface)
					{
						object[] attributes;
						attributes = t.GetCustomAttributes(typeof(PutkedProxyDescriptor), false);
						foreach (Object attribute in attributes)
						{
							PutkedProxyDescriptor desc = (PutkedProxyDescriptor) attribute;
							if (desc.PutkiName == targetName)
							{
								ProxyObject po = (ProxyObject) Activator.CreateInstance(t);
								po.Connect(mi);
								return po;
							}
						}
					}
				}
			}

			Console.WriteLine("ERROR: CreatePutkEdObj; no proxy for [" + targetName + "]");
			return null;
		}
	}
}

