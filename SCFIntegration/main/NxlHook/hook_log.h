#ifndef NXL_HOOK_LOG_H_INCLUDED
#define NXL_HOOK_LOG_H_INCLUDED
#include <stdio.h>
#include <Windows.h>
#include "hook_utils.h"

//make name to string
#define __stringify(str) #str
#define STRINGIFY(var) __stringify(var)

#define __LINE_STR__ STRINGIFY(__LINE__)

#define TO_NULL 0
#define TO_CONSOLE 1
#define TO_DBGVIEW 2
#define TO_LOGFILE 4

#if defined(WNT)
#include <Shlwapi.h>

#define PATH_DELIMITER "\\"
#define PATH_DELIMITER_CHAR '\\'

#elif defined(__linux__)

#define PATH_DELIMITER "/"
#define PATH_DELIMITER_CHAR '/'

#endif

//#endif

#ifdef __cplusplus
extern "C"
{
#endif

const char *get_log_file();
BOOL initial_log(const char *logFile);
void writemsg(const char *file, const char *msg);
const char *build_message(const char *fmt, va_list args);
const char *build_timestamp(const char *msg);
const char *nxl_log (int target, const char *fmt,...);    // logs a message to LOGFILE
void init_nxl_scf_hook_log();

void PrintLogA(const char* fmt , ... ) ;
void PrintLogW(const wchar_t* fmt , ... ) ;

#ifdef __cplusplus
}
#endif

#define SYSLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME"!"fmt, ##__VA_ARGS__)
#define ERRLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME"!ERROR!"fmt"(File-'"__FILE__"' Line-"__LINE_STR__")", ##__VA_ARGS__)
#define DBGLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME"!DEBUG!%s():"fmt, __FUNCTION__, ##__VA_ARGS__)
#ifdef DEBUG
#define DTLLOG(fmt, ...) nxl_log(TO_DBGVIEW|TO_LOGFILE,	PROJECT_NAME"!DETAIL!%s():"fmt, __FUNCTION__, ##__VA_ARGS__)
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
#endif
/*
extern char logbuf[LOG_MAX];
#define DBGLOG(fmt, ...) sprintf_s(logbuf, LOG_MAX, "nxlhook!%d!"__FUNCTION__ "():"fmt, GetCurrentThreadId(), ##__VA_ARGS__);OutputDebugStringA(logbuf);
#ifdef DEBUG
#define DTLLOG(fmt, ...) DBGLOG(fmt, ##__VA_ARGS__)
#else
#define DTLLOG(fmt, ...)
#endif
*/
#define LOG_MAX (1024)
extern wchar_t logbufw[LOG_MAX];
extern wchar_t func[100];
extern size_t funcSize;
#define DBGLOGW(fmt, ...) swprintf_s(logbufw, LOG_MAX, WSTR("nxlhook!%d!")WSTR(__FUNCTION__)WSTR("():")WSTR(fmt), GetCurrentThreadId(), ##__VA_ARGS__);OutputDebugStringW(logbufw);


#define CALL_START(fmt, ...)	DBGLOG("HOOK-Start:"fmt"@"__FILE__, __VA_ARGS__);
#define CALL_NEXT(call)			\
	try	\
	{	\
		DBGLOG("HOOK-Calling("#call")@"__FILE__);	\
		call;						\
	}	\
	catch(...)	\
	{	\
		DBGLOG("Exception happened during calling '"#call"'@"__FILE__);	\
	}
#define CALL_END(fmt, ...)		DBGLOG("HOOK-End:"fmt"@"__FILE__, __VA_ARGS__);
