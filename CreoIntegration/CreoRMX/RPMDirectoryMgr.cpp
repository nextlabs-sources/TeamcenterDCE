#include "RPMDirectoryMgr.h"

#include "..\common\CreoRMXLogger.h"
#include <RegKeyEntry.h>
#include "OtkXUtils.h"
#include <PathEx.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>

#include <pfcGlobal.h>

#include <LibRMX.h>
#include "ProtectController.h"

using namespace std;
using namespace OtkXUtils;

#define REG_KEY_CREORMX L"Software\\NextLabs\\CreoRMX"
#define REG_VALUE_RPMDIR L"RPMDir"
#define REG_QUERY_SAM_DESIRED  KEY_QUERY_VALUE|KEY_WOW64_64KEY
#define REG_SETVALUE_SAM_DESIRED  KEY_WRITE|KEY_WOW64_64KEY


void CDirectoryChangeListener::OnAfterDirectoryChange(xrstring path)
{
	RPMDIR_LOG_ENTER(RMXUtils::s2ws(path).c_str());
	string strPath(path);
	wstring wstrPath = RMXUtils::s2ws(strPath);
	// Don't set RPM folder for the temp work dir during export
	if (CProtectController::GetInstance().IsWatchingExport())
		return;
	
	CRPMDirectoryMgr::GetInstance().UpdateCurrentDirectory(wstrPath);
}

void CRPMDirectoryMgr::Start()
{
	LOG_INFO(L"Starting RPM directory control...");
	m_initialWorkDir = OtkX_GetCurrentDirectory();

	if (m_initialWorkDir.back() == CPathEx::PATH_DELIMITER_1)
		m_initialWorkDir.pop_back();

	// Register session level action listeners
	m_pDirChangeListener = new CDirectoryChangeListener();
	try
	{
		pfcSession_ptr pSession = pfcGetProESession();
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}
		pSession->AddActionListener(m_pDirChangeListener);
		LOG_INFO(L"\t-CDirectoryChangeListener added");
	}	
	OTKX_EXCEPTION_HANDLER();
}

void CRPMDirectoryMgr::Stop()
{
	LOG_INFO(L"Stopping RPM directory control...");
	try
	{
		pfcSession_ptr pSession = pfcGetProESession();
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}
		
		set<wstring, ICaseKeyLess> rpmDirsToSave;
		// Remove RPM dirs which were added in the session, except to the
		// current work dir		
		m_addedRPMDirs.erase(m_initialWorkDir);
		for (const auto& dir : m_addedRPMDirs)
		{
			// In case it's failed to unmark RPM folder, remember in registry for tracking
			if(!RMX_RemoveRPMDir(dir.c_str()))
				rpmDirsToSave.insert(dir);
		}

		// Save the rpm dirs to registry in case we need to track in future which
		// ones are added by CreoRMX.
		
		if (RMX_IsRPMFolder(m_initialWorkDir.c_str()))
			rpmDirsToSave.insert(m_initialWorkDir);
		
		// Ensure all saved dirs are still valid
		CRegKeyEntry regKey;
		vector<wstring> regValues;
		regKey.QueryMultiStringValue(HKEY_CURRENT_USER, REG_KEY_CREORMX, REG_VALUE_RPMDIR, REG_QUERY_SAM_DESIRED, regValues);
		for (auto& dir : regValues)
		{
			if (!CPathEx::IsDir(dir))
				continue;

			if (RMX_IsRPMFolder(dir.c_str()))
				rpmDirsToSave.insert(dir);
		}

		if (regValues.size() != rpmDirsToSave.size())
		{
			LOG_INFO(L"Saving RPM dir changes to registry...");
			regValues.clear();
			regValues.insert(regValues.begin(), rpmDirsToSave.begin(), rpmDirsToSave.end());
			LONG ret = regKey.SetMultiStringValue(HKEY_CURRENT_USER, REG_KEY_CREORMX, REG_VALUE_RPMDIR, regValues, REG_SETVALUE_SAM_DESIRED);
			if (REG_FAILED(ret))
			{
				const wstring keyStr = L"HKEY_CURRENT_USER\\" + wstring(REG_KEY_CREORMX);
				LOG_WARN_FMT(L"Failed to save the registry value '%s' to '%s' (error: %s)",
					keyStr.c_str(), REG_VALUE_RPMDIR, (LPCTSTR)CSysErrorMessage(ret));
				return;
			}

			for (const auto& dir : rpmDirsToSave)
			{
				LOG_INFO(L"\t-" << dir.c_str());
			}
		}

		if (m_pDirChangeListener)
			pSession->RemoveActionListener(m_pDirChangeListener);
	}
	OTKX_EXCEPTION_HANDLER();
}

void CRPMDirectoryMgr::UpdateCurrentDirectory(const wstring& newDir)
{
	if (newDir.empty())
		return;

	wstring newDirTemp(newDir);
	// Ignore the last path separator
	if (newDirTemp.back() == CPathEx::PATH_DELIMITER_1)
		newDirTemp.pop_back();

	
	if (!RMX_IsRPMFolder(newDirTemp.c_str()) && RMX_AddRPMDir(newDirTemp.c_str()))
	{
		m_addedRPMDirs.insert(newDirTemp);
		if (wcsicmp(newDirTemp.c_str(), m_currentDirectory.c_str()) == 0)
		{
			LOG_INFO_FMT(L"Set RPM working directory: '%s'", newDirTemp.c_str());
		}
		else
		{
			LOG_INFO_FMT(L"RPM working directory changed(Old: '%s', New: '%s')", m_currentDirectory.c_str(), newDirTemp.c_str());
			m_currentDirectory = newDirTemp;
		}
	}
}

void CRPMDirectoryMgr::SyncCurrentDirectory()
{
	// Ensure the current directory is up-to-date
	// Sometimes, the export process will temporarily change the current directory but
	// OnAfterDirectoryChange listener is not triggered. 
	const wstring& currDir = OtkX_GetCurrentDirectory();
	UpdateCurrentDirectory(currDir);
}

void CRPMDirectoryMgr::AddRPMDir(const std::wstring & dir)
{
	if (dir.empty())
		return;

	wstring newDirTemp(dir);
	// Ignore the last path separator
	if (newDirTemp.back() == CPathEx::PATH_DELIMITER_1)
		newDirTemp.pop_back();

	if (wcsicmp(newDirTemp.c_str(), m_currentDirectory.c_str()) == 0)
		return;

	if (!RMX_IsRPMFolder(newDirTemp.c_str()) && RMX_AddRPMDir(newDirTemp.c_str()))
	{
		m_addedRPMDirs.insert(newDirTemp);
	}
}
