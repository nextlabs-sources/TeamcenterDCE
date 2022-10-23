#include <stdio.h>
#include <Shlwapi.h>
#include <madCHook.h>
#include <hook/rmc_fixes.h>
#include <NxlUtils/Windows/ntdll.h>

#include <NxlUtils/NxlDllLoader.hpp>
#include <NxlUtils/nxlrunner.exe.hpp>
#include "stdafx.h"

#define NXL_UTILS_DLL L"NxlUtils64.dll"
#define RPM_UTILS_DLL L"RPMUtils64.dll"
BOOL init_nxlutils(const char *pname)
{
	NxlDllLoader loader;
	if(loader.Load(NXL_UTILS_DLL))
	{
		if (loader.Load(RPM_UTILS_DLL))
		{
			g_rpmSession = new RPMSession(RPM_MAGIC_SECURITY);
			return TRUE;
		}
	}
	return FALSE;
}

extern BOOL iges_install();
extern void iges_uninstall();
extern BOOL step_install();
extern void step_uninstall();
extern BOOL cgm2pdf_install();
extern void cgm2pdf_uninstall();
extern BOOL cmd_install();
extern void cmd_uninstall();
extern BOOL ugto2d_install();
extern void ugto2d_uninstall();
extern BOOL dxfdwg_install();
extern void dxfdwg_uninstall();
extern BOOL wavefront_install();
extern void wavefront_uninstall();

typedef BOOL (*pfInstall)();
typedef void (*pfUninstall)();
typedef struct hook_installer_s
{
	const TCHAR *process_name;
	pfInstall install;
	pfUninstall uninstall;	
}hook_installer_t;

static hook_installer_t s_entries[] = 
{
	{TEXT("visview.exe"), fix_stat64_install, NULL},//currently we only fix the file size issue for visview.exe
	{ TEXT("iges.exe"), iges_install, NULL},
	{ TEXT("step203ug.exe"), step_install, NULL},
	{ TEXT("step214ug.exe"), step_install, NULL},
	{ TEXT("cgm2pdf.exe"), cgm2pdf_install, NULL},
	{ TEXT("cmd.exe"), cmd_install, NULL},
	{ TEXT("ugto2d.exe"), ugto2d_install, NULL},
	{ TEXT("dxfdwg.exe"), dxfdwg_install, NULL},
	{TEXT("wavefrontobj.exe"), wavefront_install, NULL}
};

WCHAR **cmd_args = NULL;
int cmd_nargs = 0;
void load_cmd_args()
{
	HANDLE phandle = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	if(phandle != NULL)
	{
		//show command line information
		PROCESS_BASIC_INFORMATION baseInformation;
		NTSTATUS ntstatus = NtQueryInformationProcess_inner(phandle, ProcessBasicInformation, &baseInformation, sizeof(baseInformation), NULL);
		if(NT_SUCCESS(ntstatus))
		{
			PPEB peb= (PPEB)baseInformation.PebBaseAddress;
			if(peb != NULL)
			{
				PRTL_USER_PROCESS_PARAMETERS params = (PRTL_USER_PROCESS_PARAMETERS)peb->ProcessParameters;
				if(params != NULL)
				{
					DBGLOG("ImagePathName='%S'", params->ImagePathName.Buffer);
					cmd_args = CommandLineToArgvW(params->CommandLine.Buffer, &cmd_nargs);
					int iarg = 0;
					for(iarg = 0; iarg < cmd_nargs; iarg++)
					{
						DBGLOG("CommandArgs[%d]='%S'", iarg, cmd_args[iarg]);
					}
				}
			}
		}
		else
		{
			DBGLOG("NtQueryInformationProcess_inner() returns %X", ntstatus);
		}
	}
	CloseHandle(phandle);
}
BOOL WINAPI DllMain( IN HINSTANCE hDllHandle, 
         IN DWORD     nReason, 
         IN LPVOID    Reserved )
{
	wchar_t buff[MAX_PATH];
	int len = GetModuleFileNameW(hDllHandle, buff, MAX_PATH);
	NxlPath moduleFile(buff);
	GetModuleFileNameW(NULL, buff, MAX_PATH);
	NxlPath processFullName(buff);
	const TCHAR *pName = PathFindFileName(processFullName.tstr());
	switch (nReason)
	{
		case DLL_PROCESS_ATTACH:
			RMXUtils::initializeLogger();
			DBGLOG("[%s]+++++++++++++++++DLL_PROCESS_ATTACH+++++++++++++(%s)", processFullName.tstr(), moduleFile.tstr());
			if(pName != NULL)
			{
				int i;
				int n = sizeof(s_entries)/sizeof(hook_installer_t);
				for(i=0; i < n; i++)
				{
					if(_tcsicmp(pName, s_entries[i].process_name) == 0)
					{
						if(init_nxlutils(pName))
						{
							load_cmd_args();
							//init madc hook
							InitializeMadCHook();
							DBGLOG("MadCHook initialized");
							SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);
								//NxlRunner runner;
								//runner.SetTrustApp();
							if(s_entries[i].install())
							{
								DBGLOG("Install Success!");
								return TRUE;
							}
							DBGLOG("Install failed!");
						}
						break;
					}
				}
			}
			//remove current process from trusted
			if (g_rpmSession != nullptr) {
				g_rpmSession->SetTrustedProcess(GetCurrentProcessId(), false);
			}
			return FALSE;
		case DLL_THREAD_ATTACH:
			//DBGLOG("<<<<<<<<<<<<<<<DLL_THREAD_ATTACH...");
			//nx_hook_delayed();
			break;
		case DLL_THREAD_DETACH:
			//DBGLOG("DLL_THREAD_DETACH>>>>>>>>>>>>>>>>>>");
			break;
		case DLL_PROCESS_DETACH:
			TRACELOG("unloading lib-'%s'", moduleFile.tstr());
			if(pName != NULL && g_rpmSession != nullptr)
			{
				int i;
				int n = sizeof(s_entries)/sizeof(hook_installer_t);
				for(i=0; i < n; i++)
				{
					if(stricmp(pName, s_entries[i].process_name) == 0)
					{
						//free cmd_args
						LocalFree(cmd_args);
						if (s_entries[i].uninstall != NULL)
						{
							s_entries[i].uninstall();
						}
						FinalizeMadCHook();
						DBGLOG("MadCHook Finalized");
						break;
					}
				}
			}
			DBGLOG("[%s]--------------DLL_PROCESS_DETACH------------------", pName);
			log4cplus::Logger::shutdown();
			DBGVLOG("[%s]--------------DLL_PROCESS_DETACH------------------", pName);
			break;
	}
	return TRUE;
}