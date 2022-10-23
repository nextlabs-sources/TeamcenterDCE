//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Aug 2021
//////////////////////////////////////////////////////////////////////////////
//

#pragma once
#include "CommonTypes.h"
#include "AcRMXTypes.h"

struct UCCmdEntry {
	CString strCmdName;
	DWORD dwAccessRight;
	bool bCheckReferences;
	AcApDocument* pCtxDoc; // Specify context target by either pCtxDoc or strCtxDoc
	CString strCtxDoc;
	bool bAlert;
	bool (*pCB) (const UCCmdEntry& ucCmd);
};

class CAcRMXAccessControl : public RMXControllerBase<CAcRMXAccessControl>
{
public:
	
public:
	void Start();
	void Stop();

	bool CheckCommandRight(const CString& strCmdName, AcApDocument* pDoc);
	bool CheckCommandRight(const CString& strCmdName, const CString& strFilePath);

	CString GetRecentVetoedCommand() const { return m_csRecentVetoedCmd; }
	bool IsUCCmd(const CString& strCmdName) const { return (m_cmdMap.find(strCmdName) != m_cmdMap.end()); }
	bool IsImportCmd(const CString& strCmdName) const {
		return (m_importCmds.find(strCmdName) != m_importCmds.end());
	}
	void EnsureCommandName(CString& strCmdName) const;

public:
	static AcRMX::AccessRightStatus CheckRight(const CString& strFilePath, DWORD dwRight, bool bAlert = true);
	static AcRMX::AccessRightStatus CheckRight(AcApDocument* pDoc, DWORD dwRight, bool bCheckXrefs, bool bAlert = true);
	
private:
	std::map<CString, UCCmdEntry> m_cmdMap;
	std::set<CString> m_importCmds;
	CString m_csRecentVetoedCmd;
};

DEFINE_RMXCONTROLLER_GET(CAcRMXAccessControl, AccessController)

