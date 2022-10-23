#ifndef NT_DLL_H_INCLUDED
#define NT_DLL_H_INCLUDED
#include <Winternl.h>
#include <Windows.h>
//WINAPI NtQuerySystemInformation(
//  _In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
//  _Inout_   PVOID                    SystemInformation,
//  _In_      ULONG                    SystemInformationLength,
//  _Out_opt_ PULONG                   ReturnLength
//);
typedef NTSTATUS(WINAPI *PNtQuerySystemInformation)( DWORD, VOID*, DWORD, ULONG* );

typedef struct _SYSTEM_HANDLE
{
	DWORD	ProcessID;
	BYTE	HandleType;
	BYTE	HandleFlags;
	WORD	HandleNumber;
	DWORD	KernelAddress;
	DWORD	Flags;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	DWORD			Count;
	SYSTEM_HANDLE	Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif
#define SystemHandleInformation (DWORD)0x10

#endif