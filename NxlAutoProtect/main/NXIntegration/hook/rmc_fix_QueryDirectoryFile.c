#include <hook/hook_defs.h>
#include <hook/rmc_fixes.h>
#include <hook/windows/ntifs.h>
#include <NxlUtils/Windows/ntdll.h>

/*
	Fix for the issue that the file list returned by NtQueryDirectoryFile() with FileName=*.prt is not correct
	for example, there are 5 unprotected .prt files and 5 protected .prt.nxl files in the same directory
	When calling NtQueryDirectoryFile() with FileName=*.prt, the 5 unprotected .prt and some of protected .prt files will be returned
	the number of returned .prt.nxl files are not certain, it's environment dependent
	this issue may be caused by the RMC cache

	Solution:When NtQueryDirectoryFile() with FileName=*.prt returns STATUS_NO_MORE_FILES, do another NtQueryDirectoryFile() with .prt.nxl
*/
static pfNtQueryDirectoryFile NtQueryDirectoryFile_original;
static pfNtQueryDirectoryFile NtQueryDirectoryFile_next;
static NTSTATUS NTAPI NtQueryDirectoryFile_hooked(
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
	BOOLEAN RestartScan)
{
	//TODO:multiple threading
	static WCHAR lastDirMask[MAX_PATH] = L"";
	static HANDLE lastDirMaskHandle = INVALID_HANDLE_VALUE;
	NTSTATUS ret = NtQueryDirectoryFile_next(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan);
	if(FileInformationClass != FileBothDirectoryInformation)
	{
		//currently only hanlde for querying FileBothDirectoryInformation
		return ret;
	}
	else if((FileName != NULL && FileName->Length > 1 && FileName->Buffer[0]=='<'))
	{
		//FindFirstFile() with file mask like *.prt
		size_t len = FileName->Length/sizeof(WCHAR);
		DBGLOG("FindFirstFile('%S')(Len=%d)", FileName->Buffer, len);
		lastDirMaskHandle = FileHandle;
		wcsncpy(lastDirMask, FileName->Buffer, len);
		lastDirMask[len] = '\0';
	}
	else if(FileHandle == lastDirMaskHandle)
	{
		//FindNextFile() with file mask like *.prt
		DBGLOG("FindNextFile('%S')", lastDirMask);
	}
	else
	{
		//DBGLOG("FileHandle=%p FileName='%S' LastDirMaskHandle=%p lastMask='%S'", FileHandle, (FileName!=NULL?FileName->Buffer:NULL), lastDirMaskHandle, lastDirMask);
		//lastDirMaskHandle = INVALID_HANDLE_VALUE;
		//lastDirMask[0] = '\0';
		return ret;
	}
	DBGLOG("NtQueryDirectoryFile_next(FileHandle=%p, Event=%p, ApcRoutine=%p, ApcContext=%p, IoStatusBlock=%p, FileInformation=%p, Length=%lu, FileInformationClass=%d, ReturnSingleEntry=%d, FileName='%S', RestartScan=%d) returns %X"
		, FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, (FileName!=NULL?FileName->Buffer:NULL), RestartScan, ret);
	if(ret == STATUS_NO_MORE_FILES
		|| ret == STATUS_NO_SUCH_FILE)
	{
		DBGLOG("NO SUCH FILE/NO MORE FILES(%S)", lastDirMask);
		if(wcsstr(lastDirMask, L".nxl") == NULL)
		{
			wcscat(lastDirMask, L".nxl");
			UNICODE_STRING newDirMask = {0};
			newDirMask.Buffer = lastDirMask;
			newDirMask.MaximumLength = sizeof(lastDirMask);
			newDirMask.Length = wcslen(lastDirMask)*sizeof(WCHAR);
			ret = NtQueryDirectoryFile_next(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, &newDirMask, TRUE);
			DBGLOG("NtQueryDirectoryFile_next(NewFileName='%S'(len=%hu), RestartScan=TRUE) returns %X", newDirMask.Buffer, newDirMask.Length, ret);
		}
		else
		{
			return ret;
		}
	}
	//TODO?cache returned file list and check duplication
	//if(NT_SUCCESS(ret))
	//{
	//	//remove duplicated files when query .prt.nxl
	//	PFILE_BOTH_DIR_INFORMATION fileInformation = (PFILE_BOTH_DIR_INFORMATION)FileInformation;
	//	int ifile = 0;
	//	while(fileInformation != NULL)
	//	{
	//		//get shortname
	//		WCHAR shortname[12+1];
	//		size_t shortNameLen = fileInformation->ShortNameLength/sizeof(WCHAR);
	//		wcsncpy(shortname, fileInformation->ShortName, shortNameLen);
	//		shortname[shortNameLen]= '\0';
	//		//get longname
	//		WCHAR fileName[MAX_PATH];
	//		size_t fileNameLen = fileInformation->FileNameLength/sizeof(WCHAR);
	//		wcsncpy(fileName, fileInformation->FileName, fileNameLen);
	//		fileName[fileNameLen]= '\0';
	//		DBGLOG("[%d]ShortName(%d)='%S' FileName(%lu)='%S' Next=%lu", ifile, shortNameLen, shortname, fileNameLen, fileName, fileInformation->NextEntryOffset);
	//		if(fileInformation->NextEntryOffset > 0)
	//		{
	//			fileInformation = (PFILE_BOTH_DIR_INFORMATION)((char*)fileInformation + fileInformation->NextEntryOffset);
	//			ifile++;
	//		}
	//		else
	//		{
	//			fileInformation = NULL;
	//		}
	//	}
	//}
	return ret;
}
BOOL fix_QueryDirectoryFile_install()
{
	auto m = GetModuleHandle("ntdll.dll");
	HOOK_byName(NtQueryDirectoryFile, m);
	return IS_HOOKED(NtQueryDirectoryFile);
}
void fix_QueryDirectoryFile_uninstall()
{
	UNHOOK(NtQueryDirectoryFile);
}