#ifndef NXLUTILS_WINDOWS_HANDLE_H_INCLUDED
#define NXLUTILS_WINDOWS_HANDLE_H_INCLUDED
#include <Windows.h>
#include <string>
#include <vector>

// This makro assures that INVALID_HANDLE_VALUE (0xFFFFFFFF) returns FALSE
#define IsConsoleHandle(h) (((((ULONG_PTR)h) & 0x10000003) == 0x3) ? TRUE : FALSE)

DWORD GetGrantAccessFromHandle(HANDLE handle, DWORD *grantAccess);
DWORD GetNtPathFromHandle(HANDLE handle, std::wstring &ntPath);
DWORD GetDosPathFromNtPath(const std::wstring &ntPath, std::wstring &dosPath);
std::vector<HANDLE> GetHandlesFromProcess(DWORD pid);
#endif