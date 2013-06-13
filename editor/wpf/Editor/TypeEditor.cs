using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;

namespace Editor
{
    public interface TypeEditor
    {
        Control GetRoot();

        void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int araryIndex);

        List<ObjectEditor.RowNode> GetChildRows();

        void OnConnect(ObjectEditor root);
    }
}
