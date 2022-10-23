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

class CAcRMXRPMDirMgr : public RMXControllerBase<CAcRMXRPMDirMgr>
{
public:
	void Start();
	void Stop();

	void AddRPMDir(const std::wstring& strDir);
	void UpdateWorkingDir();

private:
	std::wstring GetNavDlgInitialFolder() const;

private:
	std::set<std::wstring, ICaseKeyLess> m_addedRPMDirs;
};

DEFINE_RMXCONTROLLER_GET(CAcRMXRPMDirMgr, RPMDirMgr)

