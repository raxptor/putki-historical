#ifndef __NETKI_ENCODER_H__
#define __NETKI_ENCODER_H__

namespace netki
{
	namespace encoder
	{
		struct state;
		state *create();
		void free(state*);
	}
}

#endif
