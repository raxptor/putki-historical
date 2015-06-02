using System;

namespace netki
{
	public delegate void PacketLaneOutput(Bitstream.Buffer send);

	public interface PacketLane
	{
		void Incoming(Bitstream.Buffer stream);
		void Send(Bitstream.Buffer stream);
		Bitstream.Buffer Update(float dt, PacketLaneOutput outputFn);
		float ComputePacketLoss();
	}
}