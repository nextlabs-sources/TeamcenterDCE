
#pragma once

#if !defined(LOG4CPLUS_ENABLE) 
#define LOG4CPLUS_ENABLE
#endif

#undef _PRJNAME
#define _PRJNAME "XTest"

#include <RMXLogger.h>

/*A log header for each session*/
#define LOG_BEGIN(fkRootDir) \
{ \
const wstring logConfigFile = fkRootDir + L"\\log4cplus.properties"; \
LOG_INITIALIZE2(L"", logConfigFile); \
}

#define LOG_END() LOG_SHUTDOWN();

#define XLOG_ERROR(logEvent) \
LOG_ERROR(logEvent);

#define XLOG_ERROR_STR(logEvent)  \
LOG_ERROR_STR(logEvent);

#define XLOG_ERROR_FMT(...) \
LOG_ERROR_FMT(__VA_ARGS__);