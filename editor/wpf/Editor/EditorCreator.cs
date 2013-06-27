using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor
{
    public class EditorCreator
    {
        public static TypeEditor MakeEditor(Putki.FieldHandler fh, bool isArray)
        {
            if (isArray)
            {
                return new ArrayEditor();
            }
            else
            {
                switch ((int)fh.GetType())
                {
                    // struct instance
					case 4:
						{
							Putki.TypeDefinition StructType = fh.GetRefType();
							string type = StructType.GetName();
							string editor = StructType.GetInlineEditor();
							if (StructType.GetInlineEditor() == "Vec4")
							{
								return new Vec4Editor();
							}
							else
							{
								return new ObjectEditor();
							}
						}
                    case 3:
                        return new PointerEditor();
                    case 0:
                        return new IntEditor();
                    case 5:
                        return new FileEditor();
					case 7:
						return new FloatEditor();
					case 8:
						return new EnumEditor();
                    // 
                    default:
                        return new TextEditor();
                }
            }
        }
    }
}
