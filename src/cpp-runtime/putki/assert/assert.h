#ifndef __PUTKI_RT_ASSERT_H__
#define __PUTKI_RT_ASSERT_H__

#include <putki/log/log.h>

#if 1
//defined(PUTKI_ENABLE_ASSERT)

	#define PTK_LOG(type, stmt) { \
		std::stringstream __DPRINT_LINE; \
		__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): " << stmt; \
		putki::log(type, __DPRINT_LINE.str().c_str()); \
	} \

	#define PTK_ASSERT(x) if (!(x)) PTK_WARNING("Assertion failed!")
	#define PTK_ASSERT_DESC(x, y) if (!(x)) PTK_WARNING("Assertion failed: " << y)
	#define PTK_FATAL_ASSERT(x) if (!(x)) PTK_ERROR("Fatal assert!")
	#define PTK_FATAL_ASSERT_DESC(x, y) if (!(x)) PTK_ERROR("Fatal assert: " << y)

#else

	#define PTK_ASSERT(x) {}
	#define PTK_ASSERT_DESC(x, y) {}
	#define PTK_FATAL_ASSERT(x) {}
	#define PTK_FATAL_ASSERT_DESC(x, y) {}

#endif

#endif
