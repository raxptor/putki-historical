using System;
using System.IO;
using System.Collections.Generic;

namespace PutkEd
{
	public class FileIndex
	{

		public class LoadedInfo
		{
			public DLLLoader.MemInstance LoadedMI;
		};

		public class Entry
		{
			public string FilePath;
			public string AssetName;
			public string DisplayType;
			public LoadedInfo Info;
		};

		List<string> m_dirs = new List<string>();
		List<string> m_files = new List<string>();
		List<Entry> m_assets = new List<Entry>();

		public FileIndex()
		{

		}

		public List<Entry> GetAssets()
		{
			return m_assets;
		}

		public void Load(string path)
		{
			string cp = Clean(path);

			List<string> toExplore = new List<string>();
			toExplore.Add(cp);

			while (toExplore.Count > 0)
			{
				List<string> dirs = new List<string>(Directory.EnumerateDirectories(toExplore[0]));
				toExplore.RemoveAt(0);

				foreach (string s in dirs)
				{
					string cs = Clean(s);
					toExplore.Add(cs);
					m_dirs.Add(cs);
				}
			}

			m_dirs.Sort();

			foreach (string dn in m_dirs)
			{
				List<string> files = new List<string>(Directory.EnumerateFiles(dn));
				foreach (string f in files)
				{
					string cf = Clean(f);
					m_files.Add(cf);
				}
				// Console.WriteLine("Path [" + dn + "] contains " + m_files.Count + " file(s)");
			}

			int pathCut = cp.Length + 1; // cut the '/'
			foreach (string n in m_files)
			{
				Entry e = new Entry();
				e.FilePath = n;
				e.AssetName = n.Substring(pathCut).Replace(".json", "");

				DLLLoader.MemInstance mi = PutkEd.DLLLoader.DiskLoad(e.AssetName);
				if (mi != null)
				{
					// Store 
					LoadedInfo li = new LoadedInfo();
					li.LoadedMI = mi;
					e.Info = li;
					e.DisplayType = mi.GetTypeName();
				}

				m_assets.Add(e);
				// Console.WriteLine("[" + e.FilePath + "] contains [" + e.AssetName + "]");
			}
		}

		public static string Clean(string path)
		{
			return path.Replace("\\", "/").Trim();
		}

	}
}

