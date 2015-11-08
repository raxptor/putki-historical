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

		public static void Insert(Buffer dest, Buffer source)
		{
			Bitstream.Buffer tmp = new Bitstream.Buffer();
			Copy(tmp, source);

			while (tmp.BitsLeft() > 0)
			{
				if (tmp.bitpos > 0)
				{
					int bits = 8 - tmp.bitpos;
					if (bits > tmp.BitsLeft())
						bits = tmp.BitsLeft();
					Bitstream.PutBits(dest, bits, Bitstream.ReadBits(tmp, bits));
				}
				if (tmp.BitsLeft() > 32)
					Bitstream.PutBits(dest, 32, Bitstream.ReadBits(tmp, 32));
				if (tmp.BitsLeft() >= 8)
					Bitstream.PutBits(dest, 8, Bitstream.ReadBits(tmp, 8));
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
	
			public static void PutCompressedUint(Buffer buf, uint value)
		{
			int bits = 0, prefixes = 0;
			
			if (value == 0xffffffff)
			{
				prefixes = 6;
				bits = 0;
			}
			else if (value > 0xffff) 
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
				PutBits(buf, prefixes, 0);
			if (prefixes != 6)
				PutBits(buf, 1, 1);
			if (bits > 0)
				PutBits(buf, bits, value);
		}

		public static uint ReadCompressedUint(Buffer buf)
		{
			if (ReadBits(buf, 1) == 1)
				return 0;
			if (ReadBits(buf, 1) == 1)
				return ReadBits(buf, 4);
			if (ReadBits(buf, 1) == 1)
				return ReadBits(buf, 8);
			if (ReadBits(buf, 1) == 1)
				return ReadBits(buf, 12);
			if (ReadBits(buf, 1) == 1)
				return ReadBits(buf, 16);
			if (ReadBits(buf, 1) == 1)
				return ReadBits(buf, 32);
			return 0xffffffff;
		}

		public static void PutCompressedInt(Buffer buf, int value)
		{
			if (value < 0)
			{
				PutBits(buf, 1, 1);
				PutCompressedUint(buf, (uint)(-value));
			}
			else
			{
				PutBits(buf, 1, 0);
				PutCompressedUint(buf, (uint)value);
			}
		}

		public static int ReadCompressedInt(Buffer buf)
		{
			if (ReadBits(buf, 1) == 0)
				return (int)ReadCompressedUint(buf);
			else
		        return (int)-ReadCompressedUint(buf);
		}
		
		public static float ReadFloat(Buffer buf)
		{
			SyncByte(buf);
			if (buf.BitsLeft() < 32)
				return 0;
			float f = BitConverter.ToSingle(buf.buf, buf.bytepos);
			buf.bytepos += 4;
			return f;
		}
		
		public static void PutFloat(Buffer buf, float f)
		{
			byte[] val = BitConverter.GetBytes(f);
			PutBytes(buf, val);
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

		public static void PutStringDumb(Buffer buf, string value)
		{
			SyncByte (buf);
			if (value == null)
			{
				Bitstream.PutCompressedInt(buf, -1);
				return;
			}

			byte[] bytes = System.Text.UTF8Encoding.UTF8.GetBytes(value);
			PutCompressedInt(buf, bytes.Length);
			PutBytes(buf, bytes);
		}

		public static string ReadStringDumb(Buffer buf)
		{
			SyncByte (buf);
			int len = ReadCompressedInt(buf);
			if (len > 65536)
			{
				buf.error = 4;
				return null;
			}
			if (len < 0)
			{
				return null;
			}

			byte[] data = ReadBytes(buf, len);
			if (data == null || buf.error != 0)
			{
				return null;
			}
			return System.Text.UTF8Encoding.UTF8.GetString(data);
		}
	}
}