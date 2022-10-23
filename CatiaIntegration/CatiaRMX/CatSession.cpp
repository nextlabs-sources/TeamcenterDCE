#include "stdafx.h"
#include "CatSession.h"
#include <LibRMX.h>
#include "WatermarkControl.h"
#include "RMXUsageControl.h"
#include "CatHooks.h"
#include "ProtectControl.h"
#include "ScreenCaptureControl.h"

bool CCatSession::GetDocFileName(CAT::DocumentPtr pDoc, wstring& retFileFullName)
{
	CComBSTR bstr;
	if ((pDoc == NULL) || FAILED(pDoc->get_FullName(&bstr)))
	{
		LOG_ERROR(L"Failed to get full name");
		return false;
	}
	
	retFileFullName = StringHlp::bstr2ws(bstr.m_str);
	return true;
}

void CCatSession::Start()
{
	LOG_DEBUG(L"Connecting to Catia session...");

	::CoInitialize(NULL);

	HRESULT hr = S_OK;
	hr = m_pCatApp.GetActiveObject("CATIA.Application");
	if (FAILED(hr))
	{
		LOG_ERROR(L"Caita application not running");
		return;
	}

	LOG_INFO(L"Caita application is running");
	LOG_INFO(L"Caita Information:");
	//
	// Dump to app information
	//
	CAT::SystemConfigurationPtr pSysConfig;
	
	if (SUCCEEDED(m_pCatApp->get_SystemConfiguration(&pSysConfig) && pSysConfig))
	{
		CComBSTR bstrOS;
		pSysConfig->get_OperatingSystem(&bstrOS);
		wstring wsOS = StringHlp::bstr2ws(bstrOS.m_str);
		LOG_INFO(L"\t-OS: " << wsOS.c_str());
		
		long lR, lSR, lV;
		pSysConfig->get_Release(&lR);
		pSysConfig->get_ServicePack(&lSR);
		pSysConfig->get_Version(&lV);
		LOG_INFO(L"\t-Release number: " << lR);
		LOG_INFO(L"\t-Service pack number: " << lSR);
		LOG_INFO(L"\t-Version number: " << lV);
	}
	CComBSTR bstrAppPath;
	m_pCatApp->get_FullName(&bstrAppPath);
	LOG_INFO(L"\t-FullName: " << StringHlp::bstr2ws(bstrAppPath.m_str).c_str());
	bstrAppPath.Empty();
	m_pCatApp->get_Path(&bstrAppPath);
	m_wsAppDir = StringHlp::bstr2ws(bstrAppPath.m_str);
	LOG_INFO(L"\t-AppPath: " << m_wsAppDir.c_str());

	CAT::SystemServicePtr pSysSrv;
	if (SUCCEEDED(m_pCatApp->get_SystemService(&pSysSrv) && pSysSrv))
	{
		CComBSTR bstrEnv(L"CATInstallPath");
		CComBSTR bstrEnvVal;
		if (SUCCEEDED(pSysSrv->Environ(&bstrEnv, &bstrEnvVal)))
		{
			m_wsAppInstallDir = StringHlp::bstr2ws(bstrEnvVal.m_str);
			LOG_INFO(L"\t-InstallPath: " << m_wsAppInstallDir.c_str());
		}
	}
	CComBSTR bstrSearchOrder;
	m_pCatApp->get_FileSearchOrder(&bstrSearchOrder);
	wstring wsSearchOrder = StringHlp::bstr2ws(bstrSearchOrder.m_str);
	LOG_INFO(L"\t-SearchOrder: " << wsSearchOrder.c_str());
	
	m_bAppUIMode = true;
	const auto& cmdLines = RMXUtils::GetCommandLines();
	for (auto& cmdLine : cmdLines)
	{
		if (wcsstr(cmdLine.c_str(), L"-batch") != NULL)
		{
			m_bAppUIMode = false;
			break;
		}
	}
	LOG_INFO(L"\t-BatchMode: " << !m_bAppUIMode);
	LOG_INFO(L"\t-ProcessID: " << ::GetCurrentProcessId());
	
	LOG_INFO("RMX Information:");
	LOG_INFO_FMT(L"\t-Version: %s", _W(VER_FILEVERSION_STR));
	LOG_INFO_FMT(L"\t-Build: %s", _W(VER_BUILD_STR));
	TCHAR   szDllDir[MAX_PATH] = { 0 };
	DWORD dwSize = GetModuleFileName((HINSTANCE)&__ImageBase, szDllDir, _countof(szDllDir));
	if (dwSize != 0 && dwSize != MAX_PATH)
	{
		LOG_INFO_FMT(L"\t-Path: %s", szDllDir);
		ShowStatusBar(IDS_INFO_RMX_LOADED, szDllDir);
	}

	m_hMainWnd = NULL;
	const auto& vecTopWnds = RMXUtils::FindCurrentProcessMainWindow();
	for (const auto& hWnd : vecTopWnds)
	{
		wchar_t szClassName[MAX_PATH];
		GetClassName(hWnd, szClassName, MAX_PATH);
		if (wcsstr(szClassName, L"CATDlgDocument") != NULL)
		{
			m_hMainWnd = hWnd;
			break;
		}
	}
	
	//
	// Set up default RPM folder
	//
	wstringstream tokValues(wsSearchOrder);
	wstring val;
	vector<wstring> searchDirs;
	// Tokenizing value of ValueList param by ';'
	while (getline(tokValues, val, L';'))
	{
		if (CPathEx::IsDir(val))
		{
			if (RMX_AddRPMDir(val.c_str()))
				LOG_INFO(L"Added Search folder as RPM folder: " << val.c_str());
		}
	}

	CPathEx rpmPath = RMXUtils::getEnv(L"USERPROFILE").c_str();
	rpmPath /= L"Documents\\DassaultSystemes\\CATRMX Workspace";
	if (!CPathEx::IsDir(rpmPath.ToString()))
	{
		int nRet = CPathEx::CreateDirectories(rpmPath.ToString());
		if (nRet != ERROR_SUCCESS)
		{
			LOG_ERROR_FMT(L"Failed to create directory: '%s' (error: %s)", rpmPath.c_str(), (LPCWSTR)CSysErrorMessage(nRet));
			return;
		}

		LOG_INFO(L"Default workspace dir created: " << rpmPath.c_str());
	}

	if (RMX_AddRPMDir(rpmPath.c_str()))
		LOG_INFO(L"Default workspace set to RPM folder: " << rpmPath.c_str());

	//
	// Init other modules
	//
	curEventMgr.Start();
	// Register event listeners
	// Note: Ensure to register session before others, due to other components like ucControl
	// are dependending on the cache data in session. Always refelect session on event to update cache firstly
	curEventMgr.AddListener((CCatEventLisener*)this, eCatNotify_AfterCommand | eCatNotify_FrmLayoutChanged);

	curUCControl.Start();
	curWatermarkControl.Start();
	curProtectControl.Start();
	curScnCaptureControl.Start();

	CCatHooks::InstallHooks();
}

void CCatSession::Stop()
{
	LOG_DEBUG(L"Disconnecting from Catia session...");
	
	curUCControl.Stop();
	curWatermarkControl.Stop();
	curProtectControl.Stop();
	curScnCaptureControl.Stop();

	CCatHooks::UninstallHooks();

	curEventMgr.Stop();

	m_pCatApp.Release();
	::CoUninitialize();
}

HWND CCatSession::GetMainWnd() const
{
	return m_hMainWnd;
}

CAT::DocumentPtr CCatSession::GetActiveDocument()
{
	if (!m_pCatApp)
	{
		LOG_ERROR(L"Catia session not connected");
		return NULL;
	}
	
	CAT::DocumentPtr pDoc;
	if (FAILED(m_pCatApp->get_ActiveDocument(&pDoc)))
		return NULL;

	return pDoc.Detach();
}

CAT::WindowPtr CCatSession::GetActiveWindow()
{
	if (!m_pCatApp)
	{
		LOG_ERROR(L"Catia session not connected");
		return NULL;
	}

	CAT::WindowPtr pWnd;
	if (FAILED(m_pCatApp->get_ActiveWindow(&pWnd)))
		return NULL;

	return pWnd.Detach();
}

CAT::DocumentPtr CCatSession::FindDocument(const wstring& wsFileFullName)
{
	CAT::DocumentPtr pRet = NULL;
	curCatSession.WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
		wstring fName;
		if (!CCatSession::GetDocFileName(pDoc, fName))
			return false;

		if (StringHlp::ICompare(fName, wsFileFullName))
		{
			pRet = pDoc.Detach();
			return true;
		}

		return false;
	});

	return pRet;
}

bool CCatSession::AddNxlFile(const wstring & wsFileFullName)
{
	// Alway skip the files in install folder.
	// Which will cause many isprotect calls when loading a file. 
	if (wsFileFullName.find(m_wsAppDir) == 0)
		return false;

	if (wsFileFullName.empty())
		return false;

	if (!RMX_IsProtectedFile(wsFileFullName.c_str()))
		return false;

	TCatRMXNxlFile nxFile;
	nxFile.m_wsFileFullName = wsFileFullName;

	// Extract permissions as cache backup.
	// R13.3: Still request evaluation for each permission check. In case no right found
	// by evaluation, will check through cached permission here
	// for the known issue after save: EvalRights: failed, error: -11
	nxFile.m_dwRights = 0;
	RMX_GetRights(wsFileFullName.c_str(), nxFile.m_dwRights);

	// Extract tags which will be used when protecting the file.
	char* szTags = nullptr;
	RMX_GetTags(wsFileFullName.c_str(), &szTags);
	if (szTags != nullptr)
	{
		nxFile.m_strTags = szTags;
		RMX_ReleaseArray((void*)szTags);
	}

	

	m_nxlCacheMap[wsFileFullName] = nxFile;

	LOG_DEBUG_FMT(L"Nxl file added into cache: %s", wsFileFullName.c_str());

	return true;

}

void CCatSession::RemoveNxlFile(const wstring& wsFileFullName)
{
	if (m_nxlCacheMap.find(wsFileFullName) != m_nxlCacheMap.end())
	{
		m_nxlCacheMap.erase(wsFileFullName);
		LOG_DEBUG_FMT(L"Nxl file erased from cache: %s", wsFileFullName.c_str());
	}
}

void CCatSession::RefreshNxlCache()
{
	// Update the cache list based on latest openning list, in case of any file gets closed in memory
	// Note: Current method only support to be called when "close" command is fired. Don't call for "open" command.
	std::map<std::wstring, TCatRMXNxlFile, ICaseKeyLess> backupCache = m_nxlCacheMap;
	m_nxlCacheMap.clear();
	LOG_DEBUG(L"Refreshing the nxl files in cache...");
	bool ret = WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
		wstring wsCurFullName;
		if (GetDocFileName(pDoc, wsCurFullName))
		{
			LOG_DEBUG_FMT(L"Opened file: %s", wsCurFullName.c_str());
			auto iter = backupCache.find(wsCurFullName);
			if (iter != backupCache.end()) {
				m_nxlCacheMap[iter->first] = iter->second;
				LOG_DEBUG_FMT(L"Nxl file: %s", iter->first.c_str());
			}
		}

		return false;
	});
}

bool CCatSession::IsNxlFile(const wstring & wsFileFullName) const
{
	return (m_nxlCacheMap.find(wsFileFullName) != m_nxlCacheMap.end());
}

bool CCatSession::IsNxlFile(CAT::DocumentPtr pDoc) const
{
	if (pDoc)
	{
		wstring wsFileFullName;
		if (GetDocFileName(pDoc, wsFileFullName) && IsNxlFile(wsFileFullName))
			return true;
	}
	return false;
}

string CCatSession::GetTags(const wstring& wsFileFullName) const
{
	auto iter = m_nxlCacheMap.find(wsFileFullName);
	if (iter != m_nxlCacheMap.end())
		return iter->second.m_strTags;

	return "";
}

bool CCatSession::CheckRight(const wstring & wsFileFullName, DWORD dwRight, bool audit /*= true*/) const
{
	// TODO: Always use cached permission?

	// R13.3: Still trigger right evaluation firstly. If it's failed, reuse cached permission
	// to determine. 
	DWORD dwAllRights = 0;
	RMX_GetRights(wsFileFullName.c_str(), dwAllRights);
	if (dwAllRights == 0)
	{
		LOG_WARN(L"No permission retrieved(error: EvalRights failed). Attempting to grab from cache...");
		auto iter = m_nxlCacheMap.find(wsFileFullName);
		if ((iter != m_nxlCacheMap.end()) && (iter->second.m_dwRights != 0))
		{
			dwAllRights = iter->second.m_dwRights;
		}
	}
	
	// Reuse cache permissions
	return RMX_HasRights(wsFileFullName.c_str(), dwAllRights, dwRight, audit);	
}

void CCatSession::OnAfterCommand(LPCatCommandNotify pParam)
{
	if (StringHlp::ICompare(pParam->wsCmdName.c_str(), CatRMX::CMD_FILELOAD))
	{
		AddNxlFile(pParam->wsFileFullName);
	}
}

void CCatSession::OnFrmLayoutChanged(LPCatFrmNotify pParam)
{
	if (StringHlp::ICompare(pParam->wsCmdName.c_str(), CatRMX::CMD_FRMCLOSEBOX))
	{
		RefreshNxlCache();
	}
}

void CCatSession::DisplayAlert(UINT uMsBoxType, UINT nMsgFmtID, ...)
{
	CString csFmt;
	if (!csFmt.LoadString((HINSTANCE)&__ImageBase, nMsgFmtID))
	{
		LOG_ERROR_FMT(L"%d resource string not loaded", nMsgFmtID);
		return;
	}

	va_list vl;
	va_start(vl, nMsgFmtID);

	CString csMsg;
	csMsg.FormatMessageV(csFmt, &vl);

	va_end(vl);

	switch (uMsBoxType)
	{
	case MB_ICONWARNING:
		LOG_WARN((LPCWSTR)csMsg);
		break;
	case MB_ICONERROR:
		LOG_ERROR((LPCWSTR)csMsg);
	default:
		break;
	}

	static int nBatchMode = -1;
	if (nBatchMode == -1)
	{
		const auto& cmdLines = RMXUtils::GetCommandLines();
		for (auto& cmdLine : cmdLines)
		{
			if (wcsstr(cmdLine.c_str(), L"-batch") != NULL)
			{
				nBatchMode = 1;
				break;
			}
		}

		if (nBatchMode == -1) nBatchMode = 0;
	}
	if (nBatchMode == 0)
	{
		static CString csTitle;
		if (csTitle.IsEmpty())
		{
			csTitle.LoadString((HINSTANCE)&__ImageBase, IDS_DLG_TITLE);
		}
		::MessageBox(curCatSession.GetMainWnd(), (LPCWSTR)csMsg, (LPCWSTR)csTitle, uMsBoxType);
	}
}

void CCatSession::ShowStatusBar(UINT nMsgFmtID, ...)
{
	CString csFmt;
	if (!csFmt.LoadString((HINSTANCE)&__ImageBase, nMsgFmtID))
	{
		LOG_ERROR_FMT(L"%d resource string not loaded", nMsgFmtID);
		return;
	}

	va_list vl;
	va_start(vl, nMsgFmtID);

	CString csMsg;
	csMsg.FormatMessageV(csFmt, &vl);
	va_end(vl);
	LOG_INFO((LPCWSTR)csMsg);

	if (m_bAppUIMode)
	{
		CComBSTR bstrText((LPCWSTR)csMsg);
		m_pCatApp->put_StatusBar(&bstrText);
	}
}

HWND CCatSession::GetFullScreenTopWindow() const
{
	const auto& vecTopWnds = RMXUtils::FindCurrentProcessMainWindow();
	for (const auto& hWnd : vecTopWnds)
	{
		wchar_t szText[MAX_PATH];
		GetWindowText(hWnd, szText, MAX_PATH);
		if (_wcsicmp(szText, L"VisuFullScreen") == 0)
		{
			return hWnd;
		}
	}

	return NULL;
}

bool CCatSession::IsFullScreen() const
{
	auto hWnd = GetFullScreenTopWindow();
	return (hWnd != NULL);
}

//bool CCatSession::HasNxlRefLoaded() const
//{
//	// TODO: Traverse the children
//	return false;
//}
//
//bool CCatSession::CheckRightOnXrefs(DWORD dwRights, set<wstring>& denyFiles)
//{
//	// TODO: Traverse the children
//	return false;
//}
