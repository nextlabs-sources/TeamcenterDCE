#include "RMXUtils.h"

#include <cctype>
#include <ctime>
#include <cwctype>
#include <algorithm>
#include <direct.h>
#include <iomanip>
#include <shellapi.h>
#include <Shlobj.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>

namespace RMXUtils
{
	std::wstring s2ws(const std::string & str)
	{
		return s2ws(str.c_str());
	}

	std::wstring s2ws(const char * szStr)
	{
		if (szStr == nullptr || strlen(szStr) == 0)
			return std::wstring();

		int len = (int)strlen(szStr) + 1;
		int size = MultiByteToWideChar(CP_UTF8, 0, szStr, len, 0, 0);
		wchar_t* buf = new wchar_t[size];
		MultiByteToWideChar(CP_UTF8, 0, szStr, len, buf, size);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	std::string ws2s(const std::wstring & wstr)
	{
		return ws2s(wstr.c_str());
	}

	std::string ws2s(const wchar_t * szStr)
	{
		if (szStr == nullptr || wcslen(szStr) == 0)
			return std::string();

		int len = (int)wcslen(szStr) + 1;
		int size = WideCharToMultiByte(CP_UTF8, 0, szStr, len, 0, 0, 0, 0);
		char* buf = new char[size];
		WideCharToMultiByte(CP_UTF8, 0, szStr, len, buf, size, 0, 0);
		std::string r(buf);
		delete[] buf;
		return r;
	}

	std::wstring getCurrentTime()
	{
		std::wstring currTime;
		std::time_t tNow = std::time(nullptr);
		wchar_t wstr[100];
		if (std::wcsftime(wstr, 100, L"%c", std::localtime(&tNow))) {
			currTime.assign(wstr);
		}
		return currTime;
	}

	std::wstring getEnv(const std::wstring & envName)
	{
		wchar_t* envVar;
		size_t requiredSize;
		std::wstring retVar;
		_wgetenv_s(&requiredSize, NULL, 0, envName.c_str());
		if (requiredSize>0)
		{
			envVar = new wchar_t[requiredSize * sizeof(wchar_t)];
			if (envVar)
			{
				// Get the value of the environment variable.
				if (_wgetenv_s(&requiredSize, envVar, requiredSize, envName.c_str()) == 0)
					retVar = envVar;
				delete[] envVar;
			}			
		}
		return retVar;
	}

#define PIPE_BUFFER_SIZE 1024
/*
Creating a Child Process with Redirected Input and Output
Refer to:https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
*/
bool ExecuteCmd(const std::wstring& cmd, std::string& output)
{
	bool ret = false;

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL bSuccess = FALSE;
	HANDLE hReadPipe = NULL;
	HANDLE hWritePipe = NULL;	
	SECURITY_ATTRIBUTES secAttr;
	//Create Read and WritePipes for STDOUT of child process
	// Set the bInheritHandle flag so pipe handles are inherited. 
	secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttr.bInheritHandle = TRUE;
	secAttr.lpSecurityDescriptor = NULL;
	// Create a pipe for the child process's STDOUT.  
	if (!CreatePipe(&hReadPipe, &hWritePipe, &secAttr, PIPE_BUFFER_SIZE))
	{
		goto CLOSE_PIPES;
	}
	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
	{
		goto CLOSE_PIPES;
	}

	// Set up members of the PROCESS_INFORMATION structure.  
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection. 
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = hWritePipe;//GetStdHandle(STD_ERROR_HANDLE);
	si.hStdOutput = hWritePipe;
	//siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// Create the child process.  
	LPWSTR szCmd = const_cast<wchar_t*>(cmd.c_str());
	bSuccess = CreateProcess(NULL,
		szCmd,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&si,  // STARTUPINFO pointer 
		&pi);  // receives PROCESS_INFORMATION 

	//https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx
	//It is important for the parent process to close its handle to the write end of the pipe before calling ReadFile
	if (hWritePipe != NULL)
	{
		CloseHandle(hWritePipe);
	}
	// If an error occurs, exit the application. 
	if (!bSuccess)
	{
		goto CLOSE_PIPES;
	}
	//LOG_DEBUG_FMT(L"\t->New process created: %lu", pi.dwProcessId);
		
	// Close handles to the child process and its primary thread.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	char chReadBuf[PIPE_BUFFER_SIZE + 1] = { 0 };
	DWORD dwRead = 0;
	while (ReadFile(hReadPipe, chReadBuf, PIPE_BUFFER_SIZE, &dwRead, NULL))
	{
		//LOG_DEBUG(L"\t->Read out from pipe: " << RMXUtils::s2ws(std::string(chReadBuf)));
		if (dwRead > 0)
		{
			output.append(chReadBuf);
		}
		//reset buffer
		memset(chReadBuf, 0, sizeof(chReadBuf));
	}
	DWORD err = GetLastError();
	switch (err)
	{
	case ERROR_SUCCESS:
		ret = true;
		break;
	case ERROR_BROKEN_PIPE:
		// It's not error
		//LOG_DEBUG(L"\t->Finished to read output from pipe");
		ret = true;
		break;
	default:
		break;
	}
CLOSE_PIPES:
	if (hReadPipe != NULL)
	{
		CloseHandle(hReadPipe);
	}
	//LOG_DEBUG(L"\t->End of executing command line");
	return ret;
}

#pragma comment(lib,"rpcrt4.lib")
std::wstring GenerateUUID()
{
	std::wstring retGUID(L"");
	UUID uuid = { 0 };
	RPC_STATUS ret = UuidCreate(&uuid);
	if (ret != RPC_S_OK)
	{
		return retGUID;
	}
	wchar_t *uuidStr = nullptr;
	ret = UuidToString(&uuid, (RPC_WSTR*)&uuidStr);
	if (ret != RPC_S_OK)
	{
		return retGUID;
	}
	retGUID = uuidStr;
	RpcStringFree((RPC_WSTR*)&uuidStr);
	return retGUID;
}

std::vector<std::wstring> GetCommandLines()
{
	std::vector<std::wstring> cmdLines;
	LPWSTR *szArglist = nullptr;
	int nArgs = 0;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist != nullptr)
	{
		for (int i = 0; i < nArgs; i++)
		{
			cmdLines.push_back(szArglist[i]);
		}
		// Free memory allocated for CommandLineToArgvW arguments.
		LocalFree(szArglist);
	}

	return cmdLines;
}

std::wstring GetFilePathByHandle(HANDLE hFile)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return std::wstring();

	std::wstring retFilePath;
	wchar_t szFilePath[MAX_PATH];
	DWORD dwRet = GetFinalPathNameByHandle(hFile, szFilePath, MAX_PATH, VOLUME_NAME_DOS);
	if (dwRet < MAX_PATH)
		retFilePath = szFilePath;
	else if (dwRet >= MAX_PATH)
	{
		wchar_t* szFilePath2 = new wchar_t[dwRet * sizeof(wchar_t)];
		DWORD dwRet2 = GetFinalPathNameByHandle(hFile, szFilePath2, dwRet, VOLUME_NAME_DOS);
		if (dwRet2 < dwRet)
			retFilePath = szFilePath2;
		delete[] szFilePath2;
	}
	
	if (!retFilePath.empty())
	{
		if (retFilePath.substr(0, 8).compare(L"\\\\?\\UNC\\") == 0)
		{
			// In case of a network path, replace `\\?\UNC\` with `\\`.
			retFilePath = L"\\" + retFilePath.substr(7);
		}
		else if (retFilePath.substr(0, 4).compare(L"\\\\?\\") == 0)
		{
			// In case of a local path, crop `\\?\`.
			retFilePath = retFilePath.substr(4);
		}
		return retFilePath;
	}

	return std::wstring();
}

std::wstring GetCurrentDir()
{
	std::wstring retDir;
	wchar_t szDir[MAX_PATH];
	DWORD dwRet = GetCurrentDirectory(MAX_PATH, szDir);
	if (dwRet < MAX_PATH)
		retDir = szDir;
	else if (dwRet >= MAX_PATH)
	{		
		wchar_t* szDir2 = new wchar_t[dwRet * sizeof(wchar_t)];
		DWORD dwRet2 = GetCurrentDirectory(dwRet, szDir2);
		if (dwRet2 < dwRet)
			retDir = szDir2;
		delete[] szDir2;
	}

	if (!retDir.empty())
	{
		if (retDir.substr(0, 8).compare(L"\\\\?\\UNC\\") == 0)
		{
			// In case of a network path, replace `\\?\UNC\` with `\\`.
			retDir = L"\\" + retDir.substr(7);
		}
		else if (retDir.substr(0, 4).compare(L"\\\\?\\") == 0)
		{
			// In case of a local path, crop `\\?\`.
			retDir = retDir.substr(4);
		}
		return retDir;
	}

	return std::wstring();
}

std::wstring GetCurrentProcessImagePath()
{
	wchar_t szProcessName[MAX_PATH];
	DWORD dwRet = GetModuleFileName(NULL, &szProcessName[0], MAX_PATH);
	if (dwRet > 0 && dwRet < MAX_PATH)
	{
		return std::wstring(szProcessName);
	}

	return std::wstring();
}

std::vector<HWND> FindProcessMainWindow(DWORD dwProcessId)
{
	typedef std::pair<DWORD, std::vector<HWND>> EnumWindowParams;
	EnumWindowParams params;
	params.first = dwProcessId;
	BOOL bResult = EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL
	{
		auto pParams = (EnumWindowParams*)(lParam);
		DWORD dwProcessId;
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		if (dwProcessId == pParams->first)
		{
			pParams->second.push_back(hWnd);
		}

		return TRUE;

	}, (LPARAM)&params);

	return params.second;
}

std::vector<HWND> FindCurrentProcessMainWindow()
{
	return FindProcessMainWindow(::GetCurrentProcessId());
}

std::wstring _getComputerNameExW(COMPUTER_NAME_FORMAT nameFmt)
{
	std::wstring retName;
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH;
	wchar_t szName[MAX_COMPUTERNAME_LENGTH];
	if (!GetComputerNameEx(nameFmt, szName, &dwSize))
	{
		if (GetLastError() == ERROR_MORE_DATA)
		{
			wchar_t* szName2 = new wchar_t[dwSize];
			if (GetComputerNameEx(nameFmt, szName2, &dwSize))
			{
				retName = szName2;
			}
			delete[] szName2;
		}
	}
	else
	{
		retName = szName;
	}

	return retName;
}

std::wstring GetHostName()
{
	auto domain = _getComputerNameExW(ComputerNameDnsDomain);
	if (domain.empty())
		return _getComputerNameExW(ComputerNameDnsHostname);
	else
		return _getComputerNameExW(ComputerNameDnsFullyQualified);
}

std::string FormatString(const char* format, ...)
{
	va_list vl;
	int     len = 0;
	std::string s;

	va_start(vl, format);
	len = _vscprintf_l(format, 0, vl) + 1;
	static const int DEFAULT_BUFFERSIZE = 1024;
	std::vector<char> buffer(DEFAULT_BUFFERSIZE);
	int size = _vsnprintf_s(&buffer[0], buffer.size() - 1, _TRUNCATE, format, vl);
	if (size >= DEFAULT_BUFFERSIZE)
	{
		buffer.resize(size + 1);
		va_list vlCopy;
		va_copy(vlCopy, vl);
		_vsnprintf_s(&buffer[0], buffer.size(), _TRUNCATE, format, vlCopy);
		va_end(vlCopy);
	}

	s.append(&buffer[0]);

	va_end(vl);

	return std::move(s);
}

std::wstring FormatString(const wchar_t* format, ...)
{
	va_list vl;
	int     len = 0;
	std::wstring s;

	va_start(vl, format);
	static const int DEFAULT_BUFFERSIZE = 1024;
	std::vector<wchar_t> buffer(DEFAULT_BUFFERSIZE);

	int size = _vsnwprintf_s(&buffer[0], buffer.size() - 1, _TRUNCATE, format, vl);
	if (size >= DEFAULT_BUFFERSIZE)
	{
		buffer.resize(size + 1);
		va_list vlCopy;
		va_copy(vlCopy, vl);
		_vsnwprintf_s(&buffer[0], buffer.size(), _TRUNCATE, format, vlCopy);
		va_end(vlCopy);
	}

	s.append(&buffer[0]);

	va_end(vl);

	return std::move(s);
}

template<typename CharT>
static inline void TrimLeft(std::basic_string<CharT>& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

template<typename CharT>
static inline void TrimRight(std::basic_string<CharT>& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

std::string & Trim(std::string & s)
{
	// Trim leading and trailing space
	TrimLeft<char>(s);
	TrimRight<char>(s);
	return s;
}

std::wstring & Trim(std::wstring & ws)
{
	// Trim leading and trailing space
	TrimLeft<wchar_t>(ws);
	TrimRight<wchar_t>(ws);
	return ws;
}
} // namespace RMXUtils
