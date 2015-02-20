using System;

namespace netki
{
	public struct DecodedPacket
	{
		public int type_id;
		public Packet packet;
	}

	public interface PacketDecoder
	{
		// Returns number of bytes consumed. pkt.type_id >= 0 on success. pkt.type_id < 0 on failure.
		// TODO: Returns negative numbers for stream failure
		int Decode(byte[] data, int offset, int length, out DecodedPacket pkt);
	}
}
