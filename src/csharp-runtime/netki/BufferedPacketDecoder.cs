using System;

namespace netki
{
	public class BufferedPacketDecoder : PacketDecoder
	{
		byte[] _data;
		PacketDecoder _decoder;
		int _parsepos, _readpos;

		public BufferedPacketDecoder(int bufsize, PacketDecoder decoder)
		{
			_data = new byte[bufsize];
			_decoder = decoder;
			_parsepos = 0;
			_readpos = 0;
		}

		public int Decode(byte[] data, int offset, int length, out DecodedPacket pkt)
		{
			int ret;

			// When data exists in queue, add on and attempt decode.
			if (_readpos > 0)
			{
				Save(data, offset, length);
				ret = DoDecode(_data, _parsepos, _readpos - _parsepos, out pkt);
				if (ret > 0)
				{
					OnParsed(ret);
				}
				return length;
			}

			// No data in queue; attempt decode directly in buffer
			ret = DoDecode(data, offset, length, out pkt);
			if (pkt.type_id < 0)
			{
				// No decode yet. Consume what it wants and store the rest.
				Save(data, offset + ret, length - ret);
				return length;
			}

			return ret;
		}

		public void OnParsed(int bytes)
		{
			_parsepos += bytes;
			if (_parsepos == _readpos)
			{
				_readpos = 0;
				_parsepos = 0;
			}
		}

		public void Save(byte[] data, int offset, int length)
		{
			for (int i = 0; i < length; i++)
			{
				_data[_readpos + i] = data[offset + i];
			}
			_readpos += length;
		}

		public int DoDecode(byte[] data, int offset, int length, out DecodedPacket pkt)
		{
			return _decoder.Decode(data, offset, length, out pkt);
		}
	}
}

