#ifndef __PUTKI_RUNTIME_LIVEUPDATE_H__
#define __PUTKI_RUNTIME_LIVEUPDATE_H__

#include <putki/types.h>

namespace putki
{
	namespace liveupdate
	{
		void connect();

		// returns true if updated, then pointer for new asset.
		bool update(instance_t *ptr);
	}
}

#endif