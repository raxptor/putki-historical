using System;

namespace netki
{
	public interface PacketStreamOutput
	{
		void SendPacket(byte[] data, int offset, int length);
	}

	public interface StreamConnection
	{
		void OnStreamData(byte[] data, int offset, int length);
		void OnDisconnected();
	}

	public interface StreamConnectionHandler
	{
		void OnStartup();
		StreamConnection OnConnected(int connection_id);
	}
}
