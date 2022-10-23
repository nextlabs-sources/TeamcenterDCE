#include "hook_windows.h"
#include <io.h>
#include "hook_log.h"
#include "hook_utils.h"
#include <fcntl.h>
/*
	CreateFileA and CreateFileW
*/
pfCreateFileA CreateFileA_original = NULL;
pfCreateFileA CreateFileA_next = NULL;
HANDLE WINAPI CreateFileA_hooked(LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	HANDLE hFile = NULL;
	hFile = CreateFileA_next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	//pre
	if(IS_PRT_A(lpFileName))
	{
		switch(dwDesiredAccess)
		{
			case GENERIC_READ:
				DBGLOG("(%ul)Reading File(%p)-%s", dwDesiredAccess, hFile, lpFileName);
				break;
			case GENERIC_WRITE:
				DBGLOG("(%ul)Writing File(%p)-%s", dwDesiredAccess, hFile, lpFileName);
				break;
			case GENERIC_READ|GENERIC_WRITE:
			default:
				DBGLOG("(%ul)Read&Write File(%p)-%s", dwDesiredAccess, hFile, lpFileName);
				break;
		}
	}
	//post
	//DBGLOG("CreateFile(%s) returns %d", lpFileName, hFile);
	return hFile;
}

pfCreateFileW CreateFileW_original =  NULL;
pfCreateFileW CreateFileW_next = NULL;
HANDLE WINAPI CreateFileW_hooked(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	HANDLE hFile = NULL;
	//pre
	hFile = CreateFileW_next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	if(IS_PRT_W(lpFileName))
	{
		switch(dwDesiredAccess)
		{
			case GENERIC_READ:
				DBGLOGW("(%ul)Reading File(%p)-%s", dwDesiredAccess, hFile, lpFileName);
				break;
			case GENERIC_WRITE:
				DBGLOGW("(%ul)Writing File(%p)-%s", dwDesiredAccess, hFile, lpFileName);
				break;
			case GENERIC_READ|GENERIC_WRITE:
			default:
				DBGLOGW("(%ul)Read&Write File(%p)-%s", dwDesiredAccess, hFile, lpFileName);
				break;
		}
	}
	//post
	//DBGLOGW("CreateFile(%s) returns %d", lpFileName, hFile);
	return hFile;
}
/*
	DeleteFileA and DeleteFileW
*/
pfDeleteFileA DeleteFileA_original = NULL;
pfDeleteFileA DeleteFileA_next = NULL;
pfDeleteFileW DeleteFileW_original = NULL;
pfDeleteFileW DeleteFileW_next = NULL;
BOOL WINAPI DeleteFileA_hooked(LPCSTR lpFileName)
{
	if(IS_PRT_A(lpFileName))
	{
		DBGLOG("Deleting '%s'...", lpFileName);
	}
	if(DeleteFileA_next(lpFileName))
	{
		//DBGLOG("==>DELETED");
		return TRUE;
	}
	else
	{
		DBGLOG("==>Failed(%d)", GetLastError());
		return FALSE;
	}
}
BOOL WINAPI DeleteFileW_hooked(LPCWSTR lpFileName)
{
	if(IS_PRT_W(lpFileName))
	{
		DBGLOGW("Deleting '%s'...", lpFileName);
	}
	if(DeleteFileW_next(lpFileName))
	{
		//DBGLOGW("==>DELETED");
		return TRUE;
	}
	else
	{
		DBGLOGW("==>Failed(%d)", GetLastError());
		return FALSE;
	}
}
/*
	MoveFileA and MoveFileW
*/
pfMoveFileA MoveFileA_original = NULL;
pfMoveFileA MoveFileA_next = NULL;
pfMoveFileW MoveFileW_original = NULL;
pfMoveFileW MoveFileW_next = NULL;
BOOL WINAPI MoveFileA_hooked(LPCSTR lpExistingFileName,  LPCSTR lpNewFileName)
{
	if(IS_PRT_A(lpExistingFileName) || IS_PRT_A(lpNewFileName))
	{
		DBGLOG("Moving '%s' to '%s'...", lpExistingFileName, lpNewFileName);
	}
	if(MoveFileA_next(lpExistingFileName, lpNewFileName))
	{
		//DBGLOG("==>Moved");
		return TRUE;
	}
	else
	{
		DBGLOG("==>Failed(%d)", GetLastError());
		return FALSE;
	}
}
BOOL WINAPI MoveFileW_hooked(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	if(IS_PRT_W(lpExistingFileName) || IS_PRT_W(lpNewFileName))
	{
		DBGLOGW("Moving '%s' to '%s'...", lpExistingFileName, lpNewFileName);
	}
	if(MoveFileW_next(lpExistingFileName, lpNewFileName))
	{
		DBGLOGW("==>Moved");
		return TRUE;
	}
	else
	{
		DBGLOGW("==>Failed(%d)", GetLastError());
		return FALSE;
	}
}
/*
	CopyFileA and CopyFileW
*/
pfCopyFileA CopyFileA_original = NULL;
pfCopyFileA CopyFileA_next = NULL;
pfCopyFileW CopyFileW_original = NULL;
pfCopyFileW CopyFileW_next = NULL;
BOOL WINAPI CopyFileA_hooked(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
	if(IS_PRT_A(lpExistingFileName) || IS_PRT_A(lpNewFileName))
	{
		DBGLOG("Copying '%s' to '%s' (FailIfExists=%d)...", lpExistingFileName, lpNewFileName, bFailIfExists);
	}
	if(CopyFileA_next(lpExistingFileName, lpNewFileName, bFailIfExists))
	{
		//DBGLOG("==>Copied");
		return TRUE;
	}
	else
	{
		DBGLOG("==>Failed(%d)", GetLastError());
		return FALSE;
	}
}

BOOL WINAPI CopyFileW_hooked(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
	if(IS_PRT_W(lpExistingFileName) || IS_PRT_W(lpNewFileName))
	{
		DBGLOGW("Copying '%s' to '%s' (FailIfExists=%d)...", lpExistingFileName, lpNewFileName, bFailIfExists);
	}
	if(CopyFileW_next(lpExistingFileName, lpNewFileName, bFailIfExists))
	{
		//DBGLOGW("==>Copied");
		return TRUE;
	}
	else
	{
		DBGLOGW("==>Failed(%d)", GetLastError());
		return FALSE;
	}
}
