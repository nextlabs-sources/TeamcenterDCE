#include "hook_windows_defs.h"
#include "hook_log.h"
#include "hook_defs.h"

static pfFindFirstFileA FindFirstFileA_original;
static pfFindFirstFileA FindFirstFileA_next;
static HANDLE FindFirstFileA_hooked(const char *lpFileName,  LPWIN32_FIND_DATA lpFindFileData)
{
	HANDLE result = NULL;
	CALL_START("FindFirstFileA('%s', %p)", lpFileName, lpFindFileData);
	CALL_NEXT(result = FindFirstFileA_next(lpFileName, lpFindFileData));
	CALL_END("FindFirstFileA('%s', %p) returns %d(name='%s')", lpFileName, lpFindFileData, result, lpFindFileData->cFileName);	
	return result;
}
static pfFindFirstFileW FindFirstFileW_original;
static pfFindFirstFileW FindFirstFileW_next;
static HANDLE FindFirstFileW_hooked(LPCWSTR lpFileName,   LPWIN32_FIND_DATAW lpFindFileData)
{
	HANDLE result = NULL;
	CALL_START("FindFirstFileW('%S', %p)", lpFileName, lpFindFileData);
	CALL_NEXT(result = FindFirstFileW_next(lpFileName, lpFindFileData));
	CALL_END("FindFirstFileW('%S', %p) returns %d(name='%S')", lpFileName, lpFindFileData, result, lpFindFileData->cFileName);	
	return result;
}
/*FindNextFileW*/
static pfFindNextFileW FindNextFileW_original;
static pfFindNextFileW FindNextFileW_next;
static BOOL FindNextFileW_hooked(HANDLE hFindFile,   LPWIN32_FIND_DATAW lpFindFileData)
{
	BOOL found = FALSE;
	CALL_START("FindNextFileW('%p', %p)", hFindFile, lpFindFileData);
	CALL_NEXT(found = FindNextFileW_next(hFindFile, lpFindFileData));
	CALL_END("FindNextFileW('%p', %p) returns %d(name='%S')", hFindFile, lpFindFileData, found, lpFindFileData->cFileName);
	return found;
}
/*
	4639:	BASE_open_part
	ApiName:	BASE_open_part
	FullName:	?BASE_open_part@@YAHPEBDPEAIPEAUUF_PART_load_status_s@@@Z
	UnDecorated:	int __cdecl BASE_open_part(char const * __ptr64,unsigned int * __ptr64,struct UF_PART_load_status_s * __ptr64)
*/
#define BASE_open_part_FULLNAME "?BASE_open_part@@YAHPEBDPEAIPEAUUF_PART_load_status_s@@@Z"
#define BASE_open_part_ORDINAL 4639
typedef int (*pfBASE_open_part)(char const * p1	// char const * __ptr64
	,unsigned int * p2	// unsigned int * __ptr64
	,CPP_PTR p3	// struct UF_PART_load_status_s * __ptr64
	);
static pfBASE_open_part BASE_open_part_original;
static pfBASE_open_part BASE_open_part_next;
static int BASE_open_part_hooked(char const * p1, unsigned int * p2, CPP_PTR p3)
{
	int ret = 0;
	CALL_START("BASE_open_part(p1='%s', *p2='%u', p3='%p')", p1, *p2, p3);
	CALL_NEXT(ret = BASE_open_part_next(p1, p2, p3));
	CALL_END("BASE_open_part(p1='%s', *p2='%u', p3='%p') returns '%d'", p1, *p2, p3, ret);
	return ret;
}
BOOL iges_install()
{
	 //HOOK(FindFirstFileA);
	 //HOOK(FindFirstFileW);
	 //HOOK(FindNextFileW);
	 HOOK_API("libpart.dll", BASE_open_part);
	 return TRUE;
}

void iges_uninstall()
{
	UNHOOK(FindFirstFileA);
	UNHOOK(FindFirstFileW);
	UNHOOK(FindNextFileW);
	UNHOOK(BASE_open_part);
}