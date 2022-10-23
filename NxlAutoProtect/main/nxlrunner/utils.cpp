
#include "stdafx.h"
#include "ntdll.h"
#include <ProcessContext.h>
#include <FileContext.h>
#include <filesystem>
#include <chrono>

#include <strsafe.h>
#include <tchar.h>
#include <sys/types.h>
#include <sys/stat.h>

__int64 GetLastWriteTime(LPCTSTR lpszFile)
{
	struct _stat64i32 result;
	if (_wstat(lpszFile, &result) == 0)
	{
		DBGLOG("'%s':CreateTime=%I64d AccessTime=%I64d LastWriteTime=%I64d", lpszFile, result.st_ctime, result.st_atime, result.st_mtime);
		return result.st_mtime;
	}
	else
	{
		DBGLOG("'%s':_wstat failed", lpszFile);
	}
	return 0;
}
NTSTATUS NtQuerySystemInformation_inner(
	_In_      DWORD SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
)
{
	static PNtQuerySystemInformation NtQuerySystemInformation_dynamic = NULL;
	if (NtQuerySystemInformation_dynamic == NULL)
	{
		HMODULE ntdllModule = GetModuleHandle(TEXT("ntdll.dll"));
		if (ntdllModule == NULL)
		{
			ntdllModule = LoadLibrary(TEXT("ntdll.dll"));
		}
		NtQuerySystemInformation_dynamic = (PNtQuerySystemInformation)GetProcAddress(
			ntdllModule,
			("NtQuerySystemInformation"));
		DBGLOG("ntdll Module=%p NtQuerySystemInformation=%p", ntdllModule, NtQuerySystemInformation_dynamic);
	}
	return NtQuerySystemInformation_dynamic(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
}
std::wstring FileContext::GetLongPath(const std::wstring &filePath)
{
	if(!filePath.empty())
	{
		WCHAR fileLongName[MAX_PATH];
		if(0 == GetLongPathName(filePath.c_str(), fileLongName, MAX_PATH))
		{
			DBGLOG("GetLongPathName Failed(%d)", GetLastError());
			return filePath;
		}
		else
		{
			WCHAR *ext = PathFindExtension(fileLongName);
			if(IS_NXL_EXT(ext)
				&& !IS_NXL_EXT(PathFindExtension(filePath.c_str())))
			{
				//if long path name has .nxl ext but original file doesn't have
				//remove it
				ext[0] = '\0';
			}
			return fileLongName;
		}
	}
	return L"";
}
typedef struct GetFileNameThreadParams_s
{
	HANDLE fileHandle;
	std::wstring fileName;
	DWORD error;
}GetFileNameThreadParams, *pGetFileNameThreadParams;
void GetFileNameThread( PVOID pParam )
{
	// This thread function for getting the filename
	// if access denied, we hang up in function GetFinalPathNameByHandle, 
	// so if it times out we just kill this thread
	pGetFileNameThreadParams params = (pGetFileNameThreadParams)pParam;
	params->fileName.clear();
	params->error = 0;
	WCHAR buf[MAX_PATH] = TEXT("");

	//DBGLOG("==>GetFinalPathNameByHandle...");
	if(GetFinalPathNameByHandle( params->fileHandle, buf, MAX_PATH, 0 )<=0)
	{
		//DBGLOG("==>GetFinalPathNameByHandle Failed(%lu) (Error:%#x)", params->returnCode, GetLastError());
		params->error = GetLastError();
	}
	else
	{
		params->fileName = buf;
	}
}


DWORD FileHandle::UpdateHandleName()
{
	HANDLE dupFileHandle = _fileHandle;
	_handleName.clear();
	_handleType.clear();
	if(_ownerPid != GetCurrentProcessId())
	{
		//the handle is from remote process, trying to duplicate one for current process
		HANDLE processHandle = OpenProcess( PROCESS_DUP_HANDLE , FALSE, _ownerPid);
		if( processHandle )
		{
			if(!DuplicateHandle( processHandle, _fileHandle, GetCurrentProcess(), &dupFileHandle, 0, FALSE, DUPLICATE_SAME_ACCESS ))
			{
				//if failed, set default
				dupFileHandle = _fileHandle;
			}
			CloseHandle( processHandle );
		}
	}
	//check file type
	PPUBLIC_OBJECT_TYPE_INFORMATION typeInfo = NULL;
	DWORD size;
	NtQueryObject(dupFileHandle, ObjectTypeInformation, typeInfo, 0, &size);

	typeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)realloc(typeInfo, size);

	// Query the info size ( type )
	if (NtQueryObject(dupFileHandle, ObjectTypeInformation, typeInfo, size, NULL) == 0)
	{
		_handleType = typeInfo->TypeName.Buffer;
	}
	free(typeInfo);
	DWORD error = 0;
	//start a new thread to get the file name to avoid GetFinalPathNameByHandle hang issue
	GetFileNameThreadParams threadParams = {dupFileHandle, L"", 0};
	// Let's start the thread to get the file name
	HANDLE threadHandle = (HANDLE)_beginthread( GetFileNameThread, 0, &threadParams );

	if ( threadHandle != NULL )
	{
		// Wait for finishing the thread
		if ( WaitForSingleObject( threadHandle, 100 ) == WAIT_TIMEOUT )
		{
			//DBGLOG("==>Timeout");
			// Access denied
			// Terminate the thread
			TerminateThread( threadHandle, 0 );
		}
		else
		{
			_handleName = threadParams.fileName;
		}
		error = threadParams.error;
	}
	else
	{
		error = GetLastError();
	}
	/*use GetFileInformationByHandleEx to get file name info*/
	/*
	DWORD nameInfoLen = sizeof(FILE_NAME_INFO)+MAX_PATH*(sizeof(WCHAR));
	PFILE_NAME_INFO pNameInfo = (PFILE_NAME_INFO)malloc(nameInfoLen);
	if(GetFileInformationByHandleEx(hDup, FileNameInfo, pNameInfo, nameInfoLen))
	{
		retStr = (WCHAR *)malloc(sizeof(WCHAR)*MAX_PATH);
		wcsncpy_s(retStr, MAX_PATH, pNameInfo->FileName, pNameInfo->FileNameLength);
	}
	free(pNameInfo);
	*/

	//close the file handle if it's duplicated
	if( dupFileHandle && (dupFileHandle != _fileHandle))
	{
		CloseHandle( dupFileHandle );
	}
	return error;
}
std::wstring ProcessContext::QueryProcessFullName(DWORD pid)
{
	static WCHAR pname[MAX_PATH] = TEXT("");
	HANDLE phandle = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (phandle) 
	{
		DWORD size = sizeof(pname)/sizeof(WCHAR);
		if(QueryFullProcessImageName(phandle, 0, pname, &size))
		{
			// At this point, buffer contains the full path to the executable
			CloseHandle(phandle);
			return pname;
		}
		DBGLOG("[%lu]QueryFullProcessImageName failed:%#x", pid, GetLastError());

        /*HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( phandle, &hMod, sizeof(hMod), &cbNeeded) )
        {
            if(GetModuleBaseName( phandle, hMod, pname, sizeof(pname)/sizeof(WCHAR))==0)
			{
				DBGLOG("==>GetModuleBaseName Failed(Errror:%lu )", GetLastError());
			}
        }
		else
		{
			DBGLOG("==>EnumProcessModules Failed(Error:%lu Needed=%lu)", GetLastError(), cbNeeded);
		}*/

		CloseHandle(phandle);
	}
	else
	{
		DWORD lastError = GetLastError();
		if(lastError != ERROR_ACCESS_DENIED)
			DBGLOG("==>OpenProcess(%lu) failed:%#x", pid, lastError);
	}
	return L"";
}

bool FileHandle::IsHandleMatchedFile(const std::wstring &file) const
{
	if (!IsValid())
		return false;
	//DBGLOG("Handle[%x] OwnerPid=%lu handleName='%s'", _fileHandle, _ownerPid, _handleName.c_str());
	if (_handleName.length() <= 4) return false;
	//the format fo handle name from GetFinalPathNameByHandle is \\?\C:\
			//+4 will remove the prefix \\?\ 
	std::wstring hName = _handleName.substr(4);
	const std::wstring ext = PathFindExtension(hName.c_str());
	//remove .nxl extension from handle name;
	if (IS_NXL_EXT(ext.c_str()))
	{
		hName = hName.substr(0, hName.length() - ext.length());
	}
	if (0 == _wcsicmp(hName.c_str(), file.c_str()))
	{
		return true;
	}
	return false;
}
std::wstring ProcessContext::QueryProcessFullName(HANDLE hnd)
{
	if (hnd)
	{
		static WCHAR pname[MAX_PATH] = TEXT("");
		DWORD size = sizeof(pname) / sizeof(WCHAR);
		if (QueryFullProcessImageName(hnd, 0, pname, &size))
		{
			// At this point, buffer contains the full path to the executable
			return pname;
		}
	}
	return std::wstring();
}

#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004
std::vector<FileHandle> ProcessContext::SearchFileHandlersInProcesses(const std::wstring &fileName, const std::vector<ProcessContext> &processes)
{
	NTSTATUS status;
	int nsearched = 0;
	std::vector<FileHandle> fileHandles;
	// Get the list of all handles in the system
	DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);
	PSYSTEM_HANDLE_INFORMATION pSysHandleInformation = (PSYSTEM_HANDLE_INFORMATION)malloc(size);
	DWORD needed = 0;
	std::vector<ProcessContext> xProcesses;
	while(!NT_SUCCESS(status = NtQuerySystemInformation_inner( SystemHandleInformation, pSysHandleInformation, size, &needed )))
	{
		if( status != STATUS_INFO_LENGTH_MISMATCH
			|| needed == 0)
		{
			DBGLOG("==>Failed Status=%l(%#X) Needed=%lu", status, status, needed);
			goto CLEAN;// some other error
		}
		// The previously supplied buffer wasn't enough.
		size = needed + 1024;
		pSysHandleInformation = (PSYSTEM_HANDLE_INFORMATION)realloc(pSysHandleInformation, size);
	}

	//exclude RMC process when all processes need to be searched
	if(processes.empty())
	{
		xProcesses = ProcessContext::SearchProcessesByName(L"nxrmserv.exe");
	}

	// Iterating through the objects
	for (DWORD i = 0; i < pSysHandleInformation->Count; i++ )
	{
		DWORD handlePid = pSysHandleInformation->Handles[i].ProcessID;
		//filter by process id
		if(!processes.empty()						//input pids is defined
			&& std::find(processes.begin(), processes.end(), handlePid) == processes.end())	//AND input pids doesn't contain this handle process
			continue;
		//skip rmc process if neccessary
		if(!xProcesses.empty()									//RMC process exists
			&& std::find(xProcesses.begin(), xProcesses.end(), handlePid) != xProcesses.end())	//AND this handle process is RMC process
			continue;

		HANDLE handle = (HANDLE)pSysHandleInformation->Handles[i].HandleNumber;
		FileHandle fileHandle(handle, handlePid);
		if(fileHandle.IsFileHandle())
		{
			nsearched++;
			DBGLOG("[%d/%d]ProcessId=%6lu Handle=%#.3x Type=%#.2x Flags=%#x HandleType='%s' handleName='%s'",
				i + 1,
				pSysHandleInformation->Count,
				handlePid,
				handle,
				pSysHandleInformation->Handles[i].HandleType,
				pSysHandleInformation->Handles[i].HandleFlags,
				fileHandle.type().c_str(),
				fileHandle.name().c_str());
			if(fileHandle.IsHandleMatchedFile(fileName))
			{
				fileHandles.push_back(std::move(fileHandle));
			}
		}
	}
CLEAN:
	DBGLOG("Searched %u/%u handles in %d processes, Found %d handle(s)", nsearched, pSysHandleInformation->Count, processes.size(), fileHandles.size());
	free(pSysHandleInformation);
	return fileHandles;
}
std::vector<ProcessContext> ProcessContext::SearchProcessesByName(const std::wstring &processName)
{
	std::vector<ProcessContext> processes;
	if(processName.empty())
		return processes;
	// Get the list of process identifiers
	DWORD sAllPids = 1024;	
	DWORD *allPids = (DWORD*)malloc(sAllPids * sizeof(DWORD));
	DWORD nAllPids = 0;
	DWORD cbNeeded;
	while(EnumProcesses( allPids, sAllPids, &cbNeeded ))
	{
		nAllPids = cbNeeded / sizeof(DWORD);
		DBGLOG("ArraySize=%lu ArrayCount=%lu", sAllPids, nAllPids);
		if( nAllPids == sAllPids )
		{
			//try again
			sAllPids *= 2;
			allPids = (DWORD*)realloc(allPids, sAllPids * sizeof(DWORD));
		}
		else
		{
			break;
		}
	}
	if(nAllPids == 0				//failed in first EnumProcesses
		|| nAllPids == sAllPids)	//failed in following EnumProcesses
	{
		DBGLOG("EnumProcesses Failed(Error=%lu):ArraySize=%lu ArrayCount=%lu)", GetLastError(), sAllPids, nAllPids);
		//continue with existing processes
		//goto CLEANUP;
	}

	DWORD iprocess;
	for ( iprocess = 0; iprocess < nAllPids; iprocess++ )
	{
		if( allPids[iprocess] != 0 )
		{
			auto pFullName = QueryProcessFullName(allPids[iprocess]);
			if(!pFullName.empty())
			{
				ProcessContext process(allPids[iprocess], pFullName);
				DBGLOG("[%lu]%s", process.pid(), pFullName.c_str());
				if(_wcsicmp(process.pname().c_str(), processName.c_str())==0)
				{
					processes.push_back(std::move(process));
				}
			}
		}
	}
	//CLEANUP:
	DBGLOG("Found %d processes with name-'%s'", processes.size(), processName.c_str());
	return processes;
}
#define REG_KEY_NXLRUNNER L"SOFTWARE\\NextLabs\\TeamcenterRMX\\nxlrunner"
//always query in 64 bit registry view in 64 bit system
#define REG_KEY_OPEN_SAM KEY_QUERY_VALUE|KEY_WOW64_64KEY
std::wstring reg_get_chars_value(const std::wstring &appName, const std::wstring &valName)
{
	HKEY hKey;
	std::wstring args;
	LONG returnStatus;
	std::wstring keyPath(REG_KEY_NXLRUNNER);
	if (!appName.empty())
	{
		keyPath = keyPath + L"\\" + appName;
	}
	if ((returnStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, REG_KEY_OPEN_SAM , &hKey)) == ERROR_SUCCESS)
	{
		wchar_t lszValue[MAX_PATH];
		DWORD dwType = REG_SZ;
		DWORD dwSize = MAX_PATH;
		if ((returnStatus = RegQueryValueExW(hKey, valName.c_str(), NULL, &dwType,(LPBYTE)&lszValue, &dwSize)) == ERROR_SUCCESS)
		{
			args = lszValue;
		}
	}
	RegCloseKey(hKey);
	DBGLOG("%s:%s='%s'", keyPath.c_str(), valName.c_str(), args.c_str());
	return args;
}

DWORD reg_get_dword_value(const std::wstring &appName, const std::wstring &valName, DWORD defaultValue)
{
	DWORD dwValue = defaultValue;
	HKEY hKey;
	LONG returnStatus;
	std::wstring keyPath(REG_KEY_NXLRUNNER);
	if(!appName.empty())
	{
		keyPath = keyPath + L"\\" + appName;
	}
	if ((returnStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0,  REG_KEY_OPEN_SAM , &hKey)) == ERROR_SUCCESS)
	{
		DWORD dwSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		if ((returnStatus = RegQueryValueExW(hKey, valName.c_str(), NULL, &dwType,(LPBYTE)&dwValue, &dwSize)) == ERROR_SUCCESS)
		{
			DBGLOG("%s:%s=%lu", keyPath.c_str(), valName.c_str(), dwValue);
		}
	}
	RegCloseKey(hKey);
	return dwValue;
}

bool reg_get_multi_chars(const std::wstring &appName, const std::wstring &valName, std::vector<std::wstring> &values)
{
	HKEY hKey;
	bool ret = false;
	LONG returnStatus;
	std::wstring keyPath(REG_KEY_NXLRUNNER);
	if (!appName.empty())
	{
		keyPath = keyPath + L"\\" + appName;
	}
	if ((returnStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, REG_KEY_OPEN_SAM, &hKey)) == ERROR_SUCCESS)
	{
		DWORD dwType = REG_MULTI_SZ;
		DWORD dwSize = 10;
		std::wstring buffer;
		returnStatus = ERROR_MORE_DATA;
		while (returnStatus == ERROR_MORE_DATA)
		{
			buffer.reserve(dwSize * sizeof(BYTE) / sizeof(WCHAR));
			if ((returnStatus = RegQueryValueExW(hKey, valName.c_str(), NULL, &dwType, (LPBYTE)&(buffer.data()[0]), &dwSize)) == ERROR_SUCCESS)
			{
				DBGLOG("RegQueryValueExW('%s','%s') returns %lu", keyPath.c_str(), valName.c_str(), dwSize);
				//the packed string is in following format
				//str1\0str2\0str3\0\0
				DWORD nchars = dwSize / sizeof(WCHAR);
				for(DWORD ichar = 0, istart=0;ichar <= nchars; ichar++)
				{
					if (buffer[istart] == '\0')
						break;
					if (ichar == nchars || buffer[ichar] == '\0')
					{
						if (ichar > istart)
						{
							values.push_back(std::wstring(buffer.begin()+istart, buffer.begin()+ichar));
							DBGLOG("%d:'%s'", values.size(), values.back().c_str());
						}
						istart = ichar + 1;
					}
				}
				ret = true;
			}
			else if (returnStatus == ERROR_MORE_DATA)
			{
				//allocate 
				DBGLOG("Requiring more buffer space(%lu)", dwSize);
				continue;
			}
			else
			{
				//error;
				DBGLOG("RegQueryValueExW('%s','%s', %lu) failed with %lu(%#X)", keyPath.c_str(), valName.c_str(), dwSize, GetLastError(), GetLastError());
			}
		}
	}
	else
	{
		//error;
		DBGLOG("RegOpenKeyExW('%s') failed with %lu(%#X)", keyPath.c_str(), GetLastError(), GetLastError());
	}
	RegCloseKey(hKey);
	return ret;
}