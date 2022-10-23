// nxlrunner.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <winerror.h>
#include <Shlobj.h>

#include <FileContext.h>
#include <ApplicationContext.h>
#include <ProcessContext.h>
#include <ntdll.h>
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:wmainCRTStartup")

RPMHelper *g_rpmHelper;

WCHAR sMsgBuf[1024] = TEXT("");
bool enableConsoleLog = false;	//by default disable console log
#define EXISTS(f) (GetFileAttributes(f)!=INVALID_FILE_ATTRIBUTES)
#define REMOVE_NXL_EXT(buf) buf[wcslen(buf)-4] = '\0'

int str_replace_all(const wchar_t *original, const wchar_t *pattern, const wchar_t *newStr, wchar_t **result)
{
	int nReplaced = 0;
	*result = NULL;
	if(original != NULL && pattern != NULL)
	{
		size_t patternLen = wcslen(pattern);
		if(patternLen > 0)
		{
			size_t replacedLen = 0;
			size_t newStrLen = wcslen(newStr);
			const wchar_t *pStart = original;
			while(pStart != NULL)
			{
				const wchar_t *pFound = wcsstr(pStart, pattern);
				size_t headerLen, replacingLen;
				if(pFound != NULL)
				{
					headerLen = pFound - pStart;	//appending the str before pfound
					replacingLen = newStrLen;
				}
				else
				{
					//append all folliwng chars
					headerLen = wcslen(pStart);
					replacingLen = 0;
				}
				if(headerLen + replacingLen > 0)
				{
					replacedLen += headerLen + replacingLen;
					if(*result == NULL)
					{
						*result = (wchar_t*)malloc((replacedLen+1)*sizeof(wchar_t));
						(*result)[0] = '\0';
					}
					else
					{
						*result = (wchar_t*)realloc(*result, (replacedLen+1)*sizeof(wchar_t));
					}
					wcsncat_s(*result, replacedLen+1, pStart, headerLen);//appending header
					if(pFound != NULL)
					{
						wcscat_s(*result, replacedLen+1, newStr);//appending replaced str
					}
				}
				if(pFound != NULL)
				{
					nReplaced++;
					pStart = pFound + patternLen;
				}
				else
					break;
			}
		}
	}
	return nReplaced;
}

int  print_usage()
{
	OutputDebugString(L"usage: nxlrunner -open <filePath> ");
	OutputDebugString(L"usage: nxlrunner -trust <pid> ");
	OutputDebugString(L"usage: nxlrunner -distrust <pid> ");
	OutputDebugString(L"usage: nxlrunner -list-handles <pid> ");
	return 3;
}
int wmain(int argc, wchar_t * argv[])
{
	/*TODO:check RPM installation
	SC_HANDLE hscm=OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        GENERIC_READ);
	if (NULL == hscm) 
    {
		wsprintf(sMsgBuf,L"OpenSCManager failed (%#X)",GetLastError());
        OutputDebugString(sMsgBuf);
        return 1;
    }

	SC_HANDLE hsc = OpenService( 
        hscm,         // SCM database 
        L"nxrmserv",            // name of service 
        GENERIC_READ);

	if (hsc == NULL)
    { 
		DWORD lastErr=GetLastError();
		wsprintf(sMsgBuf,L"OpenService failed (%#X)",lastErr);
		if(lastErr==ERROR_SERVICE_DOES_NOT_EXIST)
			MessageBox(NULL,L"The NextLabs Rights Management Client is not installed, please use Secure View to view the document.",L"NextLabs DRM",MB_OK);
        OutputDebugString(sMsgBuf);
        CloseServiceHandle(hscm);
        return 2;
    }
	else
	{
		CloseServiceHandle(hsc);
		CloseServiceHandle(hscm);
	}
	*/
	//MessageBox(NULL,L"test for lpText para",L"Test Box",1);
	if(argc<3)
	{
		return print_usage();
	}
	const std::wstring op = argv[1];
	const std::wstring opArg = argv[2];
	if (op.empty() || opArg.empty())
	{
		return print_usage();
	}
	if (op == L"-open")
	{
		g_rpmHelper = new RPMHelper();
		DBGLOG("Opening file-'%s'...", opArg.c_str());
	}
	else if (op == L"-trust") {
		try
		{
			g_rpmHelper = new RPMHelper();
			enableConsoleLog = true;
			DWORD pid = std::stoul(opArg);
			ProcessContext p(pid);
			return p.SetAsTrusted(true) ? 1 : 0;
		}
		catch (...) {
			DBGLOG("Exception Caugh!()");
		}
			return 1;
	}
	else if (op == L"-distrust") {
		try
		{
			g_rpmHelper = new RPMHelper();
			enableConsoleLog = true;
			DWORD pid = std::stoul(opArg);
			ProcessContext p(pid);
			return p.RemoveTrust() ? 1 : 0;
		}
		catch (...) {
			DBGLOG("Exception Caugh!()");
		}
		return 1;
	}
	else if (op == L"-list-handles") {
		DWORD pid = std::stoul(opArg);
		if (pid > 0)
		{
			ProcessContext p(pid);
			std::vector<ProcessContext> ps;
			ps.push_back(p);
			DBGLOG("searching handles in process-%lu('%s')", pid, p.full_name().c_str());
			ProcessContext::SearchFileHandlersInProcesses(L"", ps);
			return 0;
		}
		else
		{
			return print_usage();
		}
	}
	else
	{
		return print_usage();
	}

	const FileContext &fileCtx = FileContext(opArg);

	/*
		Process PreStart
	*/

	const ApplicationContext appContext = ApplicationContext::FindAssocApp(fileCtx);

	/* 
	if(_wcsicmp(pContext->ApplicationName, L"PhotoViewer.dll")==0)
	{
		//WORKAROUND for RMC bug: 
		//if the length of OS user name is larger than 6(e.g. administrator)
		//RMC return FILE NOT FOUND after the nxl file is downloaded from Teamcenter to local
		//The API-ILCreateFromPath call will make RMC aware of the exsisence of new downloaded file
		
		DBGLOG("Workaround for PhotoViewer.dll");
		PIDLIST_ABSOLUTE idlist = ILCreateFromPath(origFilePath);
		ILFree(idlist);
	}*/


	/*
		Process Starting
	*/
	__int64 oldNativeWriteTime = 0;
	__int64 oldNxlWriteTime = 0;
	fileCtx.GetFileLastWriteTime(oldNativeWriteTime, oldNxlWriteTime);
	ProcessContext processContext = appContext.Start();

	//failed to start 
	if (!processContext.IsValid())
		return 1;
	
	/*
		Process PostStart
	*/
	processContext.Wait(fileCtx.target_long());
	if (fileCtx.is_nxl_file())
	{
		//check if the file is still protected
		if (!EXISTS(fileCtx.nxl_file().c_str()))
		{
			DBGLOG("Failed to find original protected file-%s", fileCtx.nxl_file().c_str());
			if (!EXISTS(fileCtx.target_path().c_str()))
			{
				DBGLOG("Failed to find target file-%s", fileCtx.target_path().c_str());
			}
			else
			{
				DBGLOG("Somehow the protected file lost protection! rename it as original name to trigger the checkin handler which will reprotect this file!");
				if (MoveFile(fileCtx.target_path().c_str(), fileCtx.nxl_file().c_str()) == 0)
				{
					DBGLOG("==>Rename failed(%#X)", GetLastError());
				}
				else
				{
					DBGLOG("==>Rename Success");
				}
			}
		}
		else
		{
			DWORD minwaittime = reg_get_dword_value(appContext.app_name().c_str(), L"MinWaitTime", 0);
			if (minwaittime > 0) {
				Sleep(minwaittime);
			}
			//check write time
			__int64 newNativeWriteTime = 0;
			__int64 newNxlWriteTime = 0;
			fileCtx.GetFileLastWriteTime(newNativeWriteTime, newNxlWriteTime);
			if (oldNativeWriteTime != newNativeWriteTime) {
				//native file is modified
				DWORD waittime = reg_get_dword_value(appContext.app_name().c_str(), L"MaxWaitTime", 5000);
				if (minwaittime > 0)
					waittime -= minwaittime;
				while (oldNxlWriteTime == newNxlWriteTime
					&& waittime > 0) {
					DBGLOG("Waiting modification sync up...");
					Sleep(1000);
					waittime -= 1000;
					//changes has been sync back
					fileCtx.GetFileLastWriteTime(newNativeWriteTime, newNxlWriteTime);
				}
				if (oldNxlWriteTime == newNxlWriteTime) {
					DBGLOG("TIMEOUT");
				}
				else
				{
					DBGLOG("Modification has been sync back")
				}
			}
		}
	}
	return processContext.exit_code();
}
