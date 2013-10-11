using System.Text;
using System.Collections.Generic;
using System;

namespace Putki
{
    // Implemented by app.
    public interface TypeLoader
    {
        void ResolveFromPackage(int type, object obj, Putki.Package pkg);
        object LoadFromPackage(int type, Putki.PackageReader reader);
    }

	public class PackageReader
	{
		static UTF8Encoding enc = new UTF8Encoding();
		
		byte[] data;
		int pos;
		
		public PackageReader(byte[] _data)
		{
			data = _data;
            pos = 0;
		}
		
		public int ReadInt32()
		{
			int val = data[pos] + (data[pos+1] << 8) + (data[pos+2] << 16) + (data[pos+3] << 24);
			pos += 4;
			return val;
		}

		public int ReadInt16()
		{
			int val = data[pos] + (data[pos+1] << 8);
			pos += 2;
			return val;
		}
		
		public byte ReadByte()
		{
			return data[pos++];	
		}

		public float ReadFloat()
		{
			float f = System.BitConverter.ToSingle(data, pos);
			pos += 4;
			return f;
		}
		
		public string ReadString()
		{
			return ReadString(ReadInt32());
		}
		
		public string ReadString(int bytes)
		{
			byte[] tmp = new byte[bytes-1];
			for (int i=0;i<bytes-1;i++)
				tmp[i] = data[pos + i];
			pos += bytes;
			return enc.GetString(tmp);
		}
		
		public void Skip(int bytes)
		{
			pos += bytes;
		}
		
		public PackageReader CloneAux(int ofs)
		{
			PackageReader r = new PackageReader(data);
			r.pos = pos + ofs;
			return r;
		}
		
		public void MoveTo(PackageReader rdr)
		{
			pos = rdr.pos;	
		}
	}
	
	public class Package
	{
		public class Slot
		{
			public string path;
			public object inst;
			public int type;
		};
		
		public Slot[] m_slots;

		string[] m_pathTable;

		List<Package> m_extRefs = null;
		List<string> m_unresolved = null;
		bool m_gotUnresolved = false;

		public List<string> TryResolveWithRefs(List<Package> extRefs, TypeLoader loader)
		{
			m_extRefs = extRefs;
			m_unresolved = new List<string>();

			for (int i = 0; i < m_slots.Length; i++)
			{
				loader.ResolveFromPackage(m_slots[i].type, m_slots[i].inst, this);
			}

			List<string> r = m_unresolved;
			m_unresolved = null;
			m_extRefs = null;
			return r;
		}

		public string RootObjPath()
		{
			return m_slots[0].path;
		}
		
		public object ResolveSlot(int index)
		{
			if (index == 0)
			{
				// null pointer
				return null;
			}
			else if (index > 0)
			{
				// slot lookup
				if (index <= m_slots.Length)
					return m_slots[index - 1].inst;
				else
					return null; // bork.
			}
			else
			{
				int pt = -index - 1;
				if (pt < m_pathTable.Length)
					return Resolve(m_pathTable[pt]);
				else
					return null;
			}
		}
		
		public object Resolve(string path)
		{
			if (m_extRefs != null)
			{
				foreach (Package p in m_extRefs)
				{
					foreach (Slot s in p.m_slots)
					{
						if (s.path == path)
						{
							return s.inst;
						}
					}
				}
			}

			foreach (Slot s in m_slots)
			{
				if (s.path == path)
				{
					return s.inst;
				}
			}

			if (path == "")
			{
				Console.WriteLine("EMPTY PATH!");
				return null;
			}

			m_gotUnresolved = true;

			if (m_unresolved != null)
			{
				m_unresolved.Add(path);
			}

			return null;
		}
		
		public bool LoadFromBytes(byte[] data, TypeLoader loader)
		{
			PackageReader rdr = new PackageReader(data);
			
			int slots = rdr.ReadInt32();
			m_slots = new Slot[slots];
			
			for (int i=0;i<slots;i++)
			{
				m_slots[i] = new Slot();
				
				const int pathFlag = 1 << 31;
				int hdr = rdr.ReadInt32();
				m_slots[i].type = hdr & ~pathFlag;
				
				if ((hdr & pathFlag) == pathFlag)
					m_slots[i].path = rdr.ReadString(rdr.ReadInt16());	

				m_slots[i].inst = loader.LoadFromPackage(m_slots[i].type, rdr);
			}

			int paths = rdr.ReadInt32();
			m_pathTable = new string[paths];
			for (int i=0;i<paths;i++)
				m_pathTable[i] = rdr.ReadString(rdr.ReadInt16());

			for (int i=0;i<slots;i++)
			{
				loader.ResolveFromPackage(m_slots[i].type, m_slots[i].inst, this);
			}

			Console.WriteLine("LoadFromBytes: Got unresolved? " + m_gotUnresolved);
			return !m_gotUnresolved;
		}

		public void Release()
		{
			m_slots = null;
		}
	}
	
	public class PackageLoader
	{
		static public Package FromBytes(byte[] bytes, TypeLoader loader)
		{
			Package p = new Package();
			p.LoadFromBytes(bytes, loader);
			return p;
		}
	}	
}
