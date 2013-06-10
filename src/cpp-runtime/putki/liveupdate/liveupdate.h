#ifndef __PUTKI_RUNTIME_LIVEUPDATE_H__
#define __PUTKI_RUNTIME_LIVEUPDATE_H__

#include <putki/types.h>

#define LIVEUPDATE_ENABLE 

namespace putki
{
	namespace liveupdate
	{
		struct data;

#if !defined(LIVEUPDATE_ENABLE)
		inline void init() { }
		inline data* connect() { return 0; }
		inline void disconnect(data *d) { }
		inline bool update(instance_t *ptr) { return false; }
		inline bool connected(data *d) { return false; }
		inline void update(data *d) { }
		inline void hookup_object(instance_t ptr, const char *path) { }
#else
		void init();
		data* connect();
		void disconnect(data *d);
		bool connected(data *d);
		void update(data *d);
		void hookup_object(instance_t ptr, const char *path);

		// returns true if updated, then pointer for new asset.
		bool update_ptr(instance_t *ptr);

		
#endif

		#define LIVE_UPDATE(x) putki::liveupdate::update_ptr((putki::instance_t*) x)

	}
}

#endif