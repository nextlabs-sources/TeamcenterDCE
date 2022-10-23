#ifndef HOOKDLL_CREATE_FILE_H_INCLUDED
#define HOOKDLL_CREATE_FILE_H_INCLUDED
#include <Windows.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
_stat64
*/

typedef int (__cdecl *pf_stat64i32)(const char * _Name, struct _stat64i32 * _Stat);
/*
FindFirstFile
*/
typedef HANDLE (WINAPI *pfFindFirstFileA)(
  LPCTSTR           lpFileName,
  LPWIN32_FIND_DATA lpFindFileData
);
typedef HANDLE (WINAPI *pfFindFirstFileW)(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
    );

/*
FindNextFile
*/
typedef BOOL (WINAPI *pfFindNextFileW)(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
    );
/*
	CreateFileA and CreateFileW
*/
typedef HANDLE (WINAPI *pfCreateFileA)(
	LPCSTR lpFileName, 
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, 
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

extern pfCreateFileA CreateFileA_original;
extern pfCreateFileA CreateFileA_next;
HANDLE WINAPI CreateFileA_hooked(LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);

typedef HANDLE (WINAPI *pfCreateFileW)(
	LPCWSTR lpFileName, 
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, 
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

extern pfCreateFileW CreateFileW_original;
extern pfCreateFileW CreateFileW_next;
HANDLE WINAPI CreateFileW_hooked(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);

/*
	DeleteFileA and DeleteFileW
*/
typedef BOOL (WINAPI *pfDeleteFileA)(
    LPCSTR lpFileName
    );
typedef BOOL (WINAPI *pfDeleteFileW)(
    LPCWSTR lpFileName
    );
extern pfDeleteFileA DeleteFileA_original;
extern pfDeleteFileA DeleteFileA_next;
extern pfDeleteFileW DeleteFileW_original;
extern pfDeleteFileW DeleteFileW_next;
BOOL WINAPI DeleteFileA_hooked(
    LPCSTR lpFileName
    );
BOOL WINAPI DeleteFileW_hooked(
    LPCWSTR lpFileName
    );

/*
	MoveFileA and MoveFileW
*/
typedef BOOL (WINAPI *pfMoveFileA)(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    );
typedef BOOL (WINAPI *pfMoveFileW)(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    );
extern pfMoveFileA MoveFileA_original;
extern pfMoveFileA MoveFileA_next;
extern pfMoveFileW MoveFileW_original;
extern pfMoveFileW MoveFileW_next;
BOOL WINAPI MoveFileA_hooked(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    );
BOOL WINAPI MoveFileW_hooked(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    );

/*
	CopyFileA and CopyFileW
*/
typedef BOOL (WINAPI *pfCopyFileA)(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    );

typedef BOOL (WINAPI *pfCopyFileW)(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    );
extern pfCopyFileA CopyFileA_original;
extern pfCopyFileA CopyFileA_next;
extern pfCopyFileW CopyFileW_original;
extern pfCopyFileW CopyFileW_next;
BOOL WINAPI CopyFileA_hooked(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    );

BOOL WINAPI CopyFileW_hooked(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    );
/*
	ReadFile
*/
typedef BOOL (WINAPI *pfReadFile)(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
);
extern pfReadFile ReadFile_original;
extern pfReadFile ReadFile_next;
BOOL WINAPI ReadFile_hooked(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
);
/*
	LoadLibraryA and LoadLibraryW
*/
typedef HMODULE (WINAPI *pfLoadLibraryA)(
    LPCSTR lpLibFileName
    );
typedef HMODULE (WINAPI *pfLoadLibraryW)(
    LPCWSTR lpLibFileName
    );
extern pfLoadLibraryA LoadLibraryA_original;
extern pfLoadLibraryA LoadLibraryA_next;
HMODULE WINAPI LoadLibraryA_hooked(LPCSTR lpLibFileName);

extern pfLoadLibraryW LoadLibraryW_original;
extern pfLoadLibraryW LoadLibraryW_next;
HMODULE WINAPI LoadLibraryW_hooked(LPCWSTR lpLibFileName);




//
//	CloseHandle
//
#define CloseHandle_ORDINAL 54
typedef BOOL (WINAPI *pfCloseHandle)(HANDLE hnd);

#endif