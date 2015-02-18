using System;

namespace netki
{
	public interface ConnectionOutput
	{
		void Send(byte[] data, int offset, int length);
	}

	public interface DatagramOutput
	{
		void Send(byte[] data, int offset, int length);
	}

	public interface StreamConnection
	{
		void OnStreamData(byte[] data, int offset, int length);
		void OnDisconnected();
	}

	public interface StreamConnectionHandler
	{
		StreamConnection OnConnected(int connection_id, ConnectionOutput output);
	}

	public delegate void DatagramHandler(byte[] data, System.UInt64 endpoint);
}
