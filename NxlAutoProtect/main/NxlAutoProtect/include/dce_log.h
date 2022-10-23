#ifndef NXL_DCE_LOG_H_INCLUDED
#define NXL_DCE_LOG_H_INCLUDED
#include "utils_log.h"
#include <tc/tc_startup.h>

#define TC_LOG(to, fmt, ...) TC_write_syslog("%s\n", nxl_log(to, fmt, ##__VA_ARGS__))

#undef SYSLOG
//Write System log to console and teamcenter syslog without caller
#define SYSLOG(fmt, ...)  TC_LOG(TO_DBGVIEW|TO_LOGFILE, PROJECT_NAME "!" fmt, ##__VA_ARGS__)

#undef ERRLOG
//Write Error log to console and teamcenter syslog with caller
#define ERRLOG(fmt, ...)  TC_LOG(TO_DBGVIEW|TO_CONSOLE|TO_LOGFILE, PROJECT_NAME "!ERROR!" fmt "(File-'" __FILE__ "' Line-" __LINE_STR__ ")", ##__VA_ARGS__)

#define WARLOG(fmt, ...)  TC_LOG(TO_DBGVIEW|TO_LOGFILE, PROJECT_NAME "!WARN!%s():" fmt, __FUNCTION__, ##__VA_ARGS__)

#undef DBGLOG
//Write Debug log to debugview and teamcenter syslog
#define DBGLOG(fmt, ...)  TC_LOG(TO_LOGFILE, PROJECT_NAME "!DEBUG!%s():" fmt, __FUNCTION__, ##__VA_ARGS__)

#undef DTLLOG
//Write Debug log to debugview and teamcenter syslog
#define DTLLOG(fmt, ...)  TC_LOG(TO_LOGFILE, PROJECT_NAME "!DETAIL!%s():" fmt, __FUNCTION__, ##__VA_ARGS__)

#endif  /* NXL_DCE_LOG_H_INCLUDED */