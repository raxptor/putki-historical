#ifndef __PUTKI_RT_LOG_H__
#define __PUTKI_RT_LOG_H__

namespace putki
{
	enum LogType
	{
		LOG_NOTHING = 0,
		LOG_DEBUG   = 1,
		LOG_WARNING = 2,
		LOG_ERROR   = 3
	};

	void log(LogType type, const char *msg);
	void set_loglevel(LogType filter);
}

#if defined(PUTKI_ENABLE_LOG)

	#include <sstream>

	#define PTK_LOG(type, stmt) { \
		std::stringstream __DPRINT_LINE; \
		__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): " << stmt; \
		putki::log(type, __DPRINT_LINE.str().c_str()); \
	} \

	#define PTK_DEBUG(x) PTK_LOG(putki::LOG_DEBUG, x)
	#define PTK_WARNING(x) PTK_LOG(putki::LOG_WARNING, x)
	#define PTK_ERROR(x) PTK_LOG(putki::LOG_ERROR, x)

#else
	#define PTK_DEBUG(x) { }
	#define PTK_WARNING(x) { }
	#define PTK_ERROR(x) { }
#endif

#endif
