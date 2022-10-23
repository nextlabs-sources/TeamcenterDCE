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
#include "AcRMXTypes.h"
#include "ProtectJob.h"

class CAcRXMProtectControl : public RMXControllerBase<CAcRXMProtectControl>
{
public:
	void Start();
	void Stop();

	bool AddJob(AcDbDatabase* pDwg, const TCHAR* szFileName);
	bool AddJob(AcPublishReactorInfo* pInfo);
	bool AddExportJob(const TCHAR* szFileToProtect);
	bool ExecuteJob(const TCHAR* szFileName);

	bool ProtectCurDocument(bool promptToSave = true);
	bool ProtectCurDocumentWithXrefs(bool promptToSave = true);

private:
	std::map<std::wstring, ProtectJobPtr, ICaseKeyLess> m_jobPool;
};

DEFINE_RMXCONTROLLER_GET(CAcRXMProtectControl, ProtectController)
