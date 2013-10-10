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

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Type_GetField(IntPtr type, int i);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetName(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_GetType(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetString(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetStructInstance(IntPtr field, IntPtr mi);

		private static string MSTR(IntPtr a)
		{
			return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(a);
		}

		public class PutkiField
		{
			public IntPtr Handler;

			public string GetName()
			{
				return MSTR(MED_Field_GetName(Handler));
			}

			public int GetFieldType()
			{
				return MED_Field_GetType(Handler);
			}

			public string GetString(MemInstance mi)
			{
				return MSTR(MED_Field_GetString(Handler, mi.PutkiInst));
			}

			public MemInstance GetStructInstance(MemInstance mi)
			{
				MemInstance smi = new MemInstance();
				smi.PutkiInst = MED_Field_GetStructInstance(Handler, mi.PutkiInst);
				return smi;
			}

			public bool IsArray()
			{
				return false;
			}

			public bool ShowInEditor()
			{
				return true;
			}
		}

		public class MemInstance
		{
			public IntPtr PutkiInst;

			public string GetTypeName()
			{
				return MSTR(MED_Type_GetName(MED_TypeOf(PutkiInst)));
			}

			public string GetPath()
			{
				return "Sökvägen 3";
			}

			public PutkiField GetField(int index)
			{
				IntPtr p = MED_Type_GetField(MED_TypeOf(PutkiInst), index);
				if ((int)p != 0)
				{
					PutkiField pf = new PutkiField();
					pf.Handler = p;
					return pf;
				}
				return null;
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
			System.Console.WriteLine("Calling!");
			if (MED_Initialize(DataDLL, DataPath) > 0)
			{
				System.Console.WriteLine("done");

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

