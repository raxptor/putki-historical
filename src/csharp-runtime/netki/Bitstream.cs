using System;

namespace netki
{
	public static class Bitstream
	{
		public static UInt32[] s_bitmask = new UInt32[] {0x00, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};

		public class Buffer
		{
			public byte[] buf;
			public int bufsize;
			public int bytepos;
			public int bitpos;
			public int error;

			public int BitsLeft()
			{
				return (bufsize - bytepos) * 8 - bitpos;
			}

			public static Buffer Make(byte[] buffer)
			{
				Buffer b = new Buffer();
				b.buf = buffer;
				b.bufsize = buffer.Length;
				b.bytepos = 0;
				b.bitpos = 0;
				b.error = 0;
				return b;
			}

			public void Flip()
			{
				error = 0;
				if (bitpos > 0)
					bufsize = bytepos + 1;
				else
					bufsize = bytepos;
				bytepos = 0;
				bitpos = 0;
			}
		}

		public static bool PutBits(Buffer buf, int bits, UInt32 value)
		{
			if (buf.BitsLeft() < bits)
			{
				buf.error = 1;
				return false;
			}

			if (bits > 8)
			{
				PutBits(buf, 8, value & 0xff);
				PutBits(buf, bits - 8, value >> 8);
				return true;
			}

			if (buf.bitpos == 0)
			{
				buf.buf[buf.bytepos] = (byte)(value & s_bitmask[bits]);
				if (bits == 8)
					buf.bytepos++;
				else
					buf.bitpos = bits;
			}
			else
			{
				int room = 8 - buf.bitpos;
				if (bits <= room)
				{
					byte mixin = (byte)((value & s_bitmask[bits]) << buf.bitpos);
					buf.buf[buf.bytepos] = (byte)(buf.buf[buf.bytepos] | mixin);
					buf.bitpos += bits;
					if (buf.bitpos == 8)
					{
						buf.bitpos = 0;
						buf.bytepos++;
					}
				}
				else
				{
					PutBits(buf, room, value);
					return PutBits(buf, bits - room, value >> room); 
				}
			}

			return true;
		}

		public static void SyncByte(Buffer buf)
		{
			if (buf.bitpos > 0)
			{
				buf.bitpos = 0;
				buf.bytepos++;
			}
		}

		public static bool PutBytes(Buffer buf, byte[] data)
		{
			SyncByte(buf);
			if (buf.bufsize - buf.bytepos < data.Length)
				return false;
		
			System.Buffer.BlockCopy(data, 0, buf.buf, buf.bytepos, data.Length);
			buf.bytepos += data.Length;
			return false;
		}

		public static UInt32 ReadBits(Buffer buf, int bits)
		{
			if (buf.BitsLeft() < bits)
			{
				buf.error = 1;
				return 0;
			}

			int max = 8 - buf.bitpos;
			if (bits <= max)
			{
				UInt32 val = ((UInt32)(buf.buf[buf.bytepos] >> buf.bitpos)) & s_bitmask[bits];
				buf.bitpos += bits;
				if (buf.bitpos == 8)
				{
					buf.bitpos = 0;
					buf.bytepos++;
				}
				return val;
			}

			return ReadBits(buf, max) | (ReadBits(buf, bits - max) << max);
		}

		public static byte[] ReadBytes(Buffer buf, int count)
		{
			SyncByte(buf);
			if (buf.bufsize - buf.bytepos < count)
				return null;

			byte[] dst = new byte[count];
			System.Buffer.BlockCopy(buf.buf, buf.bytepos, dst, 0, count);
			buf.bytepos += count;
			return dst;
		}
	}
}