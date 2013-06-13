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
                        return new ObjectEditor();
                    case 0:
                        return new IntEditor();
                    // 
                    default:
                        return new TextEditor();
                }
            }
        }
    }
}
