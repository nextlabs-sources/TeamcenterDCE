#include "hook_log.h"
#include "ntdll.h"

typedef NTSTATUS (NTAPI *pfNtQueryObject)( HANDLE, ULONG, PVOID, ULONG, PULONG);
static pfNtQueryObject NtQueryObject_dynamic = NULL;

NTSTATUS NtQueryObject_inner(
  HANDLE                   Handle,
  OBJECT_INFORMATION_CLASS ObjectInformationClass,
  PVOID                    ObjectInformation,
  ULONG                    ObjectInformationLength,
  PULONG                   ReturnLength)
{
	//initialize NtQueryObject
	if(NtQueryObject_dynamic == NULL)
	{
		HMODULE ntdllModule = GetModuleHandle("ntdll.dll");
		if(ntdllModule == NULL)
		{
			ntdllModule = LoadLibrary("ntdll.dll");
		}
		NtQueryObject_dynamic = (pfNtQueryObject)GetProcAddress(ntdllModule, "NtQueryObject");
		DBGLOG("ntdll Module=%p NtQueryObject=%p", ntdllModule, NtQueryObject_dynamic);
	}
	return NtQueryObject_dynamic(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);
}

typedef NTSTATUS (*pfNtQueryInformationProcess)(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
    PULONG ReturnLength
);
static pfNtQueryInformationProcess NtQueryInformationProcess_dynamic = NULL;
NTSTATUS NtQueryInformationProcess_inner(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
    PULONG ReturnLength)
{
	//initialize NtQueryObject
	if(NtQueryInformationProcess_dynamic == NULL)
	{
		HMODULE ntdllModule = GetModuleHandle("ntdll.dll");
		if(ntdllModule == NULL)
		{
			ntdllModule = LoadLibrary("ntdll.dll");
		}
		NtQueryInformationProcess_dynamic = (pfNtQueryInformationProcess)GetProcAddress(ntdllModule, "NtQueryInformationProcess");
		DBGLOG("ntdll Module=%p NtQueryInformationProcess=%p", ntdllModule, NtQueryInformationProcess_dynamic);
	}
	return NtQueryInformationProcess_dynamic(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
}