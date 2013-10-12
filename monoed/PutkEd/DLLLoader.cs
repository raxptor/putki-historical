using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;

namespace PutkEd
{
	public class DLLLoader
	{
		public class Types
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
		private static extern IntPtr MED_Type_GetParentType(IntPtr type);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_PathOf(IntPtr MemInstance);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_DiskLoad(string path);

		[DllImport("monoed-interop")]
		private static extern void MED_DiskSave(IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern void MED_OnObjectModified(IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_CreateInstance(string path, IntPtr typehandler);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_CreateAuxInstance(IntPtr miOnto, IntPtr typehandler);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Type_GetField(IntPtr type, int i);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetName(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetRefType(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_IsArray(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_IsAuxPtr(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern bool MED_Field_ShowInEditor(IntPtr field);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetArrayIndex(IntPtr field, int index);
		
		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Type_GetInlineEditor(IntPtr type);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_GetArraySize(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_ArrayInsert(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_ArrayErase(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_GetType(IntPtr field);
				
		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetEnumPossibility(IntPtr field, int i);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetString(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetEnum(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetPointer(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_GetInt32(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern float MED_Field_GetFloat(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_GetBool(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern int MED_Field_GetByte(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern IntPtr MED_Field_GetStructInstance(IntPtr field, IntPtr mi);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetString(IntPtr field, IntPtr mi, string value);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetEnum(IntPtr field, IntPtr mi, string value);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetPointer(IntPtr field, IntPtr mi, string value);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetInt32(IntPtr field, IntPtr mi, int value);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetBool(IntPtr field, IntPtr mi, bool value);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetByte(IntPtr field, IntPtr mi, int value);

		[DllImport("monoed-interop")]
		private static extern void MED_Field_SetFloat(IntPtr field, IntPtr mi, float value);

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

			public string GetRefType()
			{
				return MSTR(MED_Field_GetRefType(Handler));
			}

			public bool IsArray()
			{
				return MED_Field_IsArray(Handler) != 0;
			}

			public int GetArraySize(MemInstance mi)
			{
				return MED_Field_GetArraySize(Handler, mi.PutkiInst);
			}

			public void SetArrayIndex(int i)
			{
				MED_Field_SetArrayIndex(Handler, i);
			}

			public void ArrayInsert(MemInstance mi)
			{
				MED_Field_ArrayInsert(Handler, mi.PutkiInst);
				MED_OnObjectModified(mi.PutkiInst);
		}

			public void ArrayErase(MemInstance mi)
			{
				MED_Field_ArrayErase(Handler, mi.PutkiInst);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public string GetString(MemInstance mi)
			{
				return MSTR(MED_Field_GetString(Handler, mi.PutkiInst));
			}
						
			public string GetEnum(MemInstance mi)
			{
				return MSTR(MED_Field_GetEnum(Handler, mi.PutkiInst));
			}

			public String GetEnumPossibility(int idx)
			{
				IntPtr i = MED_Field_GetEnumPossibility(Handler, idx);
				if ((int)i != 0)
					return MSTR(i);
				return null;
			}

			public string GetPointer(MemInstance mi)
			{
				return MSTR(MED_Field_GetPointer(Handler, mi.PutkiInst));
			}

			public int GetInt32(MemInstance mi)
			{
				return MED_Field_GetInt32(Handler, mi.PutkiInst);
			}

			public float GetFloat(MemInstance mi)
			{
				return MED_Field_GetFloat(Handler, mi.PutkiInst);
			}			

			public bool GetBool(MemInstance mi)
			{
				return MED_Field_GetBool(Handler, mi.PutkiInst) != 0;
			}

			public byte GetByte(MemInstance mi)
			{
				return (byte) MED_Field_GetByte(Handler, mi.PutkiInst);
			}

			public void SetString(MemInstance mi, string value)
			{
				MED_Field_SetString(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public void SetEnum(MemInstance mi, string value)
			{
				MED_Field_SetEnum(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public void SetPointer(MemInstance mi, string value)
			{
				MED_Field_SetPointer(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public void SetInt32(MemInstance mi, int value)
			{
				MED_Field_SetInt32(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public void SetBool(MemInstance mi, bool  value)
			{
				MED_Field_SetBool(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public void SetByte(MemInstance mi, byte value)
			{
				MED_Field_SetByte(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public void SetFloat(MemInstance mi, float value)
			{
				MED_Field_SetFloat(Handler, mi.PutkiInst, value);
				MED_OnObjectModified(mi.PutkiInst);
			}

			public bool IsAuxPtr()
			{
				return MED_Field_IsAuxPtr(Handler) != 0;
			}

			public MemInstance GetStructInstance(MemInstance mi)
			{
				MemInstance smi = new MemInstance();
				smi.PutkiInst = MED_Field_GetStructInstance(Handler, mi.PutkiInst);
				return smi;
			}

			public bool ShowInEditor()
			{
				return MED_Field_ShowInEditor(Handler);
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
				return MSTR(MED_PathOf(PutkiInst));
			}

			public void DiskSave()
			{
				MED_DiskSave(PutkiInst);
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

			public static MemInstance LoadFromDisk(string path)
			{
				return DiskLoad(path);
			}

			public static MemInstance Create(string path, Types th)
			{
				MemInstance mi = new MemInstance();
				mi.PutkiInst = MED_CreateInstance(path, th.Handler);
				return mi;
			}

			public MemInstance CreateAux(Types th)
			{
				MemInstance mi = new MemInstance();
				mi.PutkiInst = MED_CreateAuxInstance(PutkiInst, th.Handler);
				return mi;
			}
		}

		public static  MemInstance DiskLoad(string path)
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

		public static String GetInlineEditor(Types tu)
		{
			IntPtr t = MED_Type_GetInlineEditor(tu.Handler);
			if ((int)t != 0)
				return MSTR(t);
			return null;
		}

		public static bool HasParent(Types which, Types wouldBeParent)
		{
			IntPtr b = which.Handler;

			while ((int)b != 0)
			{
				if (MED_Type_GetName(b) == MED_Type_GetName(wouldBeParent.Handler))
					return true;
				b = MED_Type_GetParentType(b);
			}

			return false;
		}

		public List<Types> GetTypes()
		{
			return m_loadedTypes;
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

