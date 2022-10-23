#include "StdAfx.h"
#include "AcRMXRPMDirMgr.h"
#include "AcRMXUtils.h"

#include <filedialogcallback.h>

#include <LibRMX.h>
#include <RegKeyEntry.h>
#include "AcRMXHooks.h"

using namespace std;

#define REG_QUERY_SAM_DESIRED  KEY_QUERY_VALUE|KEY_WOW64_64KEY
#define REG_SETVALUE_SAM_DESIRED  KEY_WRITE|KEY_WOW64_64KEY

#define LOG_CALL_ENTER(msg) AC_LOG_ENTER_BODY(L"RPMDIR", msg) // log guard helper

void RMXNavDlgCB(CAcUiNavDialog* pDlg) {
	
	LOG_CALL_ENTER(L"");
	RPMDirMgr().UpdateWorkingDir();
}

void CAcRMXRPMDirMgr::Start()
{
	LOG_INFO(L"Starting RPM Dir Manager...");

	acedRegisterNavDialogCallbackFunction(&RMXNavDlgCB);

	// When REMEMBERFOLDERS is 0, startin folder will be used as default path 
	// for all standard file selection dialog boxes in first time.
	// Of course, the last used path is still used in next time. 
	int remberFolder;
	if (AcRMXUtils::GetSysVar(/*MSGO*/L"REMEMBERFOLDERS", remberFolder) && remberFolder == 0)
	{
		LOG_INFO(L"REMEMBERFOLDERS=0");
		CString startDir;
		if (AcRMXUtils::GetSysVar(/*MSGO*/L"STARTINFOLDER", startDir) && !startDir.IsEmpty())
		{
			LOG_INFO(L"Adding STARTINFOLDER as RPM dir: " << (LPCWSTR)startDir);
			AddRPMDir((LPCWSTR)startDir);
		}
	}
}

void CAcRMXRPMDirMgr::Stop()
{
	LOG_INFO(L"Stoppping  RPM Dir Manager...");

	acedUnregisterNavDialogCallbackFunction(&RMXNavDlgCB);

	set<wstring, ICaseKeyLess> rpmDirsToSave;
	for (const auto& dir : m_addedRPMDirs)
	{
		// In case it's failed to unmark RPM folder, remember in registry for tracking
		if (!RMX_RemoveRPMDir(dir.c_str()))
			rpmDirsToSave.insert(dir);
	}
}

void CAcRMXRPMDirMgr::AddRPMDir(const std::wstring& strDir)
{
	LOG_CALL_ENTER(L"");

	if (strDir.empty())
		return;

	wstring strTempDir(strDir);
	// Ignore the last path separator
	if (strTempDir.back() == CPathEx::PATH_DELIMITER_1)
		strTempDir.pop_back();

	if (m_addedRPMDirs.find(strTempDir) != m_addedRPMDirs.end())
		return;

	if (!RMX_IsRPMFolder(strTempDir.c_str()) && RMX_AddRPMDir(strTempDir.c_str()))
	{
		m_addedRPMDirs.insert(strTempDir);
	}
}

wstring CAcRMXRPMDirMgr::GetNavDlgInitialFolder() const
{
	// Note 1: GetDefaultInitialFolder from CAcUiNavDialog not working. It is not reflected immidiately
	// in NavDlgCB, so read value from registry directly.
	// Note 2. System variable "WORKINGFOLDER" is also not refelected in time in NavDlgCB. Cannnot grab it. 
	
	// Get current profile's name
	CString strProfileName;
	if (!AcRMXUtils::GetSysVar(/*MSGO*/L"CPROFILE", strProfileName) || strProfileName.IsEmpty())
		return wstring();

	LOG_DEBUG(L"CPROFILE=" << (LPCTSTR)strProfileName);

	// Get the profile's initial dir of select file dialog from the registry
	const ACHAR* regRoot = acdbHostApplicationServices()->getUserRegistryProductRootKey();
	auto strSubKey = RMXUtils::FormatString(/*MSGO*/L"%s\\Profiles\\%s\\Dialogs\\Select File", regRoot, strProfileName);
	CRegKeyEntry regKey;
	wstring strDir;
	regKey.QueryStringValue(HKEY_CURRENT_USER, strSubKey, /*MSGO*/L"InitialDirectory", REG_QUERY_SAM_DESIRED, strDir);
	LOG_DEBUG_FMT(L"InitialDirectory registry retrieved(Key=HKEY_CURRENT_USER\\%s, Value=%s)", strSubKey.c_str(), strDir.c_str());

	// Replace path for "Desktop" known folder
	if (wcsicmp(strDir.c_str(), L"Desktop") == 0)
	{
		PWSTR pszPath = NULL;
		if (SHGetKnownFolderPath(FOLDERID_Desktop, KF_FLAG_DEFAULT, NULL, &pszPath) != S_OK)
		{
			LOG_ERROR(L"Cannot retreive known folder path for Desktop");
			strDir.clear();
		}
		else
		{
			strDir.assign(pszPath);
			CoTaskMemFree(pszPath);
		}
	}
	return strDir;
}

void CAcRMXRPMDirMgr::UpdateWorkingDir()
{
	// Set the initial folder as RPM dir.
	const wstring& strDir = GetNavDlgInitialFolder();
	LOG_INFO(L"Adding InitialDirectory as RPM dir: " << strDir.c_str());
	AddRPMDir(strDir);
}