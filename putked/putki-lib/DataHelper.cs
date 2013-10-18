using System;

namespace PutkEd
{
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
	}
}

