using System.Text;

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
		class Slot
		{
			public string path;
			public object inst;
			public int type;
		};
		
		Slot[] m_slots;
		
		public object ResolveSlot(int index)
		{
			if (index > 0)
				return m_slots[index - 1].inst;
			return null;
		}
		
		public object Resolve(string path)
		{
			foreach (Slot s in m_slots)
				if (s.path == path)
					return s.inst;
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
			
			for (int i=0;i<slots;i++)
			{
				loader.ResolveFromPackage(m_slots[i].type, m_slots[i].inst, this);
			}
			
			return true;
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
			if (p.LoadFromBytes(bytes, loader))
				return p;
			else
				return null;
		}
	}	
}
