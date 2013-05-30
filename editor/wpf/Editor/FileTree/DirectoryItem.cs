using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor.FileTree
{
    public class DirectoryItem : Item
    {
        public List<Item> Items { get; set; }

        public DirectoryItem()
        {
            Items = new List<Item>();
        }
    }
}
