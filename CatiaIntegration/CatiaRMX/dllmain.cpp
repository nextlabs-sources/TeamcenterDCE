// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <TlHelp32.h>
#include <DelayLoadDllHlp.h>
#include <LibRMX.h>
#include "CatHooks.h"
#include "CatSession.h"

ENABLE_DELAYLOAD_HOOK

static const wchar_t* DLL_LIBRMX = L"LibRMX_x64.dll";

CPathEx g_rmxRootDir;
HMODULE g_hModule;

extern "C" __declspec(dllexport) void Uninitialize();

BOOL InitRMXLib()
{
	LOG_DEBUG(L"Initializing RMX Lib...");
	// Load library explicitly with a given path
	// Search the LibRMX dll from the same folder of RMX
	CPathEx dllPath(g_rmxRootDir.c_str());
	dllPath /= DLL_LIBRMX;
	ADD_DELAYLOAD_DLL(dllPath.ToString());

	// Init RMX
	try
	{
		if (!RMX_Initialize())
		{
			wchar_t* szErrMsg = nullptr;
			if (RMX_GetLastError(&szErrMsg) != ERROR_SUCCESS)
			{
				ALERT_ERROR(IDS_ERROR_LOADRMX, szErrMsg);
				RMX_ReleaseArray((void*)szErrMsg);
			}

			UnloadDelayLoadedDlls();

			return FALSE;
		}
	}
	catch (...)
	{
		CSysErrorMessage lastError(GetLastError());
		LOG_ERROR_FMT(L"Failed to load '%s' (error: %s)", dllPath.c_str(), (LPCTSTR)lastError);
		return FALSE;
	}

	return TRUE;
}

void UnInitRMXLib()
{
	LOG_DEBUG(L"Uninitialize CatiaRMX");
	RMX_Terminate();

	UnloadDelayLoadedDlls();
}

BOOL PrepareRMXEnv()
{
	const std::wstring& envVal = RMXUtils::getEnv(ENV_LOGCONFIG_DIR);
	if (!envVal.empty())
	{
		g_rmxRootDir = envVal.c_str();
	}
	else
	{
		// Folder structure should be ${CAITA_RMX_ROOT}\AutoCAD 2022\nxrmAutoCADRMX2022.arx
		// The log config file locates in ${CAITA_RMX_ROOT}
		// So grab the RMX root dir based on app path as below
		TCHAR   szDllDir[MAX_PATH] = { 0 };
		DWORD dwSize = GetModuleFileName(g_hModule, szDllDir, _countof(szDllDir));
		if (dwSize != 0 && dwSize != MAX_PATH)
		{
			g_rmxRootDir = szDllDir;
			g_rmxRootDir = g_rmxRootDir.GetParentPath().c_str();
			std::wstring strRMXRootEnv(ENV_LOGCONFIG_DIR);
			strRMXRootEnv += L"=" + g_rmxRootDir.ToString();
			if (_wputenv(strRMXRootEnv.c_str()) != 0) {
				DBGLOG(L"Failed to set CATIA_RMX_ROOT env var");
				return FALSE;
			}
			DBGLOG(L"[INFO]CATIA_RMX_ROOT env var is set: %s", g_rmxRootDir.c_str());
		}
		else
		{
			DBGLOG(L"[ERROR] Cannot set CATIA_RMX_ROOT env var( error: GetModuleFileName failed due to: %s)", (LPCTSTR)CSysErrorMessage(::GetLastError()));
		}
	}

	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hModule = hModule;
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		Uninitialize();
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:	
		break;
	}
	return TRUE;
}

// This function will be invoked first when this dll is loaded into catia process by PRM.
extern "C" __declspec(dllexport)
BOOL Initialize()
{
	// Set up RMX_ROOT env var 
	if (!PrepareRMXEnv())
		return 0;

	RMXLOG_BEGIN();
	LOG_DEBUG(L"Starting to init CatiaRMX...");

	// Fix:
	// Catia is single instance on COM server, it's hard to 
	// retrieve the target session instance via GetActiveObject("CATIA.Application"). It always returns the 1st one.
	PROCESSENTRY32W pe32;
	memset(&pe32, 0, sizeof(pe32));
	pe32.dwSize = sizeof(PROCESSENTRY32W);
	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		LOG_ERROR(L"Failed to take snapshot on processes");
		return FALSE;
	}
	if (!Process32FirstW(hSnapshot, &pe32)) {
		CloseHandle(hSnapshot);
		LOG_ERROR(L"Failed to retrieve the first process");
		return FALSE;
	}

	DWORD dwPID = GetCurrentProcessId();
	do {
		const wchar_t* name = wcsrchr(pe32.szExeFile, L'\\');
		name = (NULL == name) ? pe32.szExeFile : (name + 1);
		if (0 == _wcsicmp(L"cnext.exe", name) && (dwPID != pe32.th32ProcessID))
		{
			CloseHandle(hSnapshot);
			ALERT_WARN(IDS_ERROR_MULTI_PROCESS, std::to_wstring(pe32.th32ProcessID));
			return FALSE;
		}
	} while (Process32NextW(hSnapshot, &pe32));

	CloseHandle(hSnapshot);

	InitRMXLib();
	CCatHooks::Start();

	return TRUE;
}

extern "C" __declspec(dllexport)
void Uninitialize()
{
	LOG_DEBUG(L"Exiting the CatiaRMX...");

	CCatHooks::Stop();
	
	UnInitRMXLib();

	g_hModule = NULL;

	RMXLOG_END();
}