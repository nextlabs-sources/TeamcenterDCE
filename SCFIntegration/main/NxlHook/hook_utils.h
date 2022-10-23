#ifndef NXL_HOOK_UTILS_H_INCLUDED
#define NXL_HOOK_UTILS_H_INCLUDED
#include <Windows.h>
#include "hook_log.h"

BOOL StringEndsWithA(LPCSTR str, LPCSTR suffix, BOOL cs);
BOOL StringEndsWithW(LPCWSTR str, LPCWSTR suffix, BOOL cs);

#define IS_PRT_A(f) StringEndsWithA(f, ".prt", FALSE)
#define IS_PRT_W(f) StringEndsWithW(f, L".prt", FALSE)
#ifdef UNICODE
#define StringEndsWith StringEndsWithW
#define IS_PRT IS_PRT_W
#else
#define StringEndsWith StringEndsWithA
#define IS_PRT IS_PRT_A
#endif

const char *GetPathDirectory(const char *fullPath);
const char *GetPathFileName(const char *fullPath, BOOL withExt);

const char *time_tostring(time_t time);
const char *get_utils_dir();
BOOL string_isNullOrSpace(const char *str);
BOOL get_module_dir(const char *moduleName, char *dirBuf, int bufSize);

#endif

#define NXL_ALLOC(ptr, t) (ptr = (t*)malloc(sizeof(t)))
