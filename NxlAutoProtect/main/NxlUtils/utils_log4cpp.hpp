#pragma once

#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <tchar.h>

#define LOGGER_PERF log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("PerformanceMonitor"))
#define LOGGER_MONITOR log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("CallMonitor"))
#define LOGGER_PROJECT log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(PROJECT_NAME))

#ifndef DEFAULT_LOGGER
#define DEFAULT_LOGGER LOGGER_PROJECT
#endif

#define TRACELOG(fmt, ...)  LOG4CPLUS_TRACE_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT(__FUNCTION__) LOG4CPLUS_TEXT("():")  LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)
#define DBGLOG(fmt, ...) LOG4CPLUS_DEBUG_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT(__FUNCTION__) LOG4CPLUS_TEXT("():") LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)
#define INFOLOG(fmt, ...) LOG4CPLUS_INFO_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)
#define WARLOG(fmt, ...) LOG4CPLUS_WARN_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)
#define ERRLOG(fmt, ...) LOG4CPLUS_ERROR_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)
#define FATALLOG(fmt, ...) LOG4CPLUS_FATAL_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)

#define LOGLOG(loggerName, level, fmt, ...) LOG4CPLUS_##level##_FMT(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(loggerName)), LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__)

#define DBGVLOG(fmt, ...) {TCHAR sMsgBuf[1024];_stprintf(sMsgBuf, TEXT(PROJECT_NAME) TEXT("!") TEXT(fmt) TEXT("\n"), ##__VA_ARGS__);OutputDebugString(sMsgBuf);}


//make name to string
#define __stringify(str) #str
#define STRINGIFY(var) __stringify(var)

#define __LINE_STR__ STRINGIFY(__LINE__)

#define __WTEXT(quote) L##quote
#define WTEXT(quote) __WTEXT(quote)

#define LOG_PERF(call) \
	{	\
		LOG4CPLUS_TRACE_FMT(LOGGER_PERF, LOG4CPLUS_TEXT("Calling ") LOG4CPLUS_TEXT(#call));	\
		call;	\
		LOG4CPLUS_TRACE_FMT(LOGGER_PERF, LOG4CPLUS_TEXT("Finished ") LOG4CPLUS_TEXT(#call));	\
	}

#define LOG_CALLED(call) \
	{	\
		call;	\
		LOG4CPLUS_TRACE_FMT(LOGGER_MONITOR, LOG4CPLUS_TEXT("Finished ") LOG4CPLUS_TEXT(#call));	\
	}
