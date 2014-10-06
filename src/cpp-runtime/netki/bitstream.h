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
			uint8_t buffer[512];
			bitofs_t bitpos;
			int bytepos;
		};
		
		inline void set_bits(uint8_t *in, bitofs_t ofs, uint8_t value)
		{
			(*in) = (*in) | (value << ofs);
		}
		
		template<int bits>
		inline void insert_bits(buffer *target, uint8_t value)
		{
			const bitofs_t left = (8 - target->bitpos);
			set_bits(&target->buffer[target->bytepos], value, target->bitpos);

			bitofs_t np = target->bitpos + bits;
			
			if (bits > left)
				set_bits(&target->buffer[target->bytepos+1], 0, bits - left);
			
			target->bytepos = target->bytepos + (np >> 3);
			target->bitpos = np & 7;		
		}
		
		inline void sync_byte(buffer *target)
		{
			if (target->bitpos)
			{
				target->bytepos++;
				target->bitpos = 0;
			}
		}
		
		inline void insert_bytes(buffer *target, uint8_t *buf, int size)
		{
			sync_byte(target);
			for (int i=0;i!=size;i++)
				target->buffer[target->bytepos + i] = buf[i];
			target->bytepos += size; 
		}
	}
}

#endif
