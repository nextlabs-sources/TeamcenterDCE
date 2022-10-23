#include "stdafx.h"
#include "CommonUtils.h"
#include <tlhelp32.h>


namespace CommonUtils
{
	bool ExecuteCmd(const std::wstring& cmd) {
		STARTUPINFO si;
		ZeroMemory(&si, sizeof si);
		si.cb = sizeof si;
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof pi);

		LPWSTR szCmd = const_cast<wchar_t*>(cmd.c_str());
		BOOL processCreated = CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

		if (processCreated)
		{
			DWORD dwWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);

			// Close process and thread handles. 
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			// if the process finished, we break out
			if (dwWaitResult == WAIT_OBJECT_0)
			{
				LOG_INFO("Process successfully completed");
			}
			else if (dwWaitResult == WAIT_FAILED)
			{
				dwWaitResult = GetLastError();
				LOG_ERROR("Wait Failed" << dwWaitResult);
			}
			else
			{
				LOG_ERROR("Error occured");
			}
		}
		else
			LOG_ERROR("Process couldn't be created due to error code" << GetLastError());
		return true;
	}

	
	std::wstring ltrim(const wstring& str) {
		const std::wstring whitespaces(L" \t\f\v\n\r");
		size_t pos = str.find_first_not_of(whitespaces);
		if (pos != std::string::npos) {
			return str.substr(pos);
		}
		return L"";
	}

	std::wstring rtrim(const wstring& str) {
		const std::wstring whitespaces(L" \t\f\v\n\r");
		size_t pos = str.find_last_not_of(whitespaces);
		if (pos != std::string::npos) {
			return str.substr(0, pos + 1);
		}
		return L"";
	}

	bool IsLineEmpty(const wstring& str) {
		return ltrim(str).size() == 0 ? true : false;
	}

	bool IsLineComment(const wstring& str) {
		return ltrim(str)[0] == '#' ? true : false;
	}


	bool ParsePropertyLine(const wstring& str, pair<wstring, wstring>& kvp) {
		//Check if the line contains =
		size_t pos = str.find_first_of(L"=");
		if (pos != std::wstring::npos) {
			wstring key = wstring(str.begin(), str.begin() + pos);
			wstring value = wstring(str.begin() + pos + 1, str.end());
			//Remove all leading spaces and trailing spaces from key,value
			key = rtrim(ltrim(key));
			value = rtrim(ltrim(value));
			kvp = make_pair(key, value);
			return true;
		}
		return false;
	}

	void GetAllKeyValueFromPropFile(const wstring& propertyFile, map<wstring, wstring>& kvpMap) {
		std::wifstream file(propertyFile);
		std::wstring str;
		while (std::getline(file, str)) {
			//Check if a line is blank or comment
			if (!IsLineEmpty(str) && !IsLineComment(str)) {
				pair<wstring, wstring> kvp;
				if (ParsePropertyLine(str, kvp)) {
					kvpMap[kvp.first] = kvp.second;
				}
			}
		}
	}

	vector<DWORD> getchpid(DWORD ppid)
	{
		HANDLE hSnapshot;
		PROCESSENTRY32 pe32;
		vector<DWORD> chpids;

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot != INVALID_HANDLE_VALUE) {
			ZeroMemory(&pe32, sizeof(pe32));
			pe32.dwSize = sizeof(pe32);
			if (Process32First(hSnapshot, &pe32)) {
				do {
					if (pe32.th32ParentProcessID == ppid) {
						chpids.push_back(pe32.th32ProcessID);
					}
				} while (Process32Next(hSnapshot, &pe32));
			}
			
			CloseHandle(hSnapshot);
		}

		return chpids;
	}


} // namespace CommonUtils
