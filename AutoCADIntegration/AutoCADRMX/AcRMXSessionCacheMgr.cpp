#include "StdAfx.h"
#include "AcInc.h"
#include "AcRMXSessionCacheMgr.h"
#include "AcRMXUtils.h"
#include <LibRMX.h>

void CAcRMXSessionCacheMgr::Start()
{
}

void CAcRMXSessionCacheMgr::Stop()
{
	m_nxlCacheMap.clear();
	
	LOG_INFO(L"SessionCache erased");
}

bool CAcRMXSessionCacheMgr::GetTags(const ACHAR* szFileName, std::string& strTags) const
{
	auto foundIter = m_nxlCacheMap.find(szFileName);
	if (foundIter != m_nxlCacheMap.end())
	{
		strTags = foundIter->second.strTags;
		return true;
	}

	return false;
}

bool CAcRMXSessionCacheMgr::GetTagsWithRefs(AcApDocument* pDoc, std::string& strTags) const
{
	if (pDoc == nullptr)
		return false;

	std::string retTags;
	if (GetTags(pDoc->fileName(), retTags))
		strTags = retTags;

	// Grab all tags from xref if any. 
	AcRMXUtils::TraverseXrefs([&](const TCHAR* szXrefFullPath) -> bool {
		// Any file does not have granted right, stop travering and return.
		if (RMX_IsProtectedFile(szXrefFullPath))
		{
			// Extract tags which will be used when protecting the file.
			char* szTags = nullptr;
			RMX_GetTags(szXrefFullPath, &szTags);
			if (szTags != nullptr)
			{
				strTags += szTags;
				RMX_ReleaseArray((void*)szTags);
			}
		}

		return false;
	}, pDoc->database());

	AcRMXUtils::TraverseUnderlayRefs([&](const TCHAR* szRefFullPath) -> bool {
		if (szRefFullPath && RMX_IsProtectedFile(szRefFullPath))
		{
			// Extract tags which will be used when protecting the file.
			char* szTags = nullptr;
			RMX_GetTags(szRefFullPath, &szTags);
			if (szTags != nullptr)
			{
				strTags += szTags;
				RMX_ReleaseArray((void*)szTags);
			}
		}

		return false;
	}, pDoc->database());

	return strTags.empty() ? false : true;
}

bool CAcRMXSessionCacheMgr::CheckRight(const ACHAR* szFileName, DWORD dwRight) const
{
	return RMX_CheckRights(szFileName, dwRight, true);
}

bool CAcRMXSessionCacheMgr::CheckRightOnXrefs(DWORD dwRight, std::set<std::wstring>& denyFiles, AcDbDatabase* pDB)
{
	AcRMXUtils::TraverseXrefs([&](const TCHAR* szXrefFullPath) -> bool {
		// Any file does not have granted right, stop travering and return.
		if (RMX_IsProtectedFile(szXrefFullPath) && !RMX_CheckRights(szXrefFullPath, dwRight, true))
		{
			denyFiles.insert(szXrefFullPath);
			return true;
		}

		return false;
	}, pDB);

	return (denyFiles.size() == 0);
}

bool CAcRMXSessionCacheMgr::HasNxlRefLoaded(AcDbDatabase* pDB /*= nullptr*/) const
{
	bool hasProtected = false;
	AcRMXUtils::TraverseXrefs([&](const TCHAR* szXrefFullPath) -> bool {
		// Any file does not have granted right, stop travering and return.
		if (szXrefFullPath && RMX_IsProtectedFile(szXrefFullPath))
		{
			hasProtected = true;
			return true;
		}

		return false;
	}, pDB);

	AcRMXUtils::TraverseUnderlayRefs([&](const TCHAR* szRefFullPath) -> bool {
		// get the filepath for this reference
		if (szRefFullPath && RMX_IsProtectedFile(szRefFullPath))
		{
			hasProtected = true;
			return true;
		}

		return false;
	}, pDB);

	return hasProtected;
}

bool CAcRMXSessionCacheMgr::CheckRightOnUnderlays(DWORD dwRight, std::set<std::wstring>& denyFiles, AcDbDatabase* pDB)
{
	AcRMXUtils::TraverseUnderlayRefs([&](const TCHAR* szRefFullPath) -> bool {
		if (szRefFullPath && RMX_IsProtectedFile(szRefFullPath) &&
			!RMX_CheckRights(szRefFullPath, dwRight, true))
		{
			denyFiles.insert(szRefFullPath);
			return true;
		}
		return false;
	}, pDB);

	return (denyFiles.size() == 0);
}

bool CAcRMXSessionCacheMgr::GetNxlRefFiles(std::set<std::wstring>& refFiles) const
{
	AcRMXUtils::TraverseXrefsForCurrentDoc([&](const TCHAR* szXrefFullPath) -> bool {
		if (szXrefFullPath && RMX_IsProtectedFile(szXrefFullPath))
		{
			refFiles.insert(szXrefFullPath);
		}

		return false;
	});

	AcRMXUtils::TraverseUnderlayRefsForCurrent([&](const TCHAR* szRefFullPath) -> bool {
		if (szRefFullPath && RMX_IsProtectedFile(szRefFullPath))
		{
			refFiles.insert(szRefFullPath);
		}
		return false;
	});

	return (refFiles.size() > 0);
}

bool CAcRMXSessionCacheMgr::IsAllowSaveChanges(const ACHAR* szFileName) const
{
	if (IsNxlFile(szFileName) && AcRMXUtils::IsDocModified()
		&& !CheckRight(szFileName, RMX_RIGHT_EDIT))
		return false;

	return true;
}

bool CAcRMXSessionCacheMgr::HasNxlFileLoaded() const
{
	bool ret = false;
	AcApDocManager* pDocMgr = acDocManager;
	if (pDocMgr != NULL)
	{
		AcApDocumentIterator* pIter = pDocMgr->newAcApDocumentIterator();
		if (pIter != NULL)
		{
			for (; !pIter->done(); pIter->step()) {

				if (IsNxlFile(pIter->document()->fileName()))
				{
					ret = true;
					break;
				}
				// TODO: Scane all Xrefs and Underlays?
				// Now all functions for references are implemented based on working database.
				// If we want to access refs for any file which is not working database, have to re-impl
				// traverse functions. 
			}
			delete pIter;
		}
	}

	if (!ret)
		ret = SessionCacheMgr().HasNxlRefLoaded();

	return false;
}

bool CAcRMXSessionCacheMgr::AddRecoverFile(const std::wstring& strFileName)
{
	if (m_nxlRecoverFiles.find(strFileName) != m_nxlRecoverFiles.end())
		return true; // Skip if it's done already

	size_t nPos = strFileName.find(L"_recover");
	if (nPos != std::wstring::npos)
	{
		std::wstring strOrigFileName = strFileName.substr(0, nPos);
		CPathEx fPathEx(strFileName.c_str());
		strOrigFileName += fPathEx.GetExtension();
		
		if (IsNxlFile(strOrigFileName.c_str()))
		{
			m_nxlRecoverFiles.insert(strFileName);
			LOG_DEBUG(L"Recover file from nxl file caught: " << strFileName.c_str());
			return true;
		}
	}
	return false;
}

void CAcRMXSessionCacheMgr::ClearupReoverFiles()
{
	for (const auto& f : m_nxlRecoverFiles)
	{
		if (DeleteFile(f.c_str()))
		{
			LOG_INFO_FMT(L"Recover file auto-deleted: %s", f.c_str());
		}
		else
		{
			LOG_ERROR_FMT("Failed to delete recover file (error: %s)", (LPCTSTR)CSysErrorMessage(GetLastError()));
		}
	}
	m_nxlRecoverFiles.clear();
}

bool CAcRMXSessionCacheMgr::AddNxlFile(const ACHAR* szFileName)
{
	if (szFileName)
	{
		if (!RMX_IsProtectedFile(szFileName))
			return false;

		TAcRMXNxlFile acFile;
		acFile.strFileName = szFileName;

		// Extract tags which will be used when protecting the file.
		char* szTags = nullptr;
		RMX_GetTags(szFileName, &szTags);
		if (szTags != nullptr)
		{
			acFile.strTags = szTags;
			RMX_ReleaseArray((void*)szTags);
		}

		m_nxlCacheMap[szFileName] = acFile;
		LOG_DEBUG_FMT(L"Nxl file added into cache: %s", szFileName);

		return true;

	}

	return false;
}

void CAcRMXSessionCacheMgr::AddNxlFile(const ACHAR* szFileName, const std::string& strTags)
{
	TAcRMXNxlFile acFile;
	acFile.strFileName = szFileName;
	acFile.strTags = strTags;
	m_nxlCacheMap[szFileName] = acFile;
	LOG_DEBUG_FMT(L"Nxl file added into cache: %s", szFileName);
}

void CAcRMXSessionCacheMgr::RemoveNxlFile(const ACHAR* szFileName)
{
	if (m_nxlCacheMap.find(szFileName) != m_nxlCacheMap.end()) {
		m_nxlCacheMap.erase(szFileName);
		LOG_DEBUG_FMT(L"Nxl file deleted from cache: %s", szFileName);
	}
		
}

bool CAcRMXSessionCacheMgr::IsNxlFile(const ACHAR* szFileName) const
{
	if (szFileName == nullptr) return false;

	if (m_nxlCacheMap.find(szFileName) != m_nxlCacheMap.end())
		return true;

	return false;
}
