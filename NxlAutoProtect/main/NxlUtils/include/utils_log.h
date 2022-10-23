
#ifndef NXL_UTILS_LOG_H_INCLUDED
#define NXL_UTILS_LOG_H_INCLUDED
#include "nxl_utils_exports.h"
#include "utils_common.h"
#include <stdarg.h>

//make name to string
#define __stringify(str) #str
#define STRINGIFY(var) __stringify(var)

#define __LINE_STR__ STRINGIFY(__LINE__)

#define __WTEXT(quote) L##quote
#define WTEXT(quote) __WTEXT(quote)

#define TO_NULL 0
#define TO_CONSOLE 1
#define TO_DBGVIEW 2
#define TO_LOGFILE 4

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI const char *log_get_file();
NXLAPI BOOL initial_log(const char *logFile);
NXLAPI void writemsg(const char *file, const char *msg);
NXLAPI const char *build_message(const char *fmt, va_list args);
NXLAPI const char *build_timestamp(const char *msg);
NXLAPI const char *nxl_log (int target, const char *fmt,...);    // logs a message to LOGFILE

#ifdef __cplusplus
}
#endif

#define SYSLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME "!" fmt, ##__VA_ARGS__)
#define ERRLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME "!ERROR!" fmt "(File-'" __FILE__ "' Line-" __LINE_STR__ ")", ##__VA_ARGS__)
#define DBGLOG(fmt, ...) nxl_log(TO_LOGFILE,	PROJECT_NAME "!DEBUG!%s():" fmt, __FUNCTION__, ##__VA_ARGS__)
#ifdef DEBUG
#define DTLLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME "!DETAIL!%s():" fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define DTLLOG(fmt, ...)
#endif

//Debug Helper
#ifdef DEBUG
//output detail log in DEBUG mode
#define CALL_DTL(x)	\
{	\
	DTLLOG("Executing==>%s @FILE: %s LINE: %d", #x, __FILE__, __LINE__);	\
	x;	\
}
#else
#define CALL_DTL(x) x
#endif

#define CALL_DBG(x)	\
{	\
	DBGLOG("Executing==>%s @FILE: %s LINE: %d", #x, __FILE__, __LINE__);	\
	x;	\
}
#endif  /* NXL_UTILS_LOG_H_INCLUDED */