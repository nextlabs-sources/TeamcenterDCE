#ifndef NXL_UTILS_PLATFORM_INDEPENDENT_H_INCLUDED
#define NXL_UTILS_PLATFORM_INDEPENDENT_H_INCLUDED
#include <stdio.h>
#include <nxl_utils_exports.h>
#include <utils_common.h>
#include "utils_string.h"

/*
	Common API
*/

#ifdef __cplusplus
extern "C"
{
#endif

//get local ip in string format
NXLAPI char *IP_get_local();

//get local host name
NXLAPI const char *host_get_name();

#ifdef __cplusplus
}
#endif


//apis for Windows
#if defined(WNT)

#include <utils_windows.h>

#elif defined(__linux__)
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h> 
//apis for Linux

typedef unsigned long       DWORD;

#define GetLastError() 0
#define GetCurrentThreadId() 0
#define GetTempPath(bufSize, buf) sprintf(buf, "/tmp/")

#define fopen_s(pf, file, mode) !(*pf = fopen(file, mode))

#define MAX_PATH 260

typedef struct _user_info_s
{
	const char *name;
	const char *sid;
} const *user_info_ro;

#ifdef __cplusplus
extern "C"
{
#endif
	
//get logon user info
NXLAPI user_info_ro get_logon_info();

NXLAPI BOOL get_lib_dir(void *funcPtr, char *buf, int bufSize);

NXLAPI BOOL file_exist(const char *fileName);
NXLAPI BOOL file_copy(const char *src, const char *tar);
NXLAPI string_list_p dir_list_files(const char *dir);


//<lines> must be NULL or allocated before this call
NXLAPI BOOL process_read_output(char *cmd, string_list_p lines, int timeout);

#ifdef __cplusplus
}
#endif

#endif

#endif