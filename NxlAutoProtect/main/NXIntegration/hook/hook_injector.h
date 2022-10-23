#ifndef NXL_HOOK_DLL_INJECTOR_H_INCLUDED
#define NXL_HOOK_DLL_INJECTOR_H_INCLUDED

BOOL inject_dll(DWORD pid, const char *hookDll);
BOOL inject_on_CreateProcess(const char *dllToBeInjected);
#endif