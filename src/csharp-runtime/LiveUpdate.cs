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

		public static void InsertPackage(Putki.Package package)
		{
			foreach (Package.Slot s in package.m_slots)
			{
				if (s.path.Length > 0)
				{
					// someone loaded this before, point them to this new version.
					if (m_currentMapping.ContainsKey(s.path))
					{
						m_globalReplace[m_currentMapping[s.path]] = s.inst;
					}

					// update current mapping.
					m_currentMapping[s.path] = s.inst;
				}
			}
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
