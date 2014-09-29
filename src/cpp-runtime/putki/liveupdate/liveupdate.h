#ifndef __PUTKI_RUNTIME_LIVEUPDATE_H__
#define __PUTKI_RUNTIME_LIVEUPDATE_H__

#include <putki/types.h>

#if defined(LIVEUPDATE_ENABLE)
	// This macro can be used for checks that should be compiled away when
	// live editing is not enabled.
	#define LIVEUPDATE_ISNULL(x) ((void*)x == 0)
#else
	#define LIVEUPDATE_ISNULL(x) false
#endif

namespace putki
{
	namespace liveupdate
	{
		struct data;
		
#if !defined(LIVEUPDATE_ENABLE)
		// null implementation
		inline void init() { }
		inline data* connect() { return 0; }
		inline void disconnect(data *d) { }
		inline bool update(instance_t *ptr) { return false; }
		inline bool connected(data *d) { return false; }
		inline void update(data *d) { }
		inline void hookup_object(instance_t ptr, const char *path) { }
		inline bool should_reconnect() { return false; }

		#define LIVE_UPDATE(x) false
#else
		// real
		void init();
		data* connect();
		void disconnect(data *d);
		bool connected(data *d);
		void update(data *d);
		void hookup_object(instance_t ptr, const char *path);
		// returns true if updated, then pointer for new asset.
		bool update_ptr(instance_t *ptr);

		#define LIVE_UPDATE(x) putki::liveupdate::update_ptr((putki::instance_t*) x)
#endif
	}
}

#endif