using System;
using System.Collections.Generic;

namespace PutkEd
{
	public class EditorCreator
	{
		public static TypeEditor MakeEditor(DLLLoader.PutkiField fh, bool isArray)
		{
			if (isArray)
			{
				return new ArrayEditor();
			}

			switch (fh.GetFieldType())
			{
				case 4:
				{
					string RT = fh.GetRefType();
					String InlineEditor = null;
					foreach (DLLLoader.Types t in MainClass.s_dataDll.GetTypes())
					{
						if (t.Name == RT)
						{
							InlineEditor = DLLLoader.GetInlineEditor(t);
							break;
						}
					}

					if (InlineEditor == "Vec4")
						return new Vec4Editor();

					return new ObjectEditor();
				}
				case 3:
					return new PointerEditor();
				case 6:
					return new BoolEditor();
				case 7:
					return new FloatEditor();
				case 8:
					return new EnumEditor();
				case 0:
					return new IntEditor();
				default:
					return new TextEditor();
			}
		}
	}
}