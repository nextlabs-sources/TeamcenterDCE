// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include <stdio.h>
#include <uf.h>
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#ifdef WNT
#include <Windows.h>
#else
#include <utils_string.h>
#define OutputDebugString(msg) 
#endif

extern char sMsgBuf[1024];

#define OUTPUT(msg) {std::cout << msg;OutputDebugString(msg); UF_print_syslog(msg, FALSE);}

#define NXSYS(fmt, ...) {sprintf_s(sMsgBuf, sizeof(sMsgBuf), PROJECT_NAME "!%s():" fmt "\n", __FUNCTION__, ##__VA_ARGS__);OUTPUT(sMsgBuf); }
#define NXDBG(fmt, ...) {sprintf_s(sMsgBuf, sizeof(sMsgBuf), PROJECT_NAME "!DEBUG !%s():" fmt "\n", __FUNCTION__, ##__VA_ARGS__);OUTPUT(sMsgBuf);}
#define NXERR(fmt, ...) {sprintf_s(sMsgBuf, sizeof(sMsgBuf), PROJECT_NAME "!ERROR!%s():" fmt "\n", __FUNCTION__, ##__VA_ARGS__);OUTPUT(sMsgBuf);}
#define NXWAR(fmt, ...) {sprintf_s(sMsgBuf, sizeof(sMsgBuf), PROJECT_NAME "!WARN !%s():" fmt "\n", __FUNCTION__, ##__VA_ARGS__);OUTPUT(sMsgBuf);}

#define DBGLOG(fmt, ...) {sprintf_s(sMsgBuf, sizeof(sMsgBuf), PROJECT_NAME "!DEBUG !%s():" fmt "\n", __FUNCTION__, ##__VA_ARGS__);OUTPUT(sMsgBuf);}

//
//Debug Utilities
//
int report_error(const char *file, int line, const char *func, const char *call, int ret);
//TRUE=success FALSE=ERROR
#define NX_EVAL(X) (!report_error( __FILE__, __LINE__, __FUNCTION__, #X, (X)))
#define NX_CALL(X) report_error( __FILE__, __LINE__, __FUNCTION__, #X, (X))


extern void RegisterImportListeners();
extern void RegisterCloneNotifyCallbacks();

extern std::vector<std::string> load_cmd_args();
extern bool teamcenter_protect(const char *partNumber, const char *partRev, const char *partFileType, const char *partFileName);