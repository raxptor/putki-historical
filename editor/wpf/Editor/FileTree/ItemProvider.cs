using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Editor.FileTree
{
    public class ItemProvider
    {
        public List<Item> GetItems(string root, string subPath)
        {
            var items = new List<Item>();

            DirectoryInfo dirInfo;

            string nLev = "";

            if (subPath.Length > 0)
            {
                dirInfo = new DirectoryInfo(root + "/" + subPath);
                nLev = subPath + "/";
            }
            else
                dirInfo = new DirectoryInfo(root);

            foreach (var directory in dirInfo.GetDirectories())
            {
                var item = new DirectoryItem
                {
                    Name = directory.Name,
                    Path = directory.FullName,
                    Items = GetItems(root, nLev + directory.Name)
                };

                items.Add(item);
            }

            foreach (var file in dirInfo.GetFiles())
            {
                if (file.Name.Contains(".json"))
                {
                    var item = new FileItem
                    {
                        Name = file.Name.Replace(".json", ""),
                        Path = (nLev + file.Name).Replace(".json", "")
                    };

                    items.Add(item);
                }
            }

            return items;
        }
    }
}
