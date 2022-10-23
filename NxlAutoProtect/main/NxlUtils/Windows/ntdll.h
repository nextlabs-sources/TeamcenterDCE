#ifndef NXLUTILS_WINDOWS_NTDLL_H_INCLUDED
#define NXLUTILS_WINDOWS_NTDLL_H_INCLUDED
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
extern NTSTATUS NtQuerySystemInformation_inner(
	_In_      DWORD SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
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
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif


#define ObjectNameInformation ((OBJECT_INFORMATION_CLASS)1)

typedef struct _OBJECT_NAME_INFORMATION
{
	UNICODE_STRING Name; // defined in winternl.h
	WCHAR NameBuffer;
} OBJECT_NAME_INFORMATION;

#define SystemHandleInformation (OBJECT_INFORMATION_CLASS)0x10

//https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/handle_table_entry.htm?ts=0,277
#ifdef WIN64
typedef struct _SYSTEM_HANDLE
{
	USHORT	ProcessID;
	USHORT	CreatorBackTraceIndex;
	UCHAR	HandleType;
	UCHAR	HandleFlags;
	USHORT	HandleNumber;
	PVOID	KernelAddress;
	ULONG	Flags;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;
#elif defined(WIN32)
typedef struct _SYSTEM_HANDLE
{
	DWORD	ProcessID;
	BYTE	HandleType;
	BYTE	HandleFlags;
	WORD	HandleNumber;
	DWORD	KernelAddress;
	DWORD	Flags;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;
#endif

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	DWORD			Count;
	SYSTEM_HANDLE	Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

#endif