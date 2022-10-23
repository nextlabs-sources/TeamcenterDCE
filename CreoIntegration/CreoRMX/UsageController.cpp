#include "UsageController.h"

#include <pfcGlobal.h>
#include <wfcSession.h>
#include <xstring.h>

#include "..\common\CreoRMXLogger.h"
#include "OtkXModel.h"
#include "OtkXTypes.h"
#include "OtkXUIStrings.h"
#include "OtkXUtils.h"
#include <PathEx.h>
#include <RMXUtils.h>

#include "IpemRMX.h"
#include <LibRMX.h>
#include "ProtectController.h"
#include "RPMDirectoryMgr.h"
#include "OtkXSessionCache.h"

using namespace std;
using namespace OtkXUtils;

CUCCommandBracketListener::CUCCommandBracketListener(const UCCmdInfo& cmd)
{
	m_cmdInfo = cmd;
}

void CUCCommandBracketListener::OnBeforeCommand()
{
	UC_LOG_ENTER(RMXUtils::s2ws(m_cmdInfo.cmdName).c_str());

	if (m_cmdInfo.checkRightCB(m_cmdInfo.dwAccessRights, m_cmdInfo.eRMSActivityLogOp))
	{
		// Let protect controller track the file creation(hook) during the action process
		// and protect the exported files or newly created auxiliary files for saving.
		if (m_cmdInfo.dwProtectWatchers != 0)
			CProtectController::GetInstance().StartFileWatcher(m_cmdInfo.dwProtectWatchers);	
	}
	else
	{
		pfcXCancelProEAction::Throw();
	}
}

void CUCCommandBracketListener::OnAfterCommand()
{
	UC_LOG_ENTER(RMXUtils::s2ws(m_cmdInfo.cmdName).c_str());

	if (m_cmdInfo.dwProtectWatchers != 0)
		CProtectController::GetInstance().StopFileWatcher(m_cmdInfo.dwProtectWatchers);

	// Ensure the current directory is up-to-date
	// Sometimes, the export process will temporarily change the current directory but
	// OnAfterDirectoryChange listener is not triggered. 
	CRPMDirectoryMgr::GetInstance().SyncCurrentDirectory();

	CProtectController::GetInstance().ShowEditDenyDialog();
}

////
//// Loop all existing models in the session and check if all of them have certain permission
////
//bool CheckRightOnAllModels(DWORD dwAccessRights, RMXActivityLogOperation logOp)
//{
//	try
//	{
//		auto pSess = pfcGetProESession();
//		if (pSess)
//		{
//			auto pMdls = pSess->ListModels();
//			if (pMdls)
//			{
//				set<wstring> denyFiles;
//				for (xint i = 0; i < pMdls->getarraysize(); ++i)
//				{
//					COtkXModel xModel(pMdls->get(i));
//					if (!xModel.CheckRights(dwAccessRights))
//					{
//						denyFiles.insert(xModel.GetOrigin());
//					}
//					set<wstring> denyDepFiles;
//					if (!xModel.CheckDepRights(dwAccessRights, denyDepFiles))
//					{
//						denyFiles.insert(denyDepFiles.begin(), denyDepFiles.end());
//					}
//				}
//				if (denyFiles.size() > 0)
//				{
//					wchar_t szRight[_MAX_FNAME];
//					RMX_GetRightName(dwAccessRights, szRight);
//					LOG_ERROR_FMT(L"Operation Denied. You do not have '%s' permission to perform this action on the following file(s):", szRight);
//					xstring msg = xstring::Printf("Operation Denied.\nYou do not have '%' permission to perform this action the following file(s):", RMXUtils::ws2s(szRight).c_str());
//					xstring fileInfo;
//					for (const auto& file : denyFiles)
//					{
//						fileInfo = xstring::Printf("\n%s", RMXUtils::ws2s(file).c_str());
//						msg += fileInfo;
//						LOG_ERROR_FMT(L"\t%s", file.c_str());
//					}
//
//					OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
//					return false;
//				}
//			}
//		}
//	}
//	OTKX_EXCEPTION_HANDLER();
//	return true;
//}

bool CheckRightOnCurrentModel(DWORD dwAccessRights, RMXActivityLogOperation logOp)
{
	pfcModel_ptr pMdl = OtkX_GetCurrentModel();
	if (pMdl)
	{
		COtkXModel xModel(pMdl);
		UsageRightStatus rightStatus = CUsageController::CheckRight(xModel, dwAccessRights);
		if (rightStatus == Usage_Right_Deny_FileNotExists)
		{
			xstring msg(IDS_ERR_DENY_OP_LOSEFILE);
			LOG_ERROR(RMXUtils::s2ws(msg).c_str());
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
			return false;
		}
		if (rightStatus == Usage_Right_Deny)
		{
			wchar_t szRight[_MAX_FNAME];
			RMX_GetRightName(dwAccessRights, szRight);
			xstring msg = xstring::Printf(IDS_ERR_DENY_OP, RMXUtils::ws2s(szRight).c_str());
			LOG_ERROR(RMXUtils::s2ws(msg).c_str());
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
			return false;
		}
		// In case the file is modified but does not have edit permission, don't allow to do other op
		// such as save as/export/mirror based on modified models.
		if (rightStatus == Usage_Right_DenyEdit)
		{
			xstring msg = xstring::Printf(IDS_ERR_DENY_EDIT_CURRENT_MODEL);
			LOG_ERROR(RMXUtils::s2ws(msg).c_str());
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
			return false;
		}
		// Scan assembly and deny operation if any dependent model does not have permission.
		// Except "EDIT" permission. If some of dependent model has no edit permission, still allow operation on its
		// parent assembly but any modification on read-only model will be discard automatically.

		std::set<std::wstring> denyFiles;
		xstring msg;
		auto buildErrorAndPrompt = [&]() -> void {
			xstring fileInfo;
			for (const auto& file : denyFiles)
			{
				fileInfo = xstring::Printf("\n- %s", RMXUtils::ws2s(file).c_str());
				msg += fileInfo;
			}
			LOG_ERROR(RMXUtils::s2ws(msg).c_str());
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
		};

		if (dwAccessRights == RMX_RIGHT_EDIT)
		{
			xModel.TraverseDependencies([&](const COtkXModel& xDepMdl) -> bool {
				if (xDepMdl.IsValid() && !xDepMdl.FileExists() &&
					(OtkXSessionCacheHolder().IsNxlModel(xDepMdl)))
				{
					// In case the file was deleted from disdk but the model
					// is still in session. Deny any operation becuase it's not 
					// able to evaluate the permission properly
					denyFiles.insert(xDepMdl.GetOrigin());
					rightStatus = Usage_Right_Deny_FileNotExists;
				}

				return false;
			});
		}
		else
		{
			rightStatus = CUsageController::CheckDepRight(xModel, dwAccessRights, denyFiles);
		}

		if (rightStatus == Usage_Right_Deny_FileNotExists)
		{
			msg = IDS_ERR_DENY_OP_DEP_LOSEFILE;
			buildErrorAndPrompt();
			return false;
		}
		if (rightStatus == Usage_Right_DenyEdit)
		{
			msg = IDS_ERR_DENY_EDIT_DEP;
			buildErrorAndPrompt();
			return false;
		}

		if (rightStatus == Usage_Right_Deny)
		{
			wchar_t szRight[_MAX_FNAME];
			RMX_GetRightName(dwAccessRights, szRight);
			msg = xstring::Printf(IDS_ERR_DENY_OP_DEP, RMXUtils::ws2s(szRight).c_str());
			buildErrorAndPrompt();
			return false;
		}
	}
	else {
		auto pSess = wfcWSession::cast(pfcGetProESession());
		if (pSess == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return true;
		}
		auto pWin = pSess->GetCurrentWindow();
		if (pWin != NULL) {
			pMdl = pWin->GetModel();
			if (OtkXSessionCacheHolder().IsNxlModel(pMdl))
			{
				xstring msg(IDS_ERR_DENY_NO_CURRMODEL);
				LOG_ERROR(RMXUtils::s2ws(msg).c_str());
				OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
				return false;
			}
		}
	}
	return true;
}

bool CheckRightOnEmailAttach(DWORD dwAccessRights, RMXActivityLogOperation logOp)
{
	pfcModel_ptr pCurrMdl = OtkX_GetCurrentModel();
	COtkXModel xModel(pCurrMdl);
	if (xModel.IsProtected() || xModel.IsDepProtected() ||
		OtkXSessionCacheHolder().IsNxlModel(xModel) || OtkXSessionCacheHolder().HasNxlDepModel(xModel))
	{
		LOG_ERROR_STR(L"Operation Denied. You are not allowed to send the protected object as attachment(s)");
		RMX_AddActivityLog(xModel.GetOrigin().c_str(), RMX_OShare, RMX_RDenied);
		xstring msg = xstring::Printf(IDS_ERR_DENY_EMAILATTACH);
		OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
		return false;
	}

	return true;
}

bool CheckRightOnAlwaysAllow(DWORD dwAccessRights, RMXActivityLogOperation logOp)
{
	return true;
}

bool CheckRightOnIpem(DWORD dwAccessRights, RMXActivityLogOperation logOp)
{
	return true;
}

bool CheckRightOnDrwPreview(DWORD /*dwAccessRights*/, RMXActivityLogOperation /*logOp*/)
{
	pfcModel_ptr pMdl = OtkX_GetCurrentModel();
	if (pMdl)
	{
		COtkXModel xModel(pMdl);
		if (xModel.IsProtected() || xModel.IsDepProtected())
		{
			xstring msg(IDS_ERR_DENY_DRWPREVIEW);
			LOG_ERROR(RMXUtils::s2ws(msg).c_str());
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
			return false;
		}
	}

	return true;
}

static UCCmdInfo g_defaultCmdList[] =
{
	// Commands require EDIT right
	UCCmdInfo(OTKX_COMMAND_SAVE, RMX_RIGHT_EDIT, RMX_OEdit, 0, &CheckRightOnCurrentModel),

	// Commands require SAVEAS right
	// Note: Export and COPY action trigger same command ProCmdModelSaveAs.
	// Cannot distinguish them at this moment until copy action listener is triggered. Start both export and copy watchers.
	UCCmdInfo(OTKX_COMMAND_SAVEAS, RMX_RIGHT_COPY, RMX_ODownload, PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_COPY, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_BACKUP, RMX_RIGHT_COPY, RMX_ODownload, PROTECT_WATCHER_BACKUP, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_MIRRORPART, RMX_RIGHT_DECRYPT, RMX_OCopyContent, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_MIRRORASM, RMX_RIGHT_DECRYPT, RMX_OCopyContent, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_SAVEASCSA, RMX_RIGHT_COPY, RMX_ODownload, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_SAVEASMODULE, RMX_RIGHT_COPY, RMX_ODownload, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_APPMESH, RMX_RIGHT_Nil, RMX_ODownload, PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE, &CheckRightOnAlwaysAllow),
	UCCmdInfo("ProCmdFileInstAccelDlg", RMX_RIGHT_COPY, RMX_ODownload, 0, &CheckRightOnCurrentModel),
	
	// Drawing export commands - SAVEAS right
	UCCmdInfo(OTKX_COMMAND_QUICKEXPORT, RMX_RIGHT_EXPORT, RMX_ODownload, PROTECT_WATCHER_EXPORT, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_EXPORTPREVIEW, RMX_RIGHT_EXPORT, RMX_ODownload, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_PUBLISHEXPORT, RMX_RIGHT_EXPORT, RMX_ODownload, PROTECT_WATCHER_EXPORT, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_3DPRINTSAVEAS, RMX_RIGHT_EXPORT, RMX_ODownload, PROTECT_WATCHER_EXPORT, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_DWGPUBPREVIEW, RMX_RIGHT_EXPORT, RMX_ODownload, PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_EXPORT_DWGPUBPREVIEW, &CheckRightOnDrwPreview),

	// Commands require PRINT right 
	UCCmdInfo(OTKX_COMMAND_PRINT, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_PRINT3D, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_PRINT2D, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_QUICKPRINT, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_QUICKDRAWING, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_3DPRINT, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_3DPRINTFREEMIUM, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
#if defined(CREO_VER) && CREO_VER >= 4
	UCCmdInfo (OTKX_COMMAND_3DPRINTORDER, RMX_RIGHT_PRINT, RMX_OPrint, 0, &CheckRightOnCurrentModel),
#endif

	// Disable the feature with RMX_RIGHT_NONE
	UCCmdInfo(OTKX_COMMAND_EMAILATTACH, RMX_RIGHT_NONE, RMX_OShare, 0, &CheckRightOnEmailAttach),
	UCCmdInfo("ProCmdSendAsAttach", RMX_RIGHT_Nil, RMX_OShare, PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_SENDMAIL, &CheckRightOnAlwaysAllow)
};

static UCCmdInfo g_iPEMCmdList[] =
{
	UCCmdInfo(OTKX_COMMAND_TC_SAVE, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_OView, PROTECT_WATCHER_SAVE | PROTECT_WATCHER_SAVE_TC | PROTECT_WATCHER_TC, &CheckRightOnIpem),
	UCCmdInfo(OTKX_COMMAND_TC_SAVEAS, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_OView, PROTECT_WATCHER_SAVE | PROTECT_WATCHER_SAVE_TC | PROTECT_WATCHER_TC,  &CheckRightOnIpem),
	UCCmdInfo(OTKX_COMMAND_TC_SAVEALL, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_OView, PROTECT_WATCHER_SAVE | PROTECT_WATCHER_SAVE_TC | PROTECT_WATCHER_TC,  &CheckRightOnIpem),
	UCCmdInfo(OTKX_COMMAND_TC_CACHEMANAGER, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_OView, PROTECT_WATCHER_TC | PROTECT_WATCHER_TC_CACHEMANAGER, &CheckRightOnIpem),
	UCCmdInfo(OTKX_COMMAND_TC_OPEN, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_OView, PROTECT_WATCHER_TC, &CheckRightOnIpem),
	UCCmdInfo(OTKX_COMMAND_TC_CANCELCO, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_OView, PROTECT_WATCHER_TC, &CheckRightOnIpem)
};

// When the command is triggered, the target model to export 
// may have not been specified, cannot determine the permission- RMX_RIGHT_Nil 
static UCCmdInfo g_PLMJTCmdList[] = {
	UCCmdInfo(OTKX_COMMAND_JTEXPORTCURRENT, RMX_RIGHT_EXPORT, RMX_ODownload, PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_EXPORT_JTCURRENT, &CheckRightOnCurrentModel),
	UCCmdInfo(OTKX_COMMAND_JTUSEBATCH, RMX_RIGHT_Nil/*Always allow to trigger*/, RMX_ODownload, PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_EXPORT_JTBATCH, &CheckRightOnAlwaysAllow),
};

void CUsageController::Start()
{
	LOG_INFO(L"Applying usage control...");
	
	m_iPEMCmdApplied = false;
	m_iPEMRMXRegistered = false;
	m_PLMJTCmdApplied = false;

#if defined(CREO_VER) && CREO_VER == 4
	if (GetModuleHandle(L"JtTk103.dll") != NULL)
	{
		g_PLMJTCmdList[0].dwAccessRights = RMX_RIGHT_Nil;
		g_PLMJTCmdList[0].checkRightCB = &CheckRightOnAlwaysAllow;
	}
#endif
	try
	{
		auto pSession = wfcWSession::cast(pfcGetProESession());
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}
		
		// Monitor and deny operation in case of the file has no right.
		for (const auto &cmd : g_defaultCmdList)
		{
			ApplyToCommand(pSession, cmd);
		}	

		ApplyToIPEM();
		ApplyToPLMJT();

	
		auto cmdLines = RMXUtils::GetCommandLines();
		for (auto& cmdLine : cmdLines)
		{
			// HACK solution for IPEM -> Save JT Files
			// A background Creo process will launch with arguments specified like "eai_batch:C:\Users\tcadmin\AppData\Local\Temp\proetojtXX.txt"
			// Need to notify protection controller to monitor the exported JT files
			if (wcsstr(cmdLine.c_str(), L"eai_batch") != NULL)
			{
				LOG_INFO(L"PLM JT export in batch mode: " << cmdLine.c_str());
				IpemRMX::SetEAIBatchModel();
				CProtectController::GetInstance().StartFileWatcher(PROTECT_WATCHER_EXPORT | PROTECT_WATCHER_EXPORT_JTBATCH | PROTECT_WATCHER_TC);
				break;
			}
		}
	}
	OTKX_EXCEPTION_HANDLER();
}

void CUsageController::Stop()
{
	if (m_cmdListeners.empty())
		return;

	LOG_INFO(L"Stopping usage control...");
	// Remove listener from command
	try
	{
		pfcSession_ptr pSession = pfcGetProESession();
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}
		for (const auto& lis : m_cmdListeners)
		{
			pfcUICommand_ptr pCmd = pSession->UIGetCommand(CA2XS(lis.first.c_str()));
			if (pCmd)
			{
				pCmd->RemoveActionListener(lis.second);
			}
		}
		m_cmdListeners.clear();
		IpemRMX::Unregister();
	}
	OTKX_EXCEPTION_HANDLER();

}

void CUsageController::ApplyToIPEM()
{
	if (!m_iPEMRMXRegistered) 
		m_iPEMRMXRegistered = IpemRMX::Register();
	
	if (m_iPEMCmdApplied)
		return;

	LOG_INFO_STR(L"Apply usage control to iPEM...");
	try
	{
		pfcSession_ptr pSess = pfcGetProESession();
		if (pSess == nullptr)
			return;

		bool hasFailure = false;
		for (const auto& cmd : g_iPEMCmdList)
		{
			if (!ApplyToCommand(pSess, cmd))
				hasFailure = true;
		}

		if (!hasFailure)
		{
			m_iPEMCmdApplied = true;			
		}
	}
	OTKX_EXCEPTION_HANDLER();
}
	
void CUsageController::ApplyToPLMJT()
{
	if (m_PLMJTCmdApplied)
		return;

	LOG_INFO_STR(L"Apply usage control to PLM JT Translator...");
	try
	{
		pfcSession_ptr pSess = pfcGetProESession();
		if (pSess == nullptr)
			return;

		bool hasFailure = false;
		for (const auto& cmd : g_PLMJTCmdList)
		{
			if (!ApplyToCommand(pSess, cmd))
				hasFailure = true;
		}

		if (!hasFailure)
			m_PLMJTCmdApplied = true;
	}
	OTKX_EXCEPTION_HANDLER();
}

UsageRightStatus CUsageController::CheckRight(const COtkXModel & xMdl, DWORD right, bool alwaysCheckEdit /*= false*/)
{
	if (!xMdl.IsValid())
		return Usage_Right_Grant;

	if (!xMdl.FileExists())
	{
		// In case the file was deleted from disdk but the model
		// is still in session. Deny any operation becuase it's not 
		// able to evaluate the permission properly
		if (OtkXSessionCacheHolder().IsNxlModel(xMdl))
			return Usage_Right_Deny_FileNotExists;
		return Usage_Right_Grant;
	}
	if (!xMdl.CheckRights(right))
		return Usage_Right_Deny;

	// Special case to ensure file is not allowed to modify without edit permission
	if (right != RMX_RIGHT_EDIT)
	{
		if (alwaysCheckEdit || xMdl.GetModel()->GetIsModified())
		{
			if (!xMdl.CheckRights(RMX_RIGHT_EDIT))
			{
				return Usage_Right_DenyEdit;
			}
		}
	}

	return Usage_Right_Grant;
}

UsageRightStatus CUsageController::CheckRight(const wstring & filePath, DWORD right, bool alwaysCheckEdit)
{
	if (filePath.empty())
		return Usage_Right_Grant;

	if (!CPathEx::IsFile(filePath))
	{
		// In case the file was deleted from disdk but the model
		// is still in session. Deny any operation becuase it's not 
		// able to evaluate the permission properly
		if (OtkXSessionCacheHolder().IsNxlFile(filePath))
			return Usage_Right_Deny_FileNotExists;
		return Usage_Right_Grant;
	}

	if (!RMX_IsProtectedFile(filePath.c_str()))
		return Usage_Right_Grant;

	if (!RMX_CheckRights(filePath.c_str(), right, true))
		return Usage_Right_Deny;

	// Special case to ensure file is not allowed to modify without edit permission
	if (right != RMX_RIGHT_EDIT && alwaysCheckEdit)
	{
		if (!RMX_CheckRights(filePath.c_str(), RMX_RIGHT_EDIT, true))
			return Usage_Right_DenyEdit;
	}

	return Usage_Right_Grant;
}

UsageRightStatus CUsageController::CheckDepRight(const COtkXModel & xMdl, DWORD right, std::set<std::wstring>& denyFiles, bool alwaysCheckEdit /*= false*/)
{
	std::set<std::wstring> denyEditFiles;
	std::set<std::wstring> denyLoseFiles;
	xMdl.TraverseDependencies([&](const COtkXModel& xDepMdl) -> bool {
		auto rightStatus = CheckRight(xDepMdl, right, alwaysCheckEdit);
		if (rightStatus == Usage_Right_DenyEdit)
			denyEditFiles.insert(xDepMdl.GetOrigin());
		else if (rightStatus == Usage_Right_Deny)
			denyFiles.insert(xDepMdl.GetOrigin());
		else if (rightStatus == Usage_Right_Deny_FileNotExists)
			denyLoseFiles.insert(xDepMdl.GetOrigin());
		return false;
	});
	// The priority 
	// 1. nxl model but file not found
	// 2. edit deny
	// 3. right deny
	if (!denyLoseFiles.empty())
	{
		denyFiles = denyLoseFiles;
		return Usage_Right_Deny_FileNotExists;
	}
	if (!denyEditFiles.empty())
	{
		denyFiles = denyEditFiles;
		return Usage_Right_DenyEdit;
	}

	return denyFiles.empty() ? Usage_Right_Grant : Usage_Right_Deny;
}

bool CUsageController::ApplyToCommand(pfcSession_ptr pSess, const UCCmdInfo& cmd)
{
	// Ignore in case the command is registered already.
	if (m_cmdListeners.find(cmd.cmdName) != m_cmdListeners.end())
		return true;

	bool succeeded = false;
	try
	{
		pfcUICommand_ptr pCmd = pSess->UIGetCommand(CA2XS(cmd.cmdName.c_str()));
		if (pCmd)
		{
			CUCCommandBracketListener* pListener = new CUCCommandBracketListener(cmd);
			pCmd->AddActionListener(pListener);
			m_cmdListeners[cmd.cmdName] = pListener;
			LOG_INFO(L"\t-Usage control applied: " << RMXUtils::s2ws(cmd.cmdName).c_str());
			succeeded = true;
		}
		else
		{
			LOG_ERROR_FMT(L"\t-Cannot apply usage control: '%s' (error: Command not found)", RMXUtils::s2ws(cmd.cmdName).c_str());
		}
	}
	OTKX_EXCEPTION_HANDLER();

	return succeeded;
}