#ifndef NXL_UTILS_WINDOWS_H_INCLUDED
#define NXL_UTILS_WINDOWS_H_INCLUDED
#include "nxl_utils_exports.h"
#include "hashtable.h"
#include "utils_string.h"
#include <Windows.h>

typedef struct _user_info_s
{
	const char *name;
	const char *sid;
	const char *domain;
} const *user_info_ro;

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
typedef struct _service_info_s
{
	BOOL isInstalled;
	BOOL isRunning;
	char cmd[MAX_PATH];
}service_info_t;
#define QUERY_EXISTS 1
#define QUERY_STATUS 2
#define QUERY_CMD 4

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI BOOL report_win_error(long retCode, long successCode, const char *func, const char *file, int line, const char *call);

#define REG_CALL(call) report_win_error(call, ERROR_SUCCESS, __FUNCTION__, __FILE__, __LINE__, #call)
#define REPORT_ERROR(errCode, msg) report_win_error(errCode, ERROR_SUCCESS, __FUNCTION__, __FILE__, __LINE__, msg)

NXLAPI const char *REG_get(const char *key, const char *valKey);
NXLAPI DWORD REG_get_dword(const char *key, const char *valName, DWORD defaultValue);

NXLAPI user_info_ro get_logon_info();
NXLAPI int get_service_info(const char *serviceName, int query, service_info_t *result);

NXLAPI BOOL get_module_dir(const char *moduleName, char *buf, int bufSize);

NXLAPI BOOL module_query_ver(const char *moduleName, int *major, int *minor, int *maint, int *build);

//<lines> must be NULL or allocated before this call
NXLAPI BOOL process_read_output(char *cmd, string_list_p lines, int timeout);
//search child process by parent process id
NXLAPI BOOL process_search_child(DWORD parentId, DWORD *childPid, char nameBuffer[MAX_PATH]);

NXLAPI BOOL file_exist(const char *fileName);
NXLAPI BOOL file_copy(const char *src, const char *tar);

NXLAPI string_list_p dir_list_files(const char *dir);

NXLAPI WCHAR *string_to_wstr(const char *str);//TODO:
#ifdef __cplusplus
}
#endif

#endif