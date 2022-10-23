#ifndef NXL_HOOK_WINDOWS_DEFS_H_INCLUDED
#define NXL_HOOK_WINDOWS_DEFS_H_INCLUDED
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

typedef HANDLE (WINAPI *pfCreateFileW)(
	LPCWSTR lpFileName, 
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, 
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

/*
	DeleteFileA and DeleteFileW
*/
typedef BOOL (WINAPI *pfDeleteFileA)(
    LPCSTR lpFileName
    );
typedef BOOL (WINAPI *pfDeleteFileW)(
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
/*
MoveFileExA and MoveFileExW
*/
typedef
BOOL (WINAPI *pfMoveFileExA)(
	__in     LPCSTR lpExistingFileName,
	__in_opt LPCSTR lpNewFileName,
	__in     DWORD    dwFlags
);
typedef
BOOL(WINAPI *pfMoveFileExW)(
	__in     LPCWSTR lpExistingFileName,
	__in_opt LPCWSTR lpNewFileName,
	__in     DWORD    dwFlags
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

/*
	LoadLibraryA and LoadLibraryW
*/
typedef HMODULE (WINAPI *pfLoadLibraryA)(
    LPCSTR lpLibFileName
    );
typedef HMODULE (WINAPI *pfLoadLibraryW)(
    LPCWSTR lpLibFileName
    );

//
//	CloseHandle
//
typedef BOOL (WINAPI *pfCloseHandle)(HANDLE hnd);

//
// CreateProcess
//
typedef BOOL (WINAPI *pfCreateProcessA)(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);
typedef BOOL (WINAPI *pfCreateProcessW)(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
    );
#endif