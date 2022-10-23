#include "RMXMgr.h"


CRMXMgr::CRMXMgr()
{
}


CRMXMgr::~CRMXMgr()
{
}

CRMXMgr& CRMXMgr::GetInstance()
{
	static CRMXMgr s_instance;
	return s_instance;
}

bool CRMXMgr::ProtectFile(const tstring& fileFullName)
{
	wstring wstrCmdLine = wstrAppPath + L" " + wstrOrigFilePath;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	WCHAR wzCmdLine[MAX_PATH] = L"";

	wsprintfW(wzCmdLine, L"%s %s", wstrAppPath.c_str(), wstrOrigFilePath.c_str());
	LOG_INFO(_T("Command line:%s"), wzCmdLine);
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcessW(
		NULL,
		wzCmdLine,
		NULL,
		NULL,
		false,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&si,
		&pi))
	{
		PrintLog("ProcessId=%d, ThreadId=%d", pi.dwProcessId, pi.dwThreadId);
		hProcess = pi.hProcess;
		result = pDRmInstance->RPMAddTrustedProcess(pi.dwProcessId, MAGIC_SECURITY);
		if (result != (int)SDWL_SUCCESS)
		{
			PrintLog("Fail with calling ISDRmInstance::RPMAddTrustedProcess (error:%s)", result.GetMsg().c_str());
			return false;
		}
		if (ResumeThread(pi.hThread) == -1)
		{
			PrintLog(L"ResultThread(hThread=%d) failed with error code %d", pi.hThread, GetLastError());
			return false;
		}
	}
	else
	{
		PrintLog("Fail with calling CreateProcess() with error code %d", GetLastError());
		return false;
	}
}