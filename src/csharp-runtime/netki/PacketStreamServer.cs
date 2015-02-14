using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;

namespace netki
{
	public class PacketStreamServer
	{
		private StreamConnectionHandler _handler;
		private Socket _listener;

		class Connection : ConnectionOutput
		{
			public Socket socket;
			public StreamConnection conn;
			public byte[] recvbuf;

			public void Send(byte[] data, int offset, int length)
			{
				try
				{
					if (socket.Connected)
						socket.Send(data, offset, length, 0);
				}
				catch (System.ObjectDisposedException)
				{
				}
			}
		};

		private List<int> _free_connections = new List<int>();
		private Connection[] _connections;

		public PacketStreamServer(StreamConnectionHandler handler)
		{
			_handler = handler;
		}

		public void OnAsyncReceive(IAsyncResult result)
		{
			int connection_id = (int)result.AsyncState;
			Connection conn = _connections[connection_id];

			int ret = conn.socket.EndReceive(result);
			if (ret <= 0)
			{
				conn.conn.OnDisconnected();
				_free_connections.Add(connection_id);
				conn.socket.Close();
			}
			else
			{
				System.Random r = new System.Random();
				int rp = 0;
				while (rp < ret)
				{
					int amt = r.Next() % (ret - rp + 1);
					amt = ret - rp;
					if (amt > 0)
					{
						conn.conn.OnStreamData(conn.recvbuf, rp, amt);
						rp += amt;
					}
				}
				conn.socket.BeginReceive(conn.recvbuf, 0, conn.recvbuf.Length, 0, OnAsyncReceive, connection_id);
			}
		}

		public void OnAsyncAccepted(IAsyncResult result)
		{
			Socket nsock = _listener.EndAccept(result);

			// Occupy new slot.
			int pos = _free_connections.Count - 1;
			if (pos > 0)
			{
				int connection_id = _free_connections[pos];
				_free_connections.RemoveAt(pos);

				Connection c = new Connection();
				c.socket = nsock;
				c.recvbuf = new byte[4096];
				c.conn = _handler.OnConnected(connection_id, c);
				_connections[connection_id] = c;

				nsock.BeginReceive(c.recvbuf, 0, c.recvbuf.Length, 0, OnAsyncReceive, connection_id);
			}

			_listener.BeginAccept(OnAsyncAccepted, _listener);
		}

		public void Start(int port, int max_connections=100)
		{
			_connections = new Connection[max_connections];
			for (int i = 0; i < max_connections; i++)
				_free_connections.Add(max_connections - i - 1);

			IPEndPoint localEP = new IPEndPoint(0, port);		
			_listener = new Socket(localEP.Address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
			_listener.Bind(localEP);
			_listener.Listen(100);
			_listener.BeginAccept(OnAsyncAccepted, _listener);
		}
	}
}
