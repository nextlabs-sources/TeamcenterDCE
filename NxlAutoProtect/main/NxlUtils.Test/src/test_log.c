#include "test.h"
#include "utils_log.h"
#include "utils_string.h"

#define ASSERT_LOG(act, exp, r, cid) if(string_ends_with(act, exp, TRUE)){PASS(r);}else{OUTPUT("%s():[%d]Actual("#act")='%s' Expect("#exp")='%s'(Line=%d)", __FUNCTION__, cid, act, exp, __LINE__);FAIL(r);}
result_t test_log()
{
	const char *log;
	char buffer[1024];
	BOOL bl;
	int console=0;
	int dbg = 0;
	int file = 0;
	result_t ret;
	INIT(ret);

	bl = initial_log(gent_file_path("test_log.log"));
	ASSERT(bl, TRUE, ret, __LINE__);

	log = DBGLOG("This message has no argument");
	ASSERT_LOG(log, "This message has no argument", ret, __LINE__);	dbg++;	file++;
	
	log = DBGLOG("This message has 1 argument-'%s'", "ABC");
	ASSERT_LOG(log, "This message has 1 argument-'ABC'", ret, __LINE__);	dbg++;	file++;
	
	log = DBGLOG("This message has 2 argument-'%s', '%s'", "ABC", "DEFG");
	ASSERT_LOG(log, "This message has 2 argument-'ABC', 'DEFG'", ret, __LINE__);	dbg++;	file++;

#ifdef DEBUG
	log = DTLLOG("this is a '%s' log", "detail");
	ASSERT_LOG(log, "this is a 'detail' log\n", ret, __LINE__);	dbg++;	file++;
#else
	ASSERT_LOG(log, "", ret, __LINE__);	dbg++;	file++;
#endif
	log = SYSLOG("==>this is a '%s' log", "system");
	ASSERT_LOG(log, "==>this is a 'system' log", ret, __LINE__);	dbg++;	file++; console++;
	log = ERRLOG("==>this is a '%s' log", "error");
	strcpy(buffer, log);
	ASSERT_LOG(buffer, "==>this is a 'error' log", ret, __LINE__); dbg++; file++; console++;

	log = nxl_log(TO_DBGVIEW, "DEBUG!This log should be output to %s", "dbgviewer"); dbg++;
	ASSERT_LOG(log, "This log should be output to dbgviewer", ret, __LINE__);
	log = nxl_log(TO_LOGFILE, "DEBUG!This log should be output to %s", "logfile"); file++;
	ASSERT_LOG(log, "This log should be output to logfile", ret, __LINE__);
	log = nxl_log(TO_CONSOLE, "DEBUG!==>This log should be output to %s", "console"); console++;
	ASSERT_LOG(log, "==>This log should be output to console", ret, __LINE__);
	
	OUTPUT("==>DdbViewer:%d LogFile:%d Console:%d", dbg, file, console);

	bl = initial_log(gent_file_path("test_log2.log"));
	ASSERT(bl, TRUE, ret, __LINE__);
	log = nxl_log(TO_LOGFILE, "DEBUG!This log should be output to %s", "logfile2");
	ASSERT_LOG(log, "This log should be output to logfile2", ret, __LINE__);
	return ret;
}