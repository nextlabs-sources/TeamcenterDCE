#include "StdAfx.h"
#include "AcRMXAccessControl.h"
#include "AcRMXSessionCacheMgr.h"
#include <LibRMX.h>

#include "AcRMXUtils.h"
#include "AcInc.h"
#include "resource.h"

#define ADD_CMD_TO_CONTROL_DEFAULT(cmdName, right, checkRefs) \
ADD_CMD_TO_CONTROL(cmdName, right, checkRefs, true, Default)

// Note:
// Some of commands can be called with "-" prefix, which will trigger operation without option dialog
// so we attemp to monitor both formats(e.g.:-REFEDIT, REFEDIT)
#define ADD_CMD_TO_CONTROL(cmdName, right, checkRefs, alert, funcName) \
m_cmdMap.insert(std::make_pair(_W(#cmdName), UCCmdEntry{ _W(#cmdName), right, checkRefs, nullptr, L"", alert, &CheckRight_ ##funcName }));

using namespace std;

#define GRANTED(hr)   (rStatus == AcRMX::eRightGrant)
#define UNGRANTED (hr)      (rStatus != AcRMX::eRightGrant)

bool CheckRight_Default(const UCCmdEntry& ucCmd)
{
	AcRMX::AccessRightStatus rStatus;
	if (ucCmd.pCtxDoc != nullptr)
	{
		rStatus = CAcRMXAccessControl::CheckRight(ucCmd.pCtxDoc, ucCmd.dwAccessRight, ucCmd.bCheckReferences, ucCmd.bAlert);
	}
	else
	{
		rStatus = CAcRMXAccessControl::CheckRight(ucCmd.strCtxDoc, ucCmd.dwAccessRight, ucCmd.bAlert);
	}
	return (rStatus == AcRMX::eRightGrant ? true : false);
}

bool CheckRight_Preview(const UCCmdEntry& ucCmd)
{
	DWORD dwRightToCheck = ucCmd.dwAccessRight;
	// For "Output->Preview" command, it will call 2 commands. Once the first command exportsettings is canceled,
	// preview command is still triggered and we have to check "export" permission for it, instead of "print" for this case.
	if (AccessController().GetRecentVetoedCommand().CompareNoCase(L"EXPORTSETTINGS") == 0)
	{
		dwRightToCheck = RMX_RIGHT_EXPORT;
	}

	AcRMX::AccessRightStatus rStatus = AcRMX::eRightGrant;
	if (ucCmd.pCtxDoc != nullptr)
	{
		rStatus = CAcRMXAccessControl::CheckRight(ucCmd.pCtxDoc, dwRightToCheck, ucCmd.bCheckReferences);
	}
	return (rStatus == AcRMX::eRightGrant ? true : false);
}

bool CheckRight_AlwaysDisable(const UCCmdEntry& ucCmd)
{
	//Disable the command when the RMX is loaded
	CString msg = AcRMXUtils::LoadString(IDS_ERROR_DENY_WHEN_RMXLOADED);
	LOG_ERROR((LPCTSTR)msg);

	AcRMXUtils::AlertError(msg);
	return false;
}


bool CheckRight_ScreenCapture(const UCCmdEntry& ucCmd)
{
	// Suppress alert for autosave deny
	AcRMX::AccessRightStatus rStatus = AcRMX::eRightGrant;;
	if (ucCmd.pCtxDoc != nullptr)
	{
		rStatus = CAcRMXAccessControl::CheckRight(ucCmd.pCtxDoc, ucCmd.dwAccessRight, ucCmd.bCheckReferences, false);
	}
	return (rStatus == AcRMX::eRightGrant ? true : false);
}

bool CheckRight_BindXref(const UCCmdEntry& ucCmd)
{
	// Suppress alert for autosave deny
	AcRMX::AccessRightStatus rStatus = AcRMX::eRightGrant;

	// As design of AcRMXSessionCacheMgr, only keep the nxl file in cache if it's openning in session. By default, xref is loaded, but not opened.
	// But CheckRight is based on cache, so as workaround here, temporirally push xref into cache before CheckRight invoked, but remove later.
	bool addXrefToCache = false;
	if (!SessionCacheMgr().IsNxlFile(ucCmd.strCtxDoc) && SessionCacheMgr().AddNxlFile(ucCmd.strCtxDoc))
		addXrefToCache = true;
	
	rStatus = CAcRMXAccessControl::CheckRight(ucCmd.strCtxDoc, ucCmd.dwAccessRight, ucCmd.bAlert);

	if (addXrefToCache)
		SessionCacheMgr().RemoveNxlFile(ucCmd.strCtxDoc);

	return (rStatus == AcRMX::eRightGrant ? true : false);
}

//bool CheckRight_RMXSaveComplete(const UCCmdEntry& ucCmd)
//{
//	AcRMX::AccessRightStatus rStatus = CAcRMXAccessControl::CheckRight(ucCmd.strCtxDoc, ucCmd.dwAccessRight, ucCmd.bAlert);
//	if (rStatus != AcRMX::eRightGrant)
//	{
//		wchar_t szRightName[50];
//		RMX_GetRightName(ucCmd.dwAccessRight, szRightName);
//		const CString& csMsg = AcRMXUtils::FormatString(IDS_ERROR_DENY_RMXSAVECOMPLETE, szRightName);
//		LOG_ERROR((LPCTSTR)csMsg);
//		RMX_NotifyMessage((LPCWSTR)ucCmd.strCtxDoc, (LPCWSTR)csMsg);
//
//		return false;
//	}
//
//	return true;
//}

bool CheckRight_SaveAll(const UCCmdEntry& ucCmd)
{
	AcRMX::AccessRightStatus rStatus = AcRMX::eRightGrant;

	AcApDocument* pCurDoc = acDocManager->curDocument();
	AcApDocumentIterator* pIt = acDocManager->newAcApDocumentIterator();
	if (pIt)
	{
		while (!pIt->done())
		{
			//For each open document ...
			AcApDocument* pDoc = pIt->document();
			acDocManager->setCurDocument(pDoc);
			const TCHAR* szFileName = pDoc->fileName();
			if (!SessionCacheMgr().IsAllowSaveChanges(szFileName)) // If changes have been made
			{
				CString msg = AcRMXUtils::LoadString(IDS_DENY_SAVEALL);
				msg += szFileName;
				LOG_ERROR((LPCTSTR)msg);
				AcRMXUtils::AlertError(msg);

				rStatus = AcRMX::eRightDeny;
				break;
			}

			pIt->step();
		}

		delete pIt;
	}
	
	acDocManager->setCurDocument(pCurDoc);
	return GRANTED(rStatus);
}

bool CheckRight_Attach(const UCCmdEntry& ucCmd)
{
	// Case1: Don't allow to attach protected point cloud or navisworks files
	// Case2: Don't allow to attach if it's protected without extract permission.
	static std::set<std::wstring, ICaseKeyLess> sExtsToCheck = {
		L".dgn", L".nwc", L".nwd", L".rcp", L".rcs"
	};
	CPathEx fPathEx((LPCWSTR)ucCmd.strCtxDoc);
	const std::wstring wsExt = fPathEx.GetExtension();
	UCCmdEntry newCmd(ucCmd);
	if (wcsicmp(wsExt.c_str(), L".dgn") == 0)
	{
		newCmd.dwAccessRight = RMX_RIGHT_DECRYPT;
	}
	if (sExtsToCheck.find(wsExt) != sExtsToCheck.end())
	{
		return CheckRight_Default(newCmd);
	}
	
	return true;
}

void CAcRMXAccessControl::Start()
{
	LOG_INFO(L"Starting access control...");

	// Save
	ADD_CMD_TO_CONTROL_DEFAULT(QSAVE, RMX_RIGHT_EDIT, false);
	ADD_CMD_TO_CONTROL_DEFAULT(SAVE, RMX_RIGHT_EDIT, false);
	ADD_CMD_TO_CONTROL(AUTO_SAVE, RMX_RIGHT_EDIT, false, false, Default);

	// Save All
	ADD_CMD_TO_CONTROL(SAVEALL, RMX_RIGHT_EDIT, false, false, SaveAll); 

	// SaveAs
	ADD_CMD_TO_CONTROL_DEFAULT(SAVEAS, RMX_RIGHT_COPY, false);
	ADD_CMD_TO_CONTROL_DEFAULT(SAVETOWEBMOBILE, 0, true); // Disable if it's protected or any ref protected
	ADD_CMD_TO_CONTROL_DEFAULT(+SAVEAS, RMX_RIGHT_COPY, false); /*Drawing Template*/
	ADD_CMD_TO_CONTROL(DWGCONVERT, 0, false, false, AlwaysDisable); /*DWG Convert. R13.2 not support*/
	ADD_CMD_TO_CONTROL_DEFAULT(COPYBASE, RMX_RIGHT_DECRYPT, true); // Extract right required for clipboard action
	ADD_CMD_TO_CONTROL_DEFAULT(COPYCLIP, RMX_RIGHT_DECRYPT, true); // Extract right required for clipboard action
	ADD_CMD_TO_CONTROL_DEFAULT(CUTCLIP, RMX_RIGHT_EDIT | RMX_RIGHT_DECRYPT, false); // Extract right required for clipboard action
	ADD_CMD_TO_CONTROL_DEFAULT(WBLOCK, RMX_RIGHT_DECRYPT, true); // Extract for write block

	// Export - Export permission needed, including refs
	ADD_CMD_TO_CONTROL_DEFAULT(EXPORTDWF, RMX_RIGHT_EXPORT, true);
	ADD_CMD_TO_CONTROL_DEFAULT(EXPORTDWFX, RMX_RIGHT_EXPORT, true);
	ADD_CMD_TO_CONTROL_DEFAULT(EXPORTPDF, RMX_RIGHT_EXPORT, true);

	// R13.2 release, don't support below formats. simply disable
	ADD_CMD_TO_CONTROL_DEFAULT(REFEDIT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(EXPORTLAYOUT, 0, true); /*Save Layout As Drawing*/
	ADD_CMD_TO_CONTROL_DEFAULT(3DDWF, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(3DDWFPUBLISH, 0, true); // Same to 3DDWF
	ADD_CMD_TO_CONTROL_DEFAULT(DGNEXPORT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(IGESEXPORT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(STLOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(ACISOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(WMFOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(BMPOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(JPGOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(PNGOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(TIFOUT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(SAVEIMG, 0, true);
#pragma push_macro("EXPORT")
#undef EXPORT
	ADD_CMD_TO_CONTROL_DEFAULT(EXPORT, 0, true);
#pragma pop_macro("EXPORT")

	// Print permission needed, including refs.
	ADD_CMD_TO_CONTROL_DEFAULT(PLOT, RMX_RIGHT_PRINT, true);
	ADD_CMD_TO_CONTROL(PREVIEW, RMX_RIGHT_PRINT, true, true, Preview);
	ADD_CMD_TO_CONTROL_DEFAULT(VIEWPLOTDETAILS, RMX_RIGHT_PRINT, true);
	ADD_CMD_TO_CONTROL_DEFAULT(3DPRINT, RMX_RIGHT_PRINT, true);
	ADD_CMD_TO_CONTROL(PUBLISH, 0, false, false, AlwaysDisable); /*Batch Plot, not support R13.3*/
	ADD_CMD_TO_CONTROL(+PUBLISH, 0, false, false, AlwaysDisable); /*Batch Plot with dsd select option, not support R13.3*/
	ADD_CMD_TO_CONTROL_DEFAULT(AUTOPUBLISH, RMX_RIGHT_PRINT, true); 
	

	// Publish - Disable
	ADD_CMD_TO_CONTROL_DEFAULT(3DPRINTSERVICE, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(ARCHIVE, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(ETRANSMIT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(SEND, 0, false);
	ADD_CMD_TO_CONTROL_DEFAULT(SHAREVIEW, 0, false);
	ADD_CMD_TO_CONTROL_DEFAULT(SHARE, 0, true);

	// Collaborate - Disable
	ADD_CMD_TO_CONTROL_DEFAULT(PUSHTODOCSOPEN, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(COMPAREEXPORT, 0, true);
	ADD_CMD_TO_CONTROL_DEFAULT(COMPARE, 0, true);

	// Output
	ADD_CMD_TO_CONTROL(EXPORTSETTINGS, RMX_RIGHT_EXPORT, true, false, Default); // No alert as if it's denied, will trigger preview command and prompt 

	// Import - Disable if file is protected
	ADD_CMD_TO_CONTROL_DEFAULT(RMXIMPORT, 0, false);
	
	// Disabled Attach commands
	ADD_CMD_TO_CONTROL(RMXATTACH, 0, false, true, Attach);
	
	// Other functions (RMX customized)
	ADD_CMD_TO_CONTROL_DEFAULT(RMXMAPISendMail, 0, false); //Disable mail attachment if it's protected
	
	//ADD_CMD_TO_CONTROL_DEFAULT(RMXATTACHDISABLE, 0, false); // Disable attach action if it's protected. R13.2: Don't support nwc, nwd, rcp, rcs, but disable them.
	ADD_CMD_TO_CONTROL(RMXSCREENCAPTURE, RMX_RIGHT_DECRYPT, true, false, ScreenCapture);
	ADD_CMD_TO_CONTROL(RMXBINDXREF, RMX_RIGHT_DECRYPT, false, true, BindXref); //Extract permission needed if xref is binding.
	ADD_CMD_TO_CONTROL_DEFAULT(RMXNEW, RMX_RIGHT_DECRYPT, false); //Disable new from without extract right
	ADD_CMD_TO_CONTROL_DEFAULT(RMXINSERTBLOCK, RMX_RIGHT_DECRYPT, false); //Disable new from without extract right
	//ADD_CMD_TO_CONTROL(RMXSAVEASCOMPLETE, RMX_RIGHT_COPY, false, false, RMXSaveComplete);
	//ADD_CMD_TO_CONTROL(RMXSAVECOMPLETE, RMX_RIGHT_EDIT, false, false, RMXSaveComplete);

	m_importCmds = {
		L"IMPORT", L"ACISIN", L"DGNIMPORT", L"IGESIMPORT", L"WMFIN", L"3DSIN", L"PDFIMPORT"
	};
}

void CAcRMXAccessControl::Stop()
{
	LOG_INFO(L"Stopping access control...");

	m_cmdMap.clear();
}

bool CAcRMXAccessControl::CheckCommandRight(const CString& strCmdName, AcApDocument* pDoc)
{
	if (pDoc == NULL || strCmdName.IsEmpty())
		return true;

	bool ret = true;

	CString csCmdToCheck(strCmdName);
	EnsureCommandName(csCmdToCheck);

	auto cmdEntryItr = m_cmdMap.find(csCmdToCheck);
	if (cmdEntryItr != m_cmdMap.end())
	{
		// Fill context target
		cmdEntryItr->second.pCtxDoc = pDoc;
		cmdEntryItr->second.strCtxDoc.Empty();

		LOG_INFO(L"Starting to check permission on command...");
		LOG_INFO(L"\t-Command: " << (LPCTSTR)strCmdName);
		LOG_INFO(L"\t-Document: " << (LPCTSTR)pDoc->fileName());
		ret = cmdEntryItr->second.pCB(cmdEntryItr->second);

		// Special case the first command is vetoed, the nest command will be still triggered
		// Need to veto it as well. e.g.: Exportsetting, preview
		// Once the first command is called, the nest command will not be notified.
		if (!ret)
		{
			m_csRecentVetoedCmd = csCmdToCheck;
			LOG_INFO(L"End permssion check. Result: No right.");
		}
		else
		{
			LOG_INFO(L"End permssion check. Result: Granted.");
		}
	}

	return ret;
}

bool CAcRMXAccessControl::CheckCommandRight(const CString& strCmdName, const CString& strFileName)
{
	if (strCmdName.IsEmpty() || strFileName.IsEmpty())
		return true;

	bool ret = true;

	CString csCmdToCheck(strCmdName);
	EnsureCommandName(csCmdToCheck);

	auto cmdEntryItr = m_cmdMap.find(csCmdToCheck);
	if (cmdEntryItr != m_cmdMap.end())
	{
		// Fill context target
		cmdEntryItr->second.strCtxDoc = strFileName;
		cmdEntryItr->second.pCtxDoc = nullptr;

		LOG_INFO(L"Starting to check permission on command...");
		LOG_INFO(L"\t-Command: " << (LPCTSTR)strCmdName);
		LOG_INFO(L"\t-Document: " << (LPCTSTR)strFileName);
	
		ret = cmdEntryItr->second.pCB(cmdEntryItr->second);
		
		// Special case the first command is vetoed, the nest command will be still triggered
		// Need to veto it as well. e.g.: Exportsetting, preview
		// Once the first command is called, the nest command will not be notified.
		if (!ret)
		{
			m_csRecentVetoedCmd = csCmdToCheck;
			LOG_INFO(L"End permssion check. Result: No right.");
		}
		else
		{
			LOG_INFO(L"End permssion check. Result: Granted.");
		}
	}

	return ret;
}

// Other cases:
// 1. If doc has not saved to disk, ignore
// 2. If doc is not protected, ignore.
// 3. If doc is not modified, always allow to save regardless if "edit" permission is granted.
// 4. If doc is modified without edit permission, don't allow to copy/export modification
AcRMX::AccessRightStatus CAcRMXAccessControl::CheckRight(
	const CString& strFilePath,
	DWORD dwRight,
	bool bAlert /*= true*/)
{
	//
	// Only apply control if the phisical file exists
	if (!CPathEx::IsFile((LPCTSTR)strFilePath) ||
		!SessionCacheMgr().IsNxlFile((LPCTSTR)strFilePath))
	{
		return AcRMX::eRightGrant;
	}

	if (dwRight == 0)
	{
		// Some of commands are always disabled/unsupported for protected file
		CString msg = AcRMXUtils::LoadString(IDS_ERROR_DENY_ALWAYS);
		LOG_ERROR((LPCTSTR)msg);
		if (bAlert)
		{
			AcRMXUtils::AlertError(msg);
		}
		return AcRMX::eRightDeny;
	}

	// Check if file permission is granted
	if (!SessionCacheMgr().CheckRight(strFilePath, dwRight))
	{
		wchar_t szRightName[50];
		RMX_GetRightName(dwRight, szRightName);
		const CString& msg = AcRMXUtils::FormatString(IDS_ERROR_DENY_OP, szRightName);
		LOG_ERROR((LPCTSTR)msg);
		if (bAlert)
		{
			AcRMXUtils::AlertError(msg);
		}

		return AcRMX::eRightDeny;
	}

	// Special case for Copy/export and print to check if doc is modified which required edit permission
	if (HAS_BIT(dwRight, RMX_RIGHT_COPY) || HAS_BIT(dwRight, RMX_RIGHT_PRINT))
	{
		if (AcRMXUtils::IsDocModified() && !SessionCacheMgr().CheckRight(strFilePath, RMX_RIGHT_EDIT))
		{
			CString msg = AcRMXUtils::LoadString(IDS_ERROR_DENY_DIRTY);
			LOG_ERROR((LPCTSTR)msg);
			if (bAlert)
			{
				AcRMXUtils::AlertError(msg);
			}
			return AcRMX::eRightDenyEdit;
		}
	}

	return AcRMX::eRightGrant;
}

// Other cases:
// 1. If doc has not saved to disk, ignore
// 2. If doc is not protected, ignore.
// 3. If doc is not modified, always allow to save regardless if "edit" permission is granted.
// 4. If doc is modified without edit permission, don't allow to copy/export modification
AcRMX::AccessRightStatus CAcRMXAccessControl::CheckRight(
	AcApDocument* pDoc,
	DWORD dwRight, 
	bool bCheckRef,
	bool bAlert /*= true*/)
{
	if (pDoc == nullptr)
		return AcRMX::eRightGrant;

	if (pDoc->fileName() != nullptr)
	{
		// Check the main doc if it exists. 
		// Deny if the permission not found. 
		auto ret = CheckRight(pDoc->fileName(), dwRight, bAlert);
		if (ret != AcRMX::eRightGrant)
			return ret;
	}

	// Skip if the references not considered.
	if (bCheckRef)
	{
		// In case any xref protected, disable command
		if (dwRight == 0)
		{
			if (SessionCacheMgr().HasNxlRefLoaded(pDoc->database()))
			{
				CString msg = AcRMXUtils::LoadString(IDS_ERROR_DENY_ALWAYS_XREF);
				LOG_ERROR((LPCTSTR)msg);
				if (bAlert)
				{
					AcRMXUtils::AlertError(msg);
				}
				return AcRMX::eRightDenyXref;
			}

			return AcRMX::eRightGrant;
		}
		// Check permission on xrefs to determina minimal rights for print and export 
		// For performance reason, don't check all xrefs, but only abort once any one found. 
		// So return denyFiles always contains one element.
		std::set<std::wstring> denyFiles;
		if (!SessionCacheMgr().CheckRightOnXrefs(dwRight, denyFiles, pDoc->database()))
		{
			wchar_t szRightName[50];
			RMX_GetRightName(dwRight, szRightName);
			CString msg = AcRMXUtils::FormatString(IDS_ERROR_DENY_XREF, szRightName);
			msg += L"\n";
			msg += (*denyFiles.begin()).c_str();
			LOG_ERROR((LPCTSTR)msg);
			if (bAlert)
			{
				AcRMXUtils::AlertError(msg);
			}
			return AcRMX::eRightDenyXref;
		}

		// Check permission on underlays to determina minimal rights for print and export 
		// For performance reason, don't check all underlays, but only abort once any one found. 
		// So return denyFiles always contains one element.
		if (!SessionCacheMgr().CheckRightOnUnderlays(dwRight, denyFiles, pDoc->database()))
		{
			wchar_t szRightName[50];
			RMX_GetRightName(dwRight, szRightName);
			CString msg = AcRMXUtils::FormatString(IDS_ERROR_DENY_UNDERLAY, szRightName);
			msg += L"\n";
			msg += (*denyFiles.begin()).c_str();
			LOG_ERROR((LPCTSTR)msg);
			if (bAlert)
			{
				AcRMXUtils::AlertError(msg);
			}
			return AcRMX::eRightDenyUnderlay;
		}
		
	}
	return AcRMX::eRightGrant;
}

void CAcRMXAccessControl::EnsureCommandName(CString& strCmdName) const
{
	/*Command prefix:
		‘(quote mark) Transparency prefix – allows a command to be executed inside another command.Eg ‘ZOOM
		–(dash) Command line prefix – invokes a non - dialog version of a command.Eg - LAYER
		_(underscore) Non - localised command prefix – original English command names in a localised version of AutoCAD.Eg _LINE
		.	(period)Non - redefined command prefix – of commands that have been messed with by a third party, like PLOT or commands that have been undefined using the UNDEFINE command.
		+ (plus)Dialog tab prefix – for selecting a particular dialog tab.Eg ‘._ + DSETTINGS
	*/

	// Remove all possible prefix to get the global command name
	strCmdName = strCmdName.TrimLeft(L'-');
}
