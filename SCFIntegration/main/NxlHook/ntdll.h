#ifndef NXL_HOOK_NTDLL_H_INCLUDED
#define NXL_HOOK_NTDLL_H_INCLUDED
#include <Windows.h>
#include <Winternl.h>

extern NTSTATUS NtQueryObject_inner(
	HANDLE                   Handle,
	OBJECT_INFORMATION_CLASS ObjectInformationClass,
	PVOID                    ObjectInformation,
	ULONG                    ObjectInformationLength,
	PULONG                   ReturnLength
);
extern NTSTATUS NtQueryInformationProcess_inner(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
    PULONG ReturnLength OPTIONAL
);

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#endif
#ifndef STATUS_NO_MORE_FILES
#define STATUS_NO_MORE_FILES 0x80000006
#endif
#ifndef STATUS_NO_SUCH_FILE
#define STATUS_NO_SUCH_FILE 0xC000000F
#endif
#endif