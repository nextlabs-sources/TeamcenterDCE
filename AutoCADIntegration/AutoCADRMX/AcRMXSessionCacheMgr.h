//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//

#pragma once
#include "CommonTypes.h"
#include <map>

struct TAcRMXNxlFile
{
	std::wstring strFileName;
	std::string strTags;
};

class CAcRMXSessionCacheMgr : public RMXControllerBase<CAcRMXSessionCacheMgr>
{
public:
	void Start();
	void Stop();

	bool AddNxlFile(const ACHAR* szFileName);
	void AddNxlFile(const ACHAR* szFileName, const std::string& strTags);
	void RemoveNxlFile(const ACHAR* szFileName);
	bool IsNxlFile(const ACHAR* szFileName) const;
	
	bool GetTags(const ACHAR* szFileName, std::string& strTags) const;
	bool GetTagsWithRefs(AcApDocument* pDoc, std::string& strTags) const;

	// Note : Rights are not designed to be cached. Evaluate on the fly for each call. 
	bool CheckRight(const ACHAR* szFileName, DWORD right) const;
	// To optimize performance, return once any xref does not have permission. 
	// Will not scan all xrefs, so the return set always contains one element now.
	//
	bool CheckRightOnXrefs(DWORD dwRight, std::set<std::wstring>& denyFiles, AcDbDatabase* pDB = nullptr /*nullptr: Current dwg*/);
	bool CheckRightOnUnderlays(DWORD dwRight, std::set<std::wstring>& denyFiles, AcDbDatabase* pDB = nullptr /*nullptr: Current dwg*/);

	// Includes Xrefs and underlays
	bool GetNxlRefFiles(std::set<std::wstring>& refFiles) const;

	bool HasNxlRefLoaded(AcDbDatabase* pDB = nullptr /*nullptr: Current dwg*/) const;
	bool IsAllowSaveChanges(const ACHAR* szFileName) const;
	bool HasNxlFileLoaded() const;
	bool HasNxlFileOpened() const { return !m_nxlCacheMap.empty(); }

	bool AddRecoverFile(const std::wstring& strFileName);
	void ClearupReoverFiles();

public:
	std::map<std::wstring, TAcRMXNxlFile, ICaseKeyLess> m_nxlCacheMap;
	std::set<std::wstring> m_nxlRecoverFiles;
};

DEFINE_RMXCONTROLLER_GET(CAcRMXSessionCacheMgr, SessionCacheMgr)