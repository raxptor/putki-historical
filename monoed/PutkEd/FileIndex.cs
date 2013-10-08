using System;
using System.IO;
using System.Collections.Generic;

namespace PutkEd
{
	public class FileIndex
	{
		public FileIndex()
		{

		}

		public void Load(string path)
		{
			string cp = Clean(path);

			List<string> dirs = new List<string>(Directory.EnumerateDirectories(cp));
			foreach (string s in dirs)
			{
				string cs = Clean(s);
				Console.WriteLine("Added path [" + cs + "]");
			}
		}

		public static string Clean(string path)
		{
			return path.Replace("\\", "/").Trim();
		}

	}
}

