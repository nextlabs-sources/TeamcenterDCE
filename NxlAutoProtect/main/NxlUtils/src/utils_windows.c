#ifdef WNT
#include <utils_system.h>
#include "utils_log.h"
#include <Windows.h>
#include <WinBase.h>
#include <Sddl.h>
#include <Shlwapi.h>
#include <tlhelp32.h>
#include <stdio.h>
#include "utils_mem.h"
#include "utils_string.h"
#include <winerror.h>

BOOL report_win_error(LONG retCode, LONG successCode, const char *func, const char *file, int line, const char *call)
{
	if(retCode != successCode)
	{
		char *errorMessage = NULL;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, retCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage, 0, NULL);
		nxl_log(TO_DBGVIEW|TO_LOGFILE, "%s():Executing '%s' failed(Error=%lu Message='%s')@Line-%d File-%s", func, call, retCode, errorMessage, line, file);
		LocalFree(errorMessage);
		return FALSE;
	}
	return TRUE;
}

const char *REG_get(const char *key, const char *valKey)
{
	HKEY hKey;
	const char *result = NULL;
	if (REG_CALL(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0,  KEY_READ, &hKey)))
	{
		static char lszValue[MAX_PATH];
		DWORD dwType = REG_SZ;
		DWORD dwSize = MAX_PATH;
		if (REG_CALL(RegQueryValueEx(hKey, valKey, NULL, &dwType,(LPBYTE)&lszValue, &dwSize)))
		{
			result = lszValue;
		}
		RegCloseKey(hKey);
	}
	DBGLOG("HKLM\\%s:%s='%s'", key, valKey, result);
	return result;
}
DWORD REG_get_dword(const char *key, const char *valName, DWORD defaultValue)
{
	DWORD dwValue = defaultValue;
	HKEY hKey;
	if (REG_CALL(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0,  KEY_READ , &hKey)))
	{
		DWORD dwSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		if (REG_CALL(RegQueryValueEx(hKey, valName, NULL, &dwType,(LPBYTE)&dwValue, &dwSize)))
		{
			DBGLOG("%s:%s=%lu", key, valName, dwValue);
		}
	}
	RegCloseKey(hKey);
	return dwValue;
}


BOOL file_exist (const char *fileName)
{
	return fileName!=NULL && GetFileAttributes(fileName)!=INVALID_FILE_ATTRIBUTES;
}

#define BUFF_SIZE 1024
BOOL file_copy(const char *src, const char *tar)
{
	BOOL ret = FALSE;
	if(file_exist(src))
	{
		FILE *srcFile = NULL;
		FILE *tarFile = NULL;
		int error;
		if((error = fopen_s(&tarFile, tar, "w"))==0)
		{
			if((error = fopen_s(&srcFile, tar, "r"))==0)
			{
				char buffer[BUFF_SIZE+1];
				int nread;
				while((nread = fread(buffer, 1, BUFF_SIZE, srcFile)>0))
				{
					fwrite(buffer, 1, nread, tarFile);
				}
				fclose(srcFile);
				ret = TRUE;
			}
			else
			{
				ERRLOG("Failed(%d) opening file-%s", error, srcFile);
			}
			fclose(tarFile);
		}
		else
		{
			ERRLOG("Failed(%d) creating file-%s", error, tarFile);
		}
	}
	return ret;
}

#define MAX_NAME 100
user_info_ro get_logon_info()
{
	static struct _user_info_s *info = NULL;
	if(info == NULL)
	{
		static char userName[MAX_NAME];
		DWORD userNameLen = MAX_NAME;
		if(GetUserName(userName, &userNameLen)==0		)
		{
			ERRLOG("Failed to get user name-Error:%#X", GetLastError());
		}
		else
		{
			static char domain[MAX_NAME];
			PSID sid;
			DWORD sidLen = MAX_NAME;
			DWORD domainLen = MAX_NAME;
			SID_NAME_USE sidNameUse;
			NXL_ALLOCS(sid, PSID, sidLen);
			NXL_ALLOC(info, struct _user_info_s);
			info->name = NULL;
			info->domain = NULL;
			info->sid = NULL;
			//set user name
			DBGLOG("User Name(%d)=%s", userNameLen, userName);
			info->name = userName;
			if(LookupAccountName(NULL, userName, sid, &sidLen, domain, &domainLen, & sidNameUse)==0)
			{
				DBGLOG("Failed to LookupAccountName-Error:%#X", GetLastError());
			}
			else
			{
				char *sidStr;
				if(ConvertSidToStringSid(sid, &sidStr))
				{
					//set user sid
					DBGLOG("Sid=%s", sidStr);
					info->sid = string_dup(sidStr);
					LocalFree(sidStr);
				}
				//set domain name
				DBGLOG("domain(%d)=%s", domainLen, domain);
				info->domain = domain;
				//show sidNameUse
				DBGLOG("SidNameUse=%d", sidNameUse);
			}
			NXL_FREE(sid);
			MOCK_FREE(info);//cache;
		}
	}
	return info;
}

int get_service_info(const char *serviceName, int query, service_info_t *result)
{
	DWORD error = 0;
	if(result==NULL) return 1;
	result->isInstalled = FALSE;
	result->isRunning = FALSE;
	result->cmd[0] = '\0';

	if(!string_isNullOrSpace(serviceName))
	{
		SC_HANDLE hscm = NULL;
			
		CALL_DTL(hscm = OpenSCManager(NULL, NULL, GENERIC_READ));
		if(hscm != NULL)
		{
			SC_HANDLE hsc;
			CALL_DTL(hsc = OpenService(hscm, serviceName, GENERIC_READ));

			if (hsc == NULL)
			{
				DBGLOG("Failed to Open Service-'%s'.(%#X)", serviceName, error = GetLastError());
			} 
			else
			{
				DWORD dwBytesNeeded;

				result->isInstalled = TRUE;
				DBGLOG("Service-%s is installed", serviceName);

				if(query & QUERY_STATUS)
				{
					SERVICE_STATUS_PROCESS ssStatus; 
					if (QueryServiceStatusEx(
							hsc,                     // handle to service 
							SC_STATUS_PROCESS_INFO,         // information level
							(LPBYTE) &ssStatus,             // address of structure
							sizeof(SERVICE_STATUS_PROCESS), // size of structure
							&dwBytesNeeded ) )              // size needed if buffer is too small
					{
						DBGLOG("Status=%d Running=%d", ssStatus.dwCurrentState, SERVICE_RUNNING);
						if(ssStatus.dwCurrentState = SERVICE_RUNNING)
						{
							result->isRunning = TRUE;
						}
					}
					else
					{
						DBGLOG("QueryServiceStatusEx failed (%#X)", error = GetLastError());
					}
				}//QUERY_STATUS
				if(query & QUERY_CMD)
				{
					if( !QueryServiceConfig(hsc, NULL, 0, &dwBytesNeeded))
					{
						DWORD dwError = GetLastError();
						if( ERROR_INSUFFICIENT_BUFFER == dwError )
						{
							LPQUERY_SERVICE_CONFIG config;
							DWORD cbBufSize = dwBytesNeeded;
							DBGLOG("cbBufSize=%lu", cbBufSize);
							NXL_ALLOCS(config, LPQUERY_SERVICE_CONFIG, cbBufSize);
							if( !QueryServiceConfig(hsc, config, cbBufSize, &dwBytesNeeded) )
							{
								DBGLOG("QueryServiceConfig failed (%#X)", error = GetLastError());
							}
							else
							{
								DBGLOG("Executable File Path=%s", config->lpBinaryPathName);
								strcpy_s(result->cmd, MAX_PATH, config->lpBinaryPathName);
							}
							NXL_FREE(config);
						}
						else
						{
							DBGLOG("QueryServiceConfig failed (%#X)", error = dwError);
						}
					}//if( !QueryServiceConfig(hsc, NULL, 0, &dwBytesNeeded))
				}//QUERY_PATH

				CloseServiceHandle(hsc);
			}//else(hsc!=NULL)
			CloseServiceHandle(hscm);
		}//if(hscm!=NULL)
		else
		{
			DBGLOG("Failed to Open SCManager.(%#X)", error = GetLastError());
		}
	}
	return error;
}
BOOL get_module_dir(const char *moduleName, char *dirBuf, int bufSize)
{
	if(!string_isNullOrSpace(moduleName))
	{
		HMODULE module;
		//initialize current module path
		if((module = GetModuleHandle(moduleName)) != NULL)
		{
			int len  = GetModuleFileName(module, dirBuf, bufSize);
			if(len > 0)
			{
				//DBGLOG("GetModuleFileName=%s", dirBuf);
				PathRemoveFileSpec(dirBuf);
				return TRUE;
			}
			else
			{
				ERRLOG("Failed to GetModuleFileName!(ErrorCode:%#X)", GetLastError());
			}
		}
		else
		{
			ERRLOG("Failed to GetModuleHandle(%s)!(ErrorCode:%#X)", moduleName, GetLastError());
		}
	}
	return FALSE;
}

BOOL module_query_ver(const char *moduleName, int *major, int *minor, int *maint, int *build)
{
	DWORD dwHandle, size;
    if( !string_isNullOrSpace(moduleName)
		&& (size = GetFileVersionInfoSizeA( moduleName, & dwHandle )) > 0 )
    {
		void *buf;
		NXL_ALLOCN(buf, char, size);
		if ( GetFileVersionInfoA( moduleName, dwHandle, size, buf ) )
		{
			VS_FIXEDFILEINFO * pvi;
			size = sizeof( VS_FIXEDFILEINFO );
			if ( VerQueryValueA( buf, "\\", (LPVOID*)&pvi, (unsigned int*)&size ) )
			{
				//DBGLOG("VerQueryValueA():FileVersionMS=%#X FileVersionLS=%#X", pvi->dwFileVersionMS, pvi->dwFileVersionLS);
				if(major != NULL) *major = pvi->dwFileVersionMS >> 16;
				if(minor != NULL) *minor = pvi->dwFileVersionMS & 0xFFFF;
				if(maint != NULL) *maint = pvi->dwFileVersionLS >> 16;
				if(build != NULL) *build = pvi->dwFileVersionLS & 0xFFFF;
				NXL_FREE(buf);
				return TRUE;
			}
			else
			{
				ERRLOG("VerQueryValueA('%s') failed(ErrorCode:%#X)", moduleName, GetLastError());
			}
		}
		else
		{
				ERRLOG("GetFileVersionInfoA('%s') failed(ErrorCode:%#X)", moduleName, GetLastError());
		}
		NXL_FREE(buf);
    }
	else
	{
		ERRLOG("GetFileVersionInfoSizeA('%s') failed(ErrorCode:%#X)", moduleName, GetLastError());
	}
	return FALSE;
}

#define PIPE_BUFFER_SIZE 1024
static char readBuf[PIPE_BUFFER_SIZE+1] = { 0 };
static DWORD nread = 0;
/*
	Creating a Child Process with Redirected Input and Output
	Refer to:https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
*/
BOOL process_read_output(char *cmd, string_list_p lines, int timeout)
{
	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;
	HANDLE hReadPipe = NULL;
	HANDLE hWritePipe = NULL;
	BOOL ret = FALSE;
	SECURITY_ATTRIBUTES saAttr;

	//Create Read and WritePipes for STDOUT of child process
	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 
	// Create a pipe for the child process's STDOUT.  
	if ( ! CreatePipe(&hReadPipe, &hWritePipe, &saAttr, PIPE_BUFFER_SIZE) ) 
	{
		ERRLOG(TEXT("CreatePipe Failed(%lu)"), GetLastError()); 
		goto CLOSE_PIPES;
	}
	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0) )
	{
		ERRLOG("SetHandleInformation Failed(%lu)", GetLastError()); 
		goto CLOSE_PIPES;
	}

	// Set up members of the PROCESS_INFORMATION structure.  
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection. 
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = hWritePipe;//GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput = hWritePipe;
	//siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;
  
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
   
	//https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx
	//It is important for the parent process to close its handle to the write end of the pipe before calling ReadFile
	if(hWritePipe != NULL)
	{
		CloseHandle(hWritePipe);
	}
	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
	{
		ERRLOG(TEXT("CreateProcess('%s') Failed(%lu)"), cmd, GetLastError());
		goto CLOSE_PIPES;
	}
	DBGLOG("New Process Created:%lu('%s')", piProcInfo.dwProcessId, cmd);
	// Close handles to the child process and its primary thread.
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	//DBGLOG("Reading Pipe...");
	while(ReadFile(hReadPipe, readBuf, PIPE_BUFFER_SIZE, &nread, NULL))
	{
		DBGLOG("ReadPipe(%d):'%s'", nread, readBuf);
		if(nread > 0 && lines != NULL)
		{
			DBGLOG("==>nLines=%d", string_split(readBuf, "\r\n", lines));
		}
		//reset buffer
		memset(readBuf, 0, sizeof(readBuf));
	}
	switch(GetLastError())
	{
		case ERROR_SUCCESS:
			break;
		case ERROR_BROKEN_PIPE:
			DBGLOG("ReadFile() finished as pipe closed");
			break;
		default:
			REPORT_ERROR(GetLastError(), "ReadFile()");
			break;
	}
	ret = TRUE;
CLOSE_PIPES:
	if(hReadPipe != NULL)
	{
		CloseHandle(hReadPipe);
	}
	return ret && (lines == NULL || lines->count > 0);
}

BOOL process_search_child(DWORD parentId, DWORD *childPid, char nameBuffer[MAX_PATH])
{
	BOOL found = FALSE;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hProcessSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof( PROCESSENTRY32 );
		if(Process32First( hProcessSnap, &pe32))
		{
			do
			{
				if(pe32.th32ParentProcessID == parentId)
				{
					found = TRUE;
					*childPid = pe32.th32ProcessID;
					strcpy(nameBuffer, pe32.szExeFile);
					DBGLOG("[PROCESS-%lu]FOUND!Name='%s' ParentPid=%lu", pe32.th32ProcessID, pe32.szExeFile, pe32.th32ParentProcessID);
					break;
				}
				else
				{
					DBGLOG("[PROCESS-%lu]Name='%s' ParentPid=%lu", pe32.th32ProcessID, pe32.szExeFile, pe32.th32ParentProcessID);
				}
			}
			while( Process32Next( hProcessSnap, &pe32 ) );
		}
		else
		{
			ERRLOG("Process32First() failed:%lu", GetLastError());
		}
		CloseHandle( hProcessSnap );          // clean the snapshot object
	}
	else
	{
		ERRLOG("CreateToolhelp32Snapshot() failed:%lu", GetLastError());
	}
	return found;
}

NXLAPI WCHAR *string_to_wstr(const char *str)
{
	if(str != NULL)
	{
		size_t strLen = strlen(str) + 1;
		size_t converted;
		WCHAR *wstr;
		NXL_ALLOCN(wstr, wchar_t, strLen);

		CALL_DTL(mbstowcs_s(&converted, wstr, strLen, str, _TRUNCATE));
		DBGLOG("Converted '%s' to WCHAR(%d):'%S'", str, converted, wstr);
		return wstr;
	}
	return NULL;
}
char *IP_get_local()
{
	char *ip = NULL;
	char nameBuf[255];
	struct hostent *hostInfo;
	int error;
	WSADATA wsaData;
	CALL_DTL(error = WSAStartup(MAKEWORD(2, 2), &wsaData));
	if(error == 0)
	{
		CALL_DTL((error = gethostname(nameBuf, sizeof(nameBuf))));
		if(error==0)
		{
			DBGLOG("Local Host Name : %s", nameBuf);
			CALL_DTL(hostInfo = gethostbyname(nameBuf));
			if(hostInfo != NULL && hostInfo->h_length > 0)
			{
				ip = inet_ntoa (*(struct in_addr *)*hostInfo->h_addr_list);
				DBGLOG("Local IP address is '%s'", ip);
			}
			else
			{
				ERRLOG("Failed to gethostbyname(%#X)!", WSAGetLastError());
			}
		}
		else
		{
			DBGLOG("Failed to gethostname(%#X)", WSAGetLastError());
		}
		//clean
		WSACleanup();
	}
	else
	{
		ERRLOG("Failed to WSAStartup(%#X)", WSAGetLastError());
	}
	return ip;
}

const char *host_get_name()
{
	static char hostname[MAX_PATH] = "";
	if(string_isNullOrSpace(hostname))
	{
		int error;
		WSADATA wsaData;
		if((error = WSAStartup(MAKEWORD(2, 2), &wsaData)) == 0)
		{
			if((error = gethostname(hostname, sizeof(hostname)))==0)
			{
				//DBGLOG("Local Host Name : %s", hostname);
			}
			else
			{
				DBGLOG("Failed(%d) to gethostname(%#X)", error, WSAGetLastError());
				hostname[0] = '\0';
			}
			//clean
			WSACleanup();
		}
		else
		{
			ERRLOG("Failed(%d) to WSAStartup(%#X)", error, WSAGetLastError());
		}
	}
	return hostname;
}
string_list_p dir_list_files(const char *dir)
{
	WIN32_FIND_DATA ffd;
	char pattern[MAX_PATH];
	string_list_p files = string_list_new();
	sprintf_s(pattern, MAX_PATH, "%s\\*", dir);
	HANDLE hFind = FindFirstFile(pattern, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
		return files;
	DBGLOG("Enumerating-'%s'...", pattern);
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ffd.cFileName[0] != '.')
			{
				DBGLOG("\t%s <dir>", ffd.cFileName);
			}
		}
		else
		{
			DBGLOG("\t%s", ffd.cFileName);
			string_list_add(files, ffd.cFileName);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	return files;
}
#endif
