#include "hook_defs.h"
#include "hook_log.h"
#include "hook_utils.h"
#include "ntdll.h"
#include "hook_windows_defs.h"
#include <tie/tie.h>
#include <gms/gms.h>
#include <sa/tcfile.h>
#include <Shlwapi.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <winternl.h>

#define BUFSIZE MAX_PATH
#define file_exist(f) (f != NULL && GetFileAttributes(f) != INVALID_FILE_ATTRIBUTES)
/*
	Creating a Child Process with Redirected Input and Output
	Refer to:https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
*/

std::string convertWStoS(std::wstring ws){
	std::string s( (const char*)&ws[0], sizeof(wchar_t) / sizeof(char) * ws.size() );
	std::string ans = "";
	for( int i = 0; i < s.length(); i++ )
	{
		if( i % 2 == 0 )
			ans += s[i];
	}
	return ans;
}

BOOL process_run2(char *cmd)
{
	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;
	BOOL ret = FALSE;
	// Set up members of the PROCESS_INFORMATION structure.  
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection. 
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	//siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;
  
	DBGLOG("CreateProcess:'%s'", cmd);
	// Create the child process.     
	bSuccess = CreateProcess(NULL, 
			cmd,     // command line 
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			0,             // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&siStartInfo,  // STARTUPINFO pointer 
			&piProcInfo);  // receives PROCESS_INFORMATION 
   
	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
	{
		DBGLOG(TEXT("CreateProcess Failed(%lu)"), GetLastError());
	}
	else
	{
		while( TRUE ) {
			BOOL status = WaitForSingleObject(piProcInfo.hProcess, INFINITE);
			switch(status)
			{
				case WAIT_TIMEOUT:
					//WAIT TIMEOUT, MAYBE THE pipe buffer is full, it's waiting for a readfile
					DBGLOG("WaitForSingleObject(%lu) TimeOut", piProcInfo.dwProcessId);
					break;
				case WAIT_FAILED:
					DBGLOG("WaitForSingleObject(%lu) Failed:%lu", piProcInfo.dwProcessId, GetLastError());
					break;
				case WAIT_OBJECT_0:
					break;
				case WAIT_ABANDONED:
				default:
					DBGLOG("WaitForSingleObject(%lu) Status=%lu", piProcInfo.dwProcessId, status);
					break;
			}
			DWORD retCode;
			BOOL r = GetExitCodeProcess(piProcInfo.hProcess, &retCode);
			if(r == 0)
			{
				DBGLOG("GetExitCodeProcess(%lu) Failed:%lu", piProcInfo.dwProcessId, GetLastError());
				break;
			}
			else if(r != STILL_ACTIVE)
			{
				DBGLOG("GetExitCodeProcess(%lu) returns %d(ExitCode=%lu)", piProcInfo.dwProcessId, r, retCode);
				break;
			}
		}
STOP_WAIT:
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
		ret = TRUE;
	}
	return ret;
}
void run_encrypter(const char *pname, const char *file, int nargs, char **args)
{
	if(pname != NULL && file != NULL)
	{
		DBGLOG("in run_encrypter");
		char cmd[0x1000];
		HMODULE module;
		//initialize current module path
		if((module = GetModuleHandle("NxlHook.dll")) != NULL)
		{
			int len  = GetModuleFileName(module, cmd, sizeof(cmd));
			if(len > 0)
			{
				//DBGLOG("GetModuleFileName=%s", dirBuf);
				PathRemoveFileSpec(cmd);
				len = strlen(cmd);
				len += sprintf(cmd+len, "\\%s", pname);
				if(file_exist(cmd))
				{
					int iarg = 0;
					len += sprintf(cmd+len, " \"%s\"", file);
					for(iarg = 0; iarg < nargs; iarg++)
					{
						DBGLOG("arg-%d:'%s'", iarg, args[iarg]);
						len += sprintf(cmd+len, " \"%s\"", args[iarg]);
					}
					DBGLOG("Executing(%d)....'%s'", len, cmd);
					process_run2(cmd);
				}
				else
				{
					DBGLOG("FileNotFound-'%s'", cmd);
				}
			}
			else
			{
				DBGLOG("Failed to GetModuleFileName!(ErrorCode:%#X)", GetLastError());
			}
		}
	}
}


#define base_save_part_FULLNAME "?base_save_part@@YAXPEADPEAH11@Z"
#define base_save_part_ORDINAL 18071
typedef void (*pfbase_save_part)(char *  osFileName,
	int *a,
	int *b,
	int *c
);

static pfbase_save_part base_save_part_original;
static pfbase_save_part base_save_part_next;
static void base_save_part_hooked(
	char *  osFileName,
	int *a,
	int *b,
	int *c
   )
{
	
	DBGLOG("inside base_save_part function");
	CALL_START("base_save_part(osFileName='%s')", osFileName);
	CALL_NEXT(base_save_part_next(osFileName, a, b, c));
	
}


#define PART_validate_part_for_creation_FULLNAME "?PART_validate_part_for_creation@@YAHPEBDPEAPEAD@Z"
//TODO:
#define PART_validate_part_for_creation_ORDINAL 13198
typedef int (*pfPART_validate_part_for_creation)(const char *  osFileName,
	char ** nameArr
);

static pfPART_validate_part_for_creation PART_validate_part_for_creation_original;
static pfPART_validate_part_for_creation PART_validate_part_for_creation_next;
static int PART_validate_part_for_creation_hooked(
	const char *  osFileName,
	char ** nameArr
   )
{
	int ret;
	DBGLOG("inside hook function");
	CALL_START("PART_validate_part_for_creation(osFileName='%s')", osFileName);
	//run_modifier("bczModifier.exe", osFileName, 0, NULL);
	DBGLOG("nameArr0==%s", nameArr[0]);

	CALL_NEXT(ret = PART_validate_part_for_creation_next(osFileName, nameArr));
	
	CALL_START("PART_validate_part_for_creation(osFileName='%s') return %d", osFileName, ret);

	return ret;
}


static BOOL is_file_handle(HANDLE handle)
{
	BOOL isFile = FALSE;
	ULONG expectedSize = 0x100;//sizeof(PUBLIC_OBJECT_TYPE_INFORMATION);
	ULONG size = 0;
	NTSTATUS nt = STATUS_INFO_LENGTH_MISMATCH;
	PPUBLIC_OBJECT_TYPE_INFORMATION typeInfo = NULL;
	while(nt == STATUS_INFO_LENGTH_MISMATCH
		&& expectedSize > size)
	{
		size = expectedSize;
		expectedSize = 0;
		typeInfo =  (PPUBLIC_OBJECT_TYPE_INFORMATION)realloc(typeInfo, size);
		memset(typeInfo, 0, size);
		//get basic info
		if(NT_SUCCESS(nt = NtQueryObject_inner(handle, ObjectTypeInformation, typeInfo, size, &expectedSize)))
		{
			//DVLOG("[%#x]TypeName='%S' Len=%hu MaxLen=%hu", handle, typeInfo->TypeName.Buffer, typeInfo->TypeName.Length, typeInfo->TypeName.MaximumLength);
			isFile = wcsicmp(typeInfo->TypeName.Buffer, L"File") == 0;
			goto CLEANUP;
		}
		DBGLOG("[%#x]NtQueryObject(ObjectTypeInformation, size=%lu) return %#X(ExpectedSize=%lu)", handle, size, nt,  expectedSize);
	}
CLEANUP:
	free(typeInfo);
	return isFile;
}
static BOOL is_write_handle(HANDLE handle, BOOL *isWrite)
{
	BOOL ret = FALSE;
	ULONG expectedSize = sizeof(PUBLIC_OBJECT_BASIC_INFORMATION);//somehow ObjectBasicInformation can work only with this size
	ULONG size = 0;
	NTSTATUS nt = STATUS_INFO_LENGTH_MISMATCH;
	PPUBLIC_OBJECT_BASIC_INFORMATION basicInfo = NULL;
	
	while(nt == STATUS_INFO_LENGTH_MISMATCH
		&& expectedSize > size)
	{
		size = expectedSize;
		expectedSize = 0;
		basicInfo =  (PPUBLIC_OBJECT_BASIC_INFORMATION)realloc(basicInfo, size);
		memset(basicInfo, 0, size);
		//get basic info
		if(NT_SUCCESS(nt = NtQueryObject_inner(handle, ObjectBasicInformation, basicInfo, size, &expectedSize)))
		{
			ACCESS_MASK mask = basicInfo->GrantedAccess;
			BOOL fwd = mask & FILE_WRITE_DATA;
			//DVLOG("[%p]==>Attributes:%lu(%#X)", handle, basicInfo->Attributes, basicInfo->Attributes);
			//DVLOG("[%p]GrantedAccess:%#010X(FILE_WRITE_DATA=%d)", handle, mask, fwd);
			//DVLOG("[%p]==>HandleCount:%lu(%#X)", handle, basicInfo->HandleCount, basicInfo->HandleCount);
			//DVLOG("[%p]==>PointerCount:%lu(%#X)", handle, basicInfo->PointerCount, basicInfo->PointerCount);
			*isWrite = fwd;
			ret = TRUE;
			break;
		}
		//DBGLOG("[%#x]NtQueryObject(ObjectBasicInformation, size=%lu) return %#X(ExpectedSize=%lu)", handle, size, nt,  expectedSize);
	}
	free(basicInfo);
	return ret;
}



//TODO:
#define CloseHandle_FULLNAME "CloseHandle"
#define CloseHandle_ORDINAL 54
typedef BOOL (WINAPI *pfCloseHandle)(
	HANDLE hObject
);

static pfCloseHandle CloseHandle_original;
static pfCloseHandle CloseHandle_next;
static BOOL WINAPI CloseHandle_hooked(
	HANDLE hObject
   )
{
	BOOL ret;
	
	char buf[BUFSIZE] = {0};
	DWORD dwRet;
	if(is_file_handle(hObject))
	{
		//DBGLOG("This is a file handle");
		if(GetFinalPathNameByHandle(hObject, buf, BUFSIZE, 0 ) > 0)
		{
			DBGLOG("file name==%s", buf);
		}
	}
	ret = CloseHandle_next(hObject);
	
	//dwRet = GetFinalPathNameByHandle(hObject, Path, BUFSIZE, FILE_NAME_NORMALIZED);
	//if (dwRet < BUFSIZE)
	//{
	//	DBGLOG("file name == %s", Path);
	//	std::wstring s = Path;
	//	std::wstring subStr = L"";
	//	if (s.length() >=8 )
	//	{
	//		subStr = s.substr(s.length() - 8);
	//		if( subStr.compare(L".prt.nxl")==0 )
	//		{
	//			DBGLOG("find .prt.nxl");
	//			DBGLOGW("file name == %s", s.c_str());
	//			//CopyFile(s.c_str(), L"C:\\app\\scfWorkAround\\test.prt.nxl", FALSE);
	//		}
	//
	//	}
	//}
	
	return ret;

}

BOOL briefcasebrowser_install()
{	
	HOOK_API("kernel32.dll", CloseHandle);
	HOOK_API("libpart.dll", base_save_part);
	return TRUE;
}
void briefcasebrowser_uninstall()
{
}
