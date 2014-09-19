#ifndef __BUILDER_LOG_H__
#define __BUILDER_LOG_H__

#include <sstream>

namespace putki
{
	enum LogType
	{
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR
	};
	
	void print_log(LogType level, const char *indent, const char *message);
	void print_blob(const char *message);
	bool check_filter(LogType);
	
	inline bool show_line(LogType lt)
	{
		return lt == LOG_ERROR;
	}
}

#define RECORD_LOG(target, type, stmt) { \
	std::ostringstream __DPRINT_LINE; \
	if (putki::check_filter(type)) \
	{ \
		if (putki::show_line(type)) \
			__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): "; \
		__DPRINT_LINE << stmt; \
		build_db::record_log(target, type, __DPRINT_LINE.str().c_str()); \
	} \
}

#define BUILD_LOG(target, type, stmt) { \
	if (putki::check_filter(type)) \
	{ \
		std::ostringstream __DPRINT_LINE; \
		if (putki::show_line(type)) \
			__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): "; \
		__DPRINT_LINE << stmt; \
		builder::record_log(target, type, __DPRINT_LINE.str().c_str()); \
	} \
}

#define APP_LOG(type, stmt) { \
	if (putki::check_filter(type)) \
	{ \
		std::ostringstream __DPRINT_LINE; \
		if (putki::show_line(type)) \
			__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): "; \
		__DPRINT_LINE << stmt; \
		putki::print_log(type, "GLOBAL", __DPRINT_LINE.str().c_str()); \
	} \
}

#define RECORD_INFO(target, stmt) RECORD_LOG(target, putki::LOG_INFO, stmt)
#define RECORD_DEBUG(target, stmt) RECORD_LOG(target, putki::LOG_DEBUG, stmt)
#define RECORD_WARNING(target, stmt) RECORD_LOG(target, putki::LOG_WARNING, stmt)
#define RECORD_ERROR(target, stmt) RECORD_LOG(target, putki::LOG_ERROR, stmt)

#define BUILD_INFO(target, stmt) BUILD_LOG(target, putki::LOG_INFO, stmt)
#define BUILD_DEBUG(target, stmt) BUILD_LOG(target, putki::LOG_DEBUG, stmt)
#define BUILD_ERROR(target, stmt) BUILD_LOG(target, putki::LOG_ERROR, stmt)

#define APP_DEBUG(stmt) APP_LOG(putki::LOG_DEBUG, stmt)
#define APP_INFO(stmt) APP_LOG(putki::LOG_INFO, stmt)
#define APP_WARNING(stmt) APP_LOG(putki::LOG_WARNING, stmt)
#define APP_ERROR(stmt) APP_LOG(putki::LOG_ERROR, stmt)

#endif
