#ifndef NXL_HOOK_WINDOWS_NTIFS_H_INCLUDED
#define NXL_HOOK_WINDOWS_NTIFS_H_INCLUDED
#include <WinNT.h>
#include <winternl.h>

typedef struct _FILE_ID_FULL_DIR_INFORMATION {
  ULONG         NextEntryOffset;
  ULONG         FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG         FileAttributes;
  ULONG         FileNameLength;
  ULONG         EaSize;
  LARGE_INTEGER FileId;
  WCHAR         FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION {
  ULONG         NextEntryOffset;
  ULONG         FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG         FileAttributes;
  ULONG         FileNameLength;
  ULONG         EaSize;
  CCHAR         ShortNameLength;
  WCHAR         ShortName[12];
  WCHAR         FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;


#define FileBothDirectoryInformation 3

/*
	NtQueryDirectoryFile
*/
extern NTSYSAPI NTSTATUS NTAPI NtQueryDirectoryFile(
	HANDLE FileHandle, 
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine, 
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock, 
	PVOID FileInformation, 
	ULONG Length, 
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN ReturnSingleEntry, 
	PUNICODE_STRING FileName,
	BOOLEAN RestartScan);
typedef NTSTATUS (NTAPI *pfNtQueryDirectoryFile)(
	HANDLE FileHandle, 
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine, 
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock, 
	PVOID FileInformation, 
	ULONG Length, 
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN ReturnSingleEntry, 
	PUNICODE_STRING FileName,
	BOOLEAN RestartScan);
#endif