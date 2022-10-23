// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <Shlwapi.h>
#include <Psapi.h>
#include <stdio.h>

#include <string>
#include <vector>

// TODO: reference additional headers your program requires here

extern WCHAR sMsgBuf[1024];
extern bool enableConsoleLog;
#define DBGLOG(fmt, ...) {wsprintf(sMsgBuf, TEXT("nxlrunner!") TEXT(fmt), ##__VA_ARGS__);OutputDebugString(sMsgBuf); if(enableConsoleLog)wprintf(L"%s\n", sMsgBuf);}
#ifdef DETAILED
#define DTLLOG(fmt, ...) DBGLOG(fmt, ##__VA_ARGS__)
#else
#define DTLLOG(fmt, ...) 
#endif

#define IS_NXL_EXT(ext) (0 == _wcsicmp(ext, L".nxl"))

#include <RPMUtils/RPMHelper.hpp>

extern RPMHelper *g_rpmHelper;

__int64 GetLastWriteTime(LPCTSTR lpszFile);
