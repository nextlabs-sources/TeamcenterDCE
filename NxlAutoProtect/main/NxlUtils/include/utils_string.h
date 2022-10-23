#ifndef NXL_UTILS_STRING_H_INCLUDED
#define NXL_UTILS_STRING_H_INCLUDED
#include "nxl_utils_exports.h"
#include "utils_common.h"
#include "utils_string_list.h"
#include <string.h>

#if defined(WNT)
#include <Shlwapi.h>

#define PATH_DELIMITER "\\"
#define PATH_DELIMITER_CHAR '\\'

#elif defined(__linux__)

#define PATH_DELIMITER "/"
#define PATH_DELIMITER_CHAR '/'

#define strcpy_s(des, dsize, src) strcpy(des, src)
#define strncpy_s(des, dsize, src, cnt) strncpy(des, src, cnt)
#define strcat_s(des, dsize, src) strcat(des, src)
#define _stricmp(a, b) strcasecmp(a, b)
#define stricmp(a, b) strcasecmp(a, b)
#define CharLower(str) string_toLower2(str)
#define PathFindExtension(str) path_find_ext(str)
#define PathFindFileName(str) path_find_name((char*)str)
#define PathRemoveFileSpec(str) path_remove_name(str)

#define sprintf_s(des, dsize, fmt, ...) sprintf(des, fmt, __VA_ARGS__)
#define _snprintf_s(buf, bufSize, mx, fmt, ...) snprintf(buf, bufSize-1, fmt, __VA_ARGS__)
#define vsnprintf_s(des, dsize, max, fmt, valist) vsnprintf(des, max, fmt, valist)

#define sscanf_s(buf, fmt,...) sscanf(buf, fmt, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/*
	strings related utilities
*/
NXLAPI int strings_index_of(char * const *strs, const int size, const char *str);


/*
	string related utilities
*/
NXLAPI BOOL string_starts_with(const char *str, const char *prefix);
NXLAPI BOOL string_ends_with(const char *str, const char *suffix, int cs);
NXLAPI BOOL string_isNullOrSpace(const char *str);
NXLAPI BOOL string_isTrue(const char *str);
NXLAPI BOOL string_wildcmp(const char *strToCmp, const char *patternWithWildcard);

//somehow if you call free() in the external dll on the string which is strdup() in NxlUtils.dll
//the external dll will crash, cal free() in the same dll would solve the problem
NXLAPI void string_free(char *str);
NXLAPI char *string_dup(const char *str);
//The returned string of following APIs need to be freed by string_free()
NXLAPI char *string_trim(const char *str);
NXLAPI char *string_toLower(const char *str);
NXLAPI char *string_toLower2(char *str);
NXLAPI int string_lastIndexOf(const char *str, char c);

NXLAPI char *path_find_ext(char *path);
NXLAPI char *path_find_name(char *path);
NXLAPI BOOL path_remove_name(char *path);


//Result string is stored in a static array which will be override in next call
//Please create a copy in local if need to perserve
NXLAPI const char *strings_join_v(const char *split,int argNum, ...);
NXLAPI const char *strings_join(char * const *strings, int count,const char *split);

//NXLAPI void strings_free(char **strings, int cnt);

#ifdef __cplusplus
}
#endif

#endif