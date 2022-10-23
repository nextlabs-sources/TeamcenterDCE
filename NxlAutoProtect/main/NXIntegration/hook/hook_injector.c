#include <Windows.h>
#include <NxlUtils/utils_log4cpp.hpp>
#include <utils_nxl.h>
#include <utils_windows.h>
#include <NxlUtils\NxlPath.hpp>

#include <hook/hook_defs.h>
#include <hook/windows/windows_defs.h>
#include <RPMUtils/RPMUtils.h>

extern RPMSession *g_rpmSession;

BOOL inject_dll(DWORD pid, const char *hookDll)
{
	BOOL injected = FALSE;
	if(hookDll != NULL)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if(hProcess != NULL)
		{
			size_t size = strlen(hookDll) + 1;
			// Allocate space in the process for our DLL 
			LPVOID dllPathInRemote = (LPVOID)VirtualAllocEx(hProcess, NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); 
			if(dllPathInRemote != NULL)
			{
				// Write the string name of our DLL in the memory allocated 
				if(WriteProcessMemory(hProcess, dllPathInRemote, hookDll, size, NULL))
				{
					DWORD remoteThreadId;
					LPVOID loadLibAddress = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
					if(NULL != CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)loadLibAddress, dllPathInRemote, NULL, &remoteThreadId))
					{
						DBGLOG("RemoteThread Created(%lu) in Process-%p", remoteThreadId, hProcess);
						injected = TRUE;
					}
					else
					{
						ERRLOG("CreateRemoteThread() failed: %d", GetLastError());
					}
				}
				else
				{
					ERRLOG("WriteProcessMemory() failed: %d", GetLastError());
				}
			}
			else
			{
				ERRLOG("VirtualAllocEx() failed: %d", GetLastError());
			}
			CloseHandle(hProcess);
		}
		else
		{
			ERRLOG("OpenProcess(%lu) failed: %d", pid, GetLastError());
		}
	}
	return injected;
}
DWORD _last_created_pid = 0;
DWORD _last_injected_pid = 0;

static NxlPath __injectingDll;
/*
	CreateProcessW
*/
static BOOL __internalCreatingProcessW = FALSE;
static pfCreateProcessW CreateProcessW_original;
static pfCreateProcessW CreateProcessW_next;
BOOL CreateProcessW_hooked(LPCWSTR lpApplicationName, 
	LPWSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL ret;
	DWORD lasterror;
	if(__internalCreatingProcessW)
	{
		ret = CreateProcessW_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		lasterror = GetLastError();
	}
	else
	{
		BOOL injecting = FALSE;
		_last_injected_pid = 0;
		_last_created_pid = 0;
		CALL_START("CreateProcessW(appName='%S', cmd='%S' curDir='%S')", lpApplicationName, lpCommandLine, lpCurrentDirectory);
		if (__injectingDll.IsValid()
			&& ASSERT_FILE_EXISTS(__injectingDll))
		{
			injecting = TRUE;
			//TODO:WORKAROUND fix for RPM:if any file access happened before the process is set as trust process, the process will never have view right
			//to solve this issue, we need set the process as trust right after it's created
			//CreateProcess with flag-CREATE_SUSPENDED will suspend the process after it's created, we set the process as trust at this time
			DWORD dwSuspendCreationFlags = dwCreationFlags | CREATE_SUSPENDED;
			DBGLOG("CREATE_SUSPENDED=%d", dwCreationFlags & CREATE_SUSPENDED);
			__internalCreatingProcessW = TRUE;
			CALL_NEXT(ret = CreateProcessExW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwSuspendCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, __injectingDll.wstr()));
			lasterror = GetLastError();
			__internalCreatingProcessW = FALSE;
			if (ret)
			{
				DBGLOG("CreateProcess():pid=%lu tid=%lu", lpProcessInformation->dwProcessId, lpProcessInformation->dwThreadId);
				//set process as trusted
				g_rpmSession->SetTrustedProcess(lpProcessInformation->dwProcessId, true);
				//resume thread if it's set by us
				if (dwCreationFlags != dwSuspendCreationFlags)
				{
					if (ResumeThread(lpProcessInformation->hThread) == -1)
					{
						DBGLOG("ResumeThread(hThread=%d) failed with error code %d", lpProcessInformation->hThread, GetLastError());
					}
				}
			}
		}
		else
		{
			CALL_NEXT(ret = CreateProcessW_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation));
			lasterror = GetLastError();
		}
		if(ret)
		{
			_last_created_pid = lpProcessInformation->dwProcessId;
			CALL_END("CreateProcessW(appName='%S', cmd='%S' curDir='%S' pid=%lu) returns %d", lpApplicationName, lpCommandLine, lpCurrentDirectory, lpProcessInformation->dwProcessId, ret);
			if(injecting)
			{
				_last_injected_pid = _last_created_pid;
			}
		}
		else
		{
			CALL_END("CreateProcessW(appName='%S', cmd='%S' curDir='%S') returns %d(Error=%lu)", lpApplicationName, lpCommandLine, lpCurrentDirectory, ret, GetLastError());
		}
	}
	SetLastError(lasterror);
	return ret;
}

/*
	CreateProcessA
*/
static BOOL __internalCreatingProcessA = FALSE;
static pfCreateProcessA CreateProcessA_original;
static pfCreateProcessA CreateProcessA_next;
BOOL CreateProcessA_hooked(LPCSTR lpApplicationName, 
	LPSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL ret;
	DWORD lasterror;
	if(__internalCreatingProcessA)
	{
		ret = CreateProcessA_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		lasterror = GetLastError();
	}
	else
	{
		BOOL injecting = FALSE;
		_last_injected_pid = 0;
		_last_created_pid = 0;
		CALL_START("CreateProcessA(appName='%s', cmd='%s' curDir='%s')", lpApplicationName, lpCommandLine, lpCurrentDirectory);
		if(__injectingDll.IsValid()
			&& ASSERT_FILE_EXISTS(__injectingDll))
		{
			injecting = TRUE;
			//TODO:WORKAROUND fix for RPM:if any file access happened before the process is set as trust process, the process will never have view right
			//to solve this issue, we need set the process as trust right after it's created
			//CreateProcess with flag-CREATE_SUSPENDED will suspend the process after it's created, we set the process as trust at this time
			DWORD dwSuspendCreationFlags = dwCreationFlags | CREATE_SUSPENDED;
			DBGLOG("CREATE_SUSPENDED=%d", dwCreationFlags & CREATE_SUSPENDED);
			__internalCreatingProcessA = TRUE;
			CALL_NEXT(ret = CreateProcessExA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwSuspendCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, __injectingDll.str()));
			lasterror = GetLastError();
			__internalCreatingProcessA = FALSE;
			if (ret)
			{
				DBGLOG("CreateProcess():pid=%lu tid=%lu", lpProcessInformation->dwProcessId, lpProcessInformation->dwThreadId);
				//set process as trusted
				g_rpmSession->SetTrustedProcess(lpProcessInformation->dwProcessId, true);
				//resume thread if it's set by us
				if (dwCreationFlags != dwSuspendCreationFlags)
				{
					if (ResumeThread(lpProcessInformation->hThread) == -1)
					{
						DBGLOG("ResumeThread(hThread=%d) failed with error code %d", lpProcessInformation->hThread, GetLastError());
					}
				}
			}
		}
		else
		{
			CALL_NEXT(ret = CreateProcessA_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation));
			lasterror = GetLastError();
		}
		if(ret)
		{
			_last_created_pid = lpProcessInformation->dwProcessId;
			CALL_END("CreateProcessA(appName='%s', cmd='%s' curDir='%s' pid=%lu) returns %d", lpApplicationName, lpCommandLine, lpCurrentDirectory, lpProcessInformation->dwProcessId, ret);
			if(injecting)
			{
				_last_injected_pid = _last_created_pid;
			}
		}
		else
		{
			CALL_END("CreateProcessA(appName='%s', cmd='%s' curDir='%s') returns %d(Error=%lu)", lpApplicationName, lpCommandLine, lpCurrentDirectory, ret, GetLastError());
		}
	}
	SetLastError(lasterror);
	return ret;
}
BOOL inject_on_CreateProcess(const char *dllToBeInjected)
{
	DBGLOG("Injecting '%s' on CreateProcess", dllToBeInjected);
	if(dllToBeInjected != NULL)
	{
		__injectingDll = NxlPath(dllToBeInjected);
		HOOK(CreateProcessW);
		HOOK(CreateProcessA);
		return IS_HOOKED(CreateProcessW)&&IS_HOOKED(CreateProcessA);
	}
	else
	{
		__injectingDll = NxlPath();
		UNHOOK(CreateProcessW);
		UNHOOK(CreateProcessA);
		return NOT_HOOKED(CreateProcessW)&&NOT_HOOKED(CreateProcessA);
	}
}