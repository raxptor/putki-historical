#ifndef __NETKI_DECODER_H__
#define __NETKI_DECODER_H__

namespace netki
{
	namespace decoder
	{
		struct state;
		state *create();
		void free(state*);
	}
}

#endif
