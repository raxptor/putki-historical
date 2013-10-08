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

		public void Load()
		{
			if (MED_Initialize("c:\\gitproj\\putki\\ui-example\\build\\ui-example-data-dll.dll", "c:\\gitproj\\putki\\ui-example\\data") > 0)
			{
				for (int i=0;i<1000;i++)
				{
					IntPtr p = MED_TypeByIndex(i);
					if ((int)p != 0)
					{
						Types t = new Types();
						t.Name = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(MED_Type_GetName(p));
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

