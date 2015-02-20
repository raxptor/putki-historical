using System;

namespace netki
{
	public class Packet
	{
		public int type_id;

		public Packet(int _type_id)
		{
			type_id = _type_id;
		}
	}
}
