using System;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;

namespace Putki
{
	public class LiveUpdateClient
	{
		TcpClient m_client = null;
		StreamWriter m_writer = null;

		byte[] m_buffer = new byte[4 * 1024 * 1024];
		int m_bufferPos = 0;

		public class PkgE
		{
			public bool resolved;
			public Putki.Package package;
		};

		List<PkgE> m_pending = new List<PkgE>();
		List<PkgE> m_done = new List<PkgE>();
		List<string> m_waitingFor = new List<string>();

		Putki.TypeLoader m_loader = null;

		public LiveUpdateClient(string host, Putki.TypeLoader loader)
		{
			try
			{
				m_loader = loader;
				m_client = new TcpClient(host, 5556);
				if (m_client.Connected)
				{
					m_writer = new StreamWriter(m_client.GetStream());
					m_writer.WriteLine("init csharp Unity\n");
					m_writer.Flush();
				}
			}
			catch (Exception)
			{			
				m_client = null;
			}
		}

		public void Clean(int start = 0)
		{
			for (int i = start; i < m_done.Count; i++)
			{
				for (int j = i + 1; j < m_done.Count; j++)
				{
					if (m_done[i].package.RootObjPath() == m_done[j].package.RootObjPath())
					{
						m_done.RemoveAt(i);
						Clean(i);
						return;
					}
				}
			}
		}

		public void OnResolved(PkgE p)
		{
			Console.WriteLine("Replacing asset " + p.package.RootObjPath());
			LiveUpdate.InsertPackage(p.package, m_loader);
		}

		public bool ResolvePass()
		{
			bool anyUnres = false;
			for (int i = 0; i < m_pending.Count; i++)
			{
				// make resolve set.
				List<Putki.Package> extRefs = new List<Putki.Package>();
				for (int j = 0; j < m_pending.Count; j++)
				{
					if (i != j)
						extRefs.Add(m_pending[j].package);
				}

				foreach (PkgE pe in m_done)
					extRefs.Add(pe.package);

				List<string> unresolvedRefs = m_pending[i].package.TryResolveWithRefs(extRefs, m_loader);
				foreach (string s in unresolvedRefs)
				{
					anyUnres = true;
					if (!m_waitingFor.Contains(s))
					{
						Console.WriteLine("Asking for [" + s + "]");
						m_waitingFor.Add(s);
						m_writer.WriteLine("build " + s); 
						m_writer.Flush();
					}
				}
			}

			if (!anyUnres && m_pending.Count > 0)
			{
				Console.WriteLine("Completed resolvement of " + m_pending.Count + " package(s)");
				for (int i = 0; i < m_pending.Count; i++)
				{
					m_done.Add(m_pending[i]);
					OnResolved(m_pending[i]);
				}
				m_pending.Clear();
				Clean();

				LiveUpdate.GlobalUpdate(m_loader);
				return true;
			}

			return false;
		}

		public bool Update()
		{
			if (m_client == null || m_client.Connected == false)
				return false;

			if (m_writer != null)
			{
				m_writer.WriteLine("poll");
				m_writer.Flush();
			}

			while (m_client.GetStream().DataAvailable)
			{
				m_bufferPos += m_client.GetStream().Read(m_buffer, m_bufferPos, 4 * 1024 * 1024 - m_bufferPos);
				Process();
			}

			return ResolvePass();
		}

		public void Process()
		{
			if (m_bufferPos < 16)
				return;

			// - forgot the point of this.
			// byte pkt_type = m_buffer[0];

			int sz0 = 0, sz1 = 0;
			for (int i = 0; i < 4; i++)
				sz0 |= (((byte)m_buffer[8 + i]) << 8 * i);

			for (int i = 0; i < 4; i++)
				sz1 |= (((byte)m_buffer[12 + i]) << 8 * i);

			if (m_bufferPos < (sz0 + sz1))
				return;
				
			byte[] ut =new byte[sz0+sz1];
			for (int i=0;i<(sz0+sz1);i++)
				ut[i] = m_buffer[i];

			PkgE p = new PkgE();
			p.resolved = false;
			p.package = Putki.PackageManager.LoadFromBytes(ut, m_loader);
			m_pending.Add(p);

			int peel = sz0 + sz1;
			for (int bk = 0; bk < m_bufferPos - peel; bk++)
				m_buffer[bk] = m_buffer[bk + peel];

			m_bufferPos -= peel;
			Process();
		}
	}
}
