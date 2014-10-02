using System;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;

namespace Putki
{
	public class LiveUpdate
	{
		private static Dictionary<string, object> m_currentMapping = new Dictionary<string, object>();
		private static Dictionary<object, object> m_globalReplace = new Dictionary<object, object>();
		private static List<Putki.Package> m_old = new List<Putki.Package>();
		private static List<Putki.Package> m_patchOrder = new List<Putki.Package>();

		public static void InsertPackage(Putki.Package package, TypeLoader ldr)
		{
			foreach (Package.Slot s in package.m_slots)
			{
				if (s.path.Length > 0 && s.inst != null)
				{
					// someone loaded this before, point them to this new version.
					if (m_currentMapping.ContainsKey(s.path))
					{
						if (m_currentMapping[s.path] != s.inst)
							m_globalReplace[m_currentMapping[s.path]] = s.inst;
					}

					// update current mapping.
					m_currentMapping[s.path] = s.inst;
				}
			}

			m_patchOrder.Insert(0, package);
			m_old.Add(package);
		}

		public static void GlobalUpdate(TypeLoader ldr)
		{
			// patch up all the old ones.
			foreach (Package p in m_old)
			{
				List<string> borv = p.TryResolveWithRefs(m_patchOrder, ldr);
				if (borv.Count > 0)
				{
					Console.WriteLine("AAAAGHHH!" + borv.Count);
				}
			}
		}

		public static bool IsOld<Type>(Type probe)
		{
			if (probe == null)
				return false;
				
			return m_globalReplace.ContainsKey((object)probe);
		}

		public static bool Update<Type>(ref Type probe)	
		{
			if (probe == null)
				return false;
			
			if (m_globalReplace.ContainsKey((object)probe))
			{
				probe = (Type) m_globalReplace[(object)probe];
				return true;
			}
			return false;
		}
	}
}
