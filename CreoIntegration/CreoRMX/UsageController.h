#pragma once

#include <map>
#include <set>
#include <string>
#include <windows.h>

#include "..\common\CommonTypes.h"
#include <RMXTypes.h>

#include <pfcCommand.h>
#include <pfcModel.h>
#include <pfcSession.h>
#include <xstring.h>

class COtkXModel;

typedef bool (*pfCheckRightCB) (DWORD rights, RMXActivityLogOperation logOp);

struct UCCmdInfo {
	std::string cmdName;
	DWORD dwAccessRights;
	DWORD dwProtectWatchers;
	RMXActivityLogOperation eRMSActivityLogOp;
	pfCheckRightCB checkRightCB;
	UCCmdInfo() {}
	UCCmdInfo(const std::string& name, DWORD rights, RMXActivityLogOperation logOp, DWORD watchers, pfCheckRightCB cb)
	{
		cmdName = name;
		dwAccessRights = rights;
		dwProtectWatchers = watchers;
		eRMSActivityLogOp = logOp;
		checkRightCB = cb;
	}
};

enum UsageRightStatus
{
	Usage_Right_Grant,
	Usage_Right_Deny,
	Usage_Right_DenyEdit,
	Usage_Right_Deny_FileNotExists
};

//*****************************************************************************
class CUCCommandBracketListener : public virtual pfcUICommandBracketListener
{
public:
	CUCCommandBracketListener(const UCCmdInfo& cmd);
	void OnBeforeCommand();
	void OnAfterCommand();

private:
	UCCmdInfo m_cmdInfo;
};

class CUsageController : public RMXControllerBase<CUsageController>
{
public:
	void Start();
	void Stop();

	bool IsIPEMCmdApplied() const { return m_iPEMCmdApplied;}
	bool IsPLMJTCmdApplied() const { return m_PLMJTCmdApplied; }

	void ApplyToIPEM();
	void ApplyToPLMJT();

	static UsageRightStatus CheckRight(const COtkXModel& xMdl, DWORD right, bool alwaysCheckEdit = false);
	static UsageRightStatus CheckRight(const wstring& filePath, DWORD right, bool alwaysCheckEdit = false);
	static UsageRightStatus CheckDepRight(const COtkXModel& xMdl, DWORD right, std::set<std::wstring>& denyFiles, bool alwaysCheckEdit = false);

private:
	bool ApplyToCommand(pfcSession_ptr pSess, const UCCmdInfo& cmd);
	
private:
	std::map<std::string, pfcActionListener_ptr> m_cmdListeners;
	bool m_iPEMCmdApplied;
	bool m_PLMJTCmdApplied;
	bool m_iPEMRMXRegistered;
	pfcDll_ptr m_pIpemRMXDll;
};

DEFINE_RMXCONTROLLER_GET(UsageController)