using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;

namespace PutkEd
{
	public class DLLLoader
	{
		public struct Types
		{
			public string Name;
			public IntPtr Handler;
		};

		List<Types> m_loadedTypes = new List<Types>();
		
		[DllImport("monoed-interop")]
		private static extern int MED_Initialize(string DllPath, string DataPath);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_TypeByIndex(int i);
		
		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Type_GetName(IntPtr type);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_TypeOf(IntPtr MemInstance);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_DiskLoad(string path);

		private static string MSTR(IntPtr a)
		{
			return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(a);
		}

		public class MemInstance
		{
			public IntPtr PutkiInst;

			public string GetTypeName()
			{
				return MSTR(DLLLoader.MED_Type_GetName(MED_TypeOf(PutkiInst)));
			}
		}

		public MemInstance DiskLoad(string path)
		{
			IntPtr p = MED_DiskLoad(path);
			if ((int)p != 0)
			{
				MemInstance mi = new MemInstance();
				mi.PutkiInst = p;
				return mi;
			}
			return null;
		}

		public void Load(string DataDLL, string DataPath)
		{
			if (MED_Initialize(DataDLL, DataPath) > 0)
			{
				for (int i=0;i<1000;i++)
				{
					IntPtr p = MED_TypeByIndex(i);
					if ((int)p != 0)
					{
						Types t = new Types();
						t.Name = MSTR(MED_Type_GetName(p));
						t.Handler = p;
						m_loadedTypes.Add(t);
					}
				}
			}

			System.Console.WriteLine("I have loaded (" + m_loadedTypes.Count + ") types.");
		}

		public DLLLoader()
		{

		}
	}
}

