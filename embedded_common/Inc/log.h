#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <stdarg.h>

enum {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL
};

#ifndef DISABLE_LOGGING
#define LOG_TRACE(...) log_log(LOG_TRACE, __FUNCTION__, sizeof(__FUNCTION__), __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(LOG_DEBUG, __FUNCTION__, sizeof(__FUNCTION__), __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_log(LOG_INFO,  __FUNCTION__, sizeof(__FUNCTION__), __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_log(LOG_WARN,  __FUNCTION__, sizeof(__FUNCTION__), __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(LOG_ERROR, __FUNCTION__, sizeof(__FUNCTION__), __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(LOG_FATAL, __FUNCTION__, sizeof(__FUNCTION__), __LINE__, __VA_ARGS__)
#else
#define LOG_TRACE(...)
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)
#endif

void log_set_level(int level);

void log_log(int level, const char *function, int function_len, int line, const char *fmt, ...);

#endif /* LOG_H_ */
