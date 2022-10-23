#pragma once
#include <windows.h>
#if _MSC_VER >= 1900
#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#endif
#include <Delayimp.h>  
#include <stdio.h> 
#include <string>
#include <vector>

#pragma comment(lib, "Delayimp.lib")

#define ENABLE_DELAYLOAD_HOOK \
PfnDliHook __pfnDliNotifyHook2 = DelayLoadHook;

#define ADD_DELAYLOAD_DLL(dllPath) g_delayLoadDlls.push_back(dllPath)

#ifdef __cplusplus
extern "C"
{
#endif

FARPROC WINAPI DelayLoadHook(unsigned dliNotify, PDelayLoadInfo pdli);

extern std::vector<std::wstring> g_delayLoadDlls;

extern void UnloadDelayLoadedDlls();

#ifdef __cplusplus
}
#endif

