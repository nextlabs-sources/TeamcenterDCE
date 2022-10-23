#include "RMXCommandMgr.h"
#include "..\common\CreoRMXLogger.h"
#include "OtkXUtils.h"
#include "OtkXUIStrings.h"

#include <pfcCommand.h>
#include <pfcGlobal.h>
#include <wfcClient.h>
#include <LibRMX.h>
#include "OtkXModel.h"
#include "ProtectJob.h"
#include "ProtectController.h"
#include "RPMDirectoryMgr.h"
#include <PathEx.h>
#include "OtkXSessionCache.h"
#include "UsageController.h"

using namespace OtkXUtils;

extern "C" wfcStatus OnProtect();
extern "C" wfcStatus OnProtectWithDeps();
extern "C" wfcStatus OnViewInfo();
extern "C" wfcStatus OnAbout();
extern "C" pfcCommandAccess OnAccessProtect();
extern "C" pfcCommandAccess OnAccessViewInfo();
extern "C" pfcCommandAccess OnAccessProtectWithDeps();

extern "C" {
	typedef wfcStatus(*pfRMXCommandCallback) ();
	typedef pfcCommandAccess(*pfRMXComandAccessCallback)();
}

//*****************************************************************************
class CRMXCommandAccessListener : public virtual pfcUICommandAccessListener
{
public:
	CRMXCommandAccessListener(pfRMXComandAccessCallback func) {
		m_func = func;
	}

	virtual pfcCommandAccess OnCommandAccess(xbool AllowErrorMessages) {
		return m_func();
	}
private:
	pfRMXComandAccessCallback m_func;
};

class CRMXCommandListener : public virtual pfcUICommandActionListener
{
public:
	CRMXCommandListener(pfRMXCommandCallback func) { m_func = func; }

    virtual void OnCommand() {
        m_func();
    }
private:
    pfRMXCommandCallback m_func;
};

void CRMXCommandMgr::Start()
{
	LOG_INFO(L"Starting commands manager...");
	try
	{
		auto pSess = pfcGetProESession();
		try
		{
			/* Loading the ribbon */
			pSess->RibbonDefinitionfileLoad("creormx.rbn");
			LOG_INFO(L"Ribbon definition loaded");
		}
		OTKX_EXCEPTION_HANDLER();

		// Function to create a custom command in Creo
		auto createCommandFunc = [&](xrstring cmdName, xrstring cmdLabel, xrstring cmdHelp, xrstring icon, pfRMXCommandCallback cmdCB, pfRMXComandAccessCallback cmdAcessCB) -> pfcUICommand_ptr {
			pfcUICommand_ptr pCmd = pSess->UICreateCommand(cmdName, new CRMXCommandListener(cmdCB));
			if (pCmd)
			{
				pCmd->Designate(OTKX_MSGFILE, cmdLabel, cmdHelp, cmdHelp);
				pCmd->SetIcon(icon);
				if (cmdAcessCB != nullptr)
					pCmd->AddActionListener(new CRMXCommandAccessListener(cmdAcessCB));

				return pCmd;
			}

			return nullptr;
		};

		// Create RMX specific commands used by menu entries
		createCommandFunc("RMXProtect", "Protect", "Protect current file", "protect_large.png",
			&OnProtect,
			&OnAccessProtect);

		createCommandFunc("RMXProtectWithDeps", "Protect With Dependencies", "Protect current file and its dependencies", "protect_deps_large.png",
			&OnProtectWithDeps,
			&OnAccessProtectWithDeps);

		createCommandFunc("RMXViewInfo", "File Info", "View infomation on protected file", "fileinfo_large.png",
			&OnViewInfo,
			&OnAccessViewInfo);
		
		createCommandFunc("RMXAbout", "About", "About NextLabs RMX for Creo", "about_large.png",
			&OnAbout,
			nullptr);
	}
	OTKX_EXCEPTION_HANDLER();
}

void CRMXCommandMgr::Stop()
{
	LOG_INFO(L"Stopping commands manager...");
}

extern "C" pfcCommandAccess OnAccessProtect()
{
	// Don't support in managed mode
	if (UsageControllerHolder().IsIPEMCmdApplied())
		return pfcACCESS_UNAVAILABLE;
	try
	{
		LOG_TRACE(L"Checking the access permission on RMXProtect command...");
		auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
		if (pCurrMdl)
		{
			COtkXModel xMdl(pCurrMdl);
			if (!xMdl.IsValid() || !OtkXSessionCacheHolder().IsNxlModel(xMdl))
			{
				return pfcACCESS_AVAILABLE;
			}
		}
	}
	OTKX_EXCEPTION_HANDLER();

	return pfcACCESS_UNAVAILABLE;
}

void _CloseAllModels()
{
	try
	{
		auto pSess = pfcGetProESession();
		if (pSess)
		{
			auto pMdls = pSess->ListModels();
			for (xint i = 0; i < pMdls->getarraysize(); ++i)
			{
				auto pMdl = pMdls->get(i);
				if (pMdl)
					pMdl->EraseWithDependencies();
			}

			pSess->EraseUndisplayedModels();
		}
	}
	OTKX_EXCEPTION_HANDLER();
}

extern "C" wfcStatus OnProtect()
{
	try
	{
		LOG_DEBUG(L"Executing RMXProtect command...");
		ProtectControllerHolder().ProtectCurrentCommand();
	}
	OTKX_EXCEPTION_HANDLER(); 

	return wfcTK_NO_ERROR;
}

extern "C" pfcCommandAccess OnAccessProtectWithDeps()
{
	// Don't support in managed mode
	if (UsageControllerHolder().IsIPEMCmdApplied())
		return pfcACCESS_UNAVAILABLE;
	try
	{
		LOG_TRACE(L"Checking the access permission on RMXProtectWithDeps command...");
		auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
		if (pCurrMdl)
		{
			COtkXModel xMdl(pCurrMdl);
			if (!xMdl.IsValid() || !OtkXSessionCacheHolder().IsNxlModel(xMdl))
			{
				return pfcACCESS_AVAILABLE;
			}

			bool hasUnprotectedDep = false;
			xMdl.TraverseDependencies([&](COtkXModel& xDepMdl) -> bool {
				if (!xDepMdl.IsValid() || !OtkXSessionCacheHolder().IsNxlModel(xDepMdl))
				{
					hasUnprotectedDep = true;
					return true;
				}

				return false;
				});

			if (hasUnprotectedDep)
				return pfcACCESS_AVAILABLE;
		}
	}
	OTKX_EXCEPTION_HANDLER();

	return pfcACCESS_UNAVAILABLE;
}

extern "C" wfcStatus OnProtectWithDeps()
{
	try
	{
		LOG_DEBUG(L"Executing RMXProtectWithDeps command...");

		ProtectControllerHolder().ProtecCurrentWithDepsCommand();
	}
	OTKX_EXCEPTION_HANDLER();

	return wfcTK_NO_ERROR;
}

extern "C" pfcCommandAccess OnAccessViewInfo()
{
	try
	{
		LOG_TRACE(L"Checking the access permission on ViewInfo command...");
		auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
		if (pCurrMdl)
		{
			COtkXModel xMdl(pCurrMdl);
			if (OtkXSessionCacheHolder().IsNxlModel(xMdl))
			{
				return pfcACCESS_AVAILABLE;
			}
		}
	}
	OTKX_EXCEPTION_HANDLER();

	return pfcACCESS_UNAVAILABLE;
}

extern "C" wfcStatus OnViewInfo()
{
	try
	{
		LOG_DEBUG(L"Executing RMXViewInfo command...");
		auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
		if (pCurrMdl)
		{
			COtkXModel xMdl(pCurrMdl);
			if (OtkXSessionCacheHolder().IsNxlModel(xMdl))
			{
				RMX_ShowFileInfoDialog(xMdl.GetOrigin().c_str(), nullptr, _W(OTKX_MESSAGEDLG_LABEL));
			}
		}	
	}
	OTKX_EXCEPTION_HANDLER();

	return wfcTK_NO_ERROR;
}

extern "C" wfcStatus OnAbout()
{
	try
	{
		LOG_INFO(L"Executing RMXAbout command...");
		std::string msg = RMXUtils::FormatString(IDS_ABOUT_TEXT, VER_FILEVERSION_STR, VER_BUILD_STR, VER_COPYRIGHT_Y);
		OtkX_ShowMessageDialog(msg.c_str(), pfcMESSAGE_INFO);
	}
	OTKX_EXCEPTION_HANDLER();

	return wfcTK_NO_ERROR;
}