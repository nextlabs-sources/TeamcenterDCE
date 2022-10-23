#pragma once

#include "CatEventMgr.h"

struct UCCmd;
typedef bool (*pfCheckRightCB) (const UCCmd& ucCmd);

enum eUCCmdContext
{
	eContext_Current,
	eContext_File,
	eContext_Files
};

struct UCCmd {
	wstring wsCmdName;
	DWORD dwAccessRight;
	bool bCheckReferences;
	bool bAlert;
	pfCheckRightCB pCB;
	eUCCmdContext eCtx;
	wstring wsCtxFileFullName; // Specify the file as target to check. 
	std::set<wstring, ICaseKeyLess> ctxFiles; // Specify multiple files as target to check

	UCCmd()
	{
		bAlert = true;
		pCB = NULL;
	}
	UCCmd(const wstring& ws, DWORD dw, bool b, pfCheckRightCB p, eUCCmdContext ctx = eContext_Current)
		: eCtx(ctx), wsCmdName(ws), dwAccessRight(dw), bCheckReferences(b), pCB(p), bAlert(true)
	{
		
	}
};

class CRMXUsageControl : public RMXControllerBase<CRMXUsageControl>, public CCatEventLisener
{
public:
	void Start();
	void Stop();

	BOOL CheckCommandRight(const wstring& wsCmdName);
	BOOL CheckCommandRightByFile(const wstring& wsCmdName, const wstring& wsFileFullName);
	BOOL CheckCommandRightByFiles(const wstring& wsCmdName, const std::set<wstring, ICaseKeyLess>& files);

	// Event handler
	void OnBeforeCommand(LPCatCommandNotify pParam);
	//void OnAfterCommand(LPCatCommandNotify pParam);

	template<typename CheckOneDocCallback>
	CatRMX::RightStatus CheckRight(bool bCheckRefs, CheckOneDocCallback& callback)
	{
		auto pCatApp = curCatSession.GetCatApp();
		if (!pCatApp)
		{
			LOG_ERROR(L"Catia session not connected yet. Please restart and try again");
			return CatRMX::eRightDeny;
		}

		HRESULT hr = S_OK;
		CAT::DocumentPtr pDoc;
		hr = pCatApp->get_ActiveDocument(&pDoc);
		if (FAILED(hr) || pDoc == NULL)
		{
			LOG_DEBUG(L"Skip. Active document not found");
			return CatRMX::eRightGrant; // It might happen when no any file is created or opened yet.
		}

		if (!callback(pDoc, false))
			return CatRMX::eRightDeny;

		if (bCheckRefs)
		{
			if (curCatSession.TraverseLinks(pDoc, [&](CAT::DocumentPtr pLinkDoc) -> bool {
				if (!callback(pLinkDoc, true))
				{
					return true; // Terminate loop
				}

				return false;
			}))
			{
				return CatRMX::eRightDenyXref;
			}
		}

		return CatRMX::eRightGrant;
	}
	
private:
	map<wstring, UCCmd, ICaseKeyLess> m_cmdMap;
};

#define curUCControl CRMXUsageControl::GetInstance()

