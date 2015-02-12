using System;

namespace netki
{
	public interface ConnectionOutput
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
		void OnStartup();
		StreamConnection OnConnected(int connection_id, ConnectionOutput output);
	}
}
