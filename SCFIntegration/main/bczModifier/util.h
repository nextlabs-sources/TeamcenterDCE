#ifndef __UTIL_H__
#define __UTIL_H__
#include <Windows.h>

const char *time_tostring(time_t time);
const char *get_utils_dir();
BOOL string_isNullOrSpace(const char *str);
BOOL get_module_dir(const char *moduleName, char *dirBuf, int bufSize);
#endif
