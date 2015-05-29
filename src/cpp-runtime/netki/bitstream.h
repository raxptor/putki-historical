#ifndef __NETKI_BITSTREAM_H__
#define __NETKI_BITSTREAM_H__

#include <cstdint>

namespace netki
{
	namespace bitstream
	{
		typedef int bitofs_t;

		struct buffer
		{
			uint8_t *buf;
			uint32_t bufsize;
			bitofs_t bitpos;
			int bytepos;
			int error;
		};

		void flip_buffer(buffer *buf)
		{
			if (buf->bitpos)
				buf->bufsize = buf->bytepos + 1;
			else
				buf->bufsize = buf->bytepos;
			buf->bitpos = 0;
			buf->bytepos = 0;
			buf->error = 0;
		}

		template<int size>
		void init_buffer(buffer *buf, char(&data)[size])
		{
			buf->buf = (uint8_t*)data;
			buf->bufsize = size;
			buf->bitpos = 0;
			buf->bytepos = 0;
			buf->error = 0;
		}

		static const uint8_t bitmask[9] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};

		long bits_left(buffer *buf)
		{
			return (buf->bufsize - buf->bytepos) * 8 - buf->bitpos;
		}

		long bytes_left(buffer *buf)
		{
			return buf->bufsize - buf->bytepos - 1;
		}

		void set_bits(uint8_t *in, bitofs_t ofs, uint8_t value)
		{
			if (!ofs)
			{
				*in = value;
			}
			else
			{
				(*in) = (*in) | (value << ofs);
			}
		}

		template<int bits>
		uint8_t get_bits(uint8_t *in, bitofs_t ofs)
		{
			return ((*in) >> ofs) & bitmask[bits];
		}

		template<int bits>
		void insert_bits(buffer *target, uint32_t value)
		{
			if (bits_left(target) < bits)
			{
				target->error = 1;
				return;
			}
			
			if (bits > 8)
			{
				insert_bits<8>(target, value & 0xff);
				insert_bits<bits - 8>(target, value >> 8);
				return;
			}

			const bitofs_t left = (8 - target->bitpos);
			set_bits(&target->buf[target->bytepos], target->bitpos, value);

			bitofs_t np = target->bitpos + bits;

			if (bits > left)
				set_bits(&target->buf[target->bytepos+1], 0, value >> left);

			target->bytepos = target->bytepos + (np >> 3);
			target->bitpos = np & 7;
		}

		template<> void insert_bits<0>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-1>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-2>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-3>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-4>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-5>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-6>(buffer *target, uint32_t value) { }
		template<> void insert_bits<-7>(buffer *target, uint32_t value) { }

		uint32_t read_bits(buffer *source, unsigned int count);

		// MAX bits is 8
		template<int bits>
		uint32_t read_bits(buffer *source)
		{
			if (bits_left(source) < bits)
			{
				source->error = 1;
				return 0;
			}
		
			if (bits > 8)
			{
				return read_bits(source, bits);
			}

			const bitofs_t left = (8 - source->bitpos);

			uint32_t fetch_first = left < bits ? left : bits;
			uint32_t value = (source->buf[source->bytepos] >> source->bitpos) & bitmask[fetch_first];

			bitofs_t np = source->bitpos + bits;
			source->bytepos = source->bytepos + (np >> 3);
			source->bitpos = np & 7;

			if (fetch_first != bits)
			{
				uint32_t remaining = bits - fetch_first;
				value = value | ((source->buf[source->bytepos] & bitmask[remaining]) << fetch_first);
				source->bitpos = remaining;
			}
			
			return value;
		}

		uint32_t read_bits(buffer *source, unsigned int count)
		{
			switch (count)
			{
				case 0: return 0;
				case 1: return read_bits<1>(source);
				case 2: return read_bits<2>(source);
				case 3: return read_bits<3>(source);
				case 4: return read_bits<4>(source);
				case 5: return read_bits<5>(source);
				case 6: return read_bits<6>(source);
				case 7: return read_bits<7>(source);
				case 8: return read_bits<8>(source);
				default:
					return read_bits<8>(source) | (read_bits(source, count - 8) << 8);
			}
		}

		void sync_byte(buffer *target)
		{
			if (target->bitpos)
			{
				target->bytepos++;
				target->bitpos = 0;
			}
		}

		bool insert_bytes(buffer *target, uint8_t *buf, int size)
		{
			sync_byte(target);
			if (bytes_left(target) < size)
			{
				target->error = 1;
				return false;
			}
				
			for (int i=0; i!=size; i++)
				target->buf[target->bytepos + i] = buf[i];
			target->bytepos += size;
			return true;
		}

		bool read_bytes(buffer *source, uint8_t *buf, int size)
		{
			sync_byte(source);
		
			if (bytes_left(source) < size)
			{
				source->error = 1;
				return false;
			}
				
			for (int i=0; i!=size; i++)
				buf[i] = source->buf[source->bytepos + i];
			source->bytepos += size;
			return true;
		}

		void *alloc(buffer *target, uint32_t size)
		{
			if (bytes_left(target) < size)
			{
				target->error = 1;
				return 0;
			}

			void *ptr = &target->buf[target->bytepos];
			target->bytepos += size;
			return ptr;
		}
		
		void insert_compressed_int(buffer *target, uint32_t value)
		{
			uint32_t bits;
			if (value == 0xffffffff)
			{
				insert_bits<6>(target, 0);
				insert_bits<6>(target, 1);
			}
			else if (value > 0xffff) 
			{
				insert_bits<5>(target, 0);
				insert_bits<1>(target, 1);
				insert_bits<32>(target, value);
			}
			else if (value > 0xfff)
			{
				insert_bits<4>(target, 0);
				insert_bits<1>(target, 1);
				insert_bits<15>(target, value);
			}
			else if (value > 0xff)
			{
				insert_bits<3>(target, 0);
				insert_bits<1>(target, 1);
				insert_bits<12>(target, value);
			}
			else if (value > 0xf)
			{
				insert_bits<2>(target, 0);
				insert_bits<1>(target, 1);
				insert_bits<8>(target, value);
			}
			else if (value > 0)
			{
				insert_bits<1>(target, 0);
				insert_bits<1>(target, 1);
				insert_bits<4>(target, value);
			}
			else
			{
				insert_bits<1>(target, 1);
			}
		}
		
		uint32_t read_compressed_int(buffer *source)
		{
			if (read_bits<1>(source) == 1)
				return 0;
			if (read_bits<1>(source) == 1)
				return read_bits<4>(source);
			if (read_bits<1>(source) == 1)
				return read_bits<4>(source);
			if (read_bits<1>(source) == 1)
				return read_bits<8>(source);
			if (read_bits<1>(source) == 1)
				return read_bits<12>(source);
			if (read_bits<1>(source) == 1)
				return read_bits<16>(source);
			if (read_bits<1>(source) == 1)
				return read_bits<32>(source);
			return 0xffffffff;
		}
	}
}

#endif
