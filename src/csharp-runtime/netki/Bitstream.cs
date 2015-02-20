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

		public static void Copy(Buffer dst, Buffer src)
		{
			dst.buf = src.buf;
			dst.bufsize = src.bufsize;
			dst.bytepos = src.bytepos;
			dst.bitpos = src.bitpos;
			dst.error = src.error;
		}

		public static void Insert(Buffer dst, Buffer src)
		{
			while (src.BitsLeft() > 0)
			{
				if (src.bitpos > 0)
					Bitstream.PutBits(dst, 8 - src.bitpos, Bitstream.ReadBits(src, 8 - src.bitpos));
				if (src.BitsLeft() > 32)
					Bitstream.PutBits(dst, 32, Bitstream.ReadBits(src, 32));
				if (src.BitsLeft() >= 8)
					Bitstream.PutBits(dst, 8, Bitstream.ReadBits(src, 8));
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
	
		public static void PutCompressedUInt(Buffer buf, uint value)
		{
			int bits = 0, prefixes = 0;
			
			if (value == 0xffffffff)
			{
				prefixes = 6;
				bits = 0;
			}
			
			if (value > 0xffff) 
			{
				bits = 32;
				prefixes = 5;
			}
			else if (value > 0xfff)
			{
				bits = 16;
				prefixes = 4;
			}
			else if (value > 0xff)
			{
				bits = 12;
				prefixes = 3;
			}
			else if (value > 0xf)
			{
				bits = 8;
				prefixes = 2;
			}
			else if (value > 0)
			{
				bits = 4;
				prefixes = 1;
			}
			
			if (prefixes > 0)
				PutBits(buf, prefixes, 0xffff);
			PutBits(buf, 1, 0);
			if (bits > 0)
				PutBits(buf, bits, value);
		}

		public static void PutCompressedInt(Buffer buf, int value)
		{
			if (value < 0)
			{
				PutBits(buf, 1, 1);
				PutCompressedUInt(buf, (uint)(-value));
			}
			else
			{
				PutBits(buf, 1, 0);
				PutCompressedUInt(buf, (uint)value);
			}
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