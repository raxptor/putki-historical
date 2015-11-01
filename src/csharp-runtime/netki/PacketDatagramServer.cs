using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;

namespace netki
{
	public delegate void OnDatagramDelegate(byte[] data, uint length, ulong EndPoint);

	public class PacketDatagramServer
	{
		private Socket _listener;
		private OnDatagramDelegate _pkt;
		private int _port;
		private string _host;

		public PacketDatagramServer(OnDatagramDelegate pkt)
		{
			_host = "localhost";
			_pkt = pkt;
		}

		private void NextRead()
		{
		}

		public int GetPort()
		{
			return _port;
		}

		public string GetHost()
		{
			return _host;
		}

		private void ReadLoop()
		{
			byte[] _recvBuf = new byte[65536];
			EndPoint ep = new IPEndPoint(IPAddress.Any, 0);
			while (true)
			{
				int bytes = _listener.ReceiveFrom(_recvBuf, ref ep);
				if (bytes > 0)
				{
					IPEndPoint ipep = (IPEndPoint)ep;

					byte[] addr = ipep.Address.GetAddressBytes();

					int addr_portion = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
					ulong endpoint = (ulong)addr_portion | ((ulong)ipep.Port) << 32;
					_pkt(_recvBuf, (uint)bytes, endpoint);
				}
			}
		}

		public void Send(byte[] data, int offset, int length, ulong endpoint)
		{
			IPAddress p = new IPAddress((long)(endpoint & 0xffffffff));
			IPEndPoint ipep = new IPEndPoint(p, (int)(endpoint >> 32));
			_listener.SendTo(data, offset, length, 0, ipep);
		}

		public void Start(int port, int max_peers = 100)
		{
			IPEndPoint localEP = new IPEndPoint(0, 0);
			_listener = new Socket(localEP.Address.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
			_listener.Bind(localEP);
			_port = ((IPEndPoint)_listener.LocalEndPoint).Port;

			System.Threading.Thread th = new System.Threading.Thread(ReadLoop);
			th.Start();
		}
	}
}
