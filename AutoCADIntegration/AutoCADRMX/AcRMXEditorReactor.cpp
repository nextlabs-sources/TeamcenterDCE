//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Aug 2021
//////////////////////////////////////////////////////////////////////////////
//

#include "StdAfx.h"
#include "AcRMXEditorReactor.h"
#include "AcRMXUtils.h"

#include "AcRXMProtectControl.h"
#include "AcRMXSessionCacheMgr.h"
#include "AcRMXHooks.h"
#include "AcRMXAccessControl.h"
#include "AcRMXWatermarkControl.h"
#include "LibRMX.h"
#include "resource.h"

#define AC_LOG_ENTER(msg) AC_LOG_ENTER_BODY(L"EDITORREACTOR", msg) // log guard helper

static std::set<std::wstring, ICaseKeyLess> g_attachCommandsToHook = {
	L"ATTACH",
	L"DGNATTACH",
	L"POINTCLOUDATTACH",
	L"COORDINATIONMODELATTACH", L"NWATTACH"
};
//

ACRX_NO_CONS_DEFINE_MEMBERS(CAcRMXEditorReactor, AcEditorReactor);

CAcRMXEditorReactor* CAcRMXEditorReactor::m_pInstance = NULL;
extern int acdbSetDbmod(class AcDbDatabase*, int);

CAcRMXEditorReactor::CAcRMXEditorReactor()
{
	acedEditor->addReactor(this);
}

CAcRMXEditorReactor::~CAcRMXEditorReactor()
{
	acedEditor->removeReactor(this);
}

void CAcRMXEditorReactor::UpdateViewOverlay()
{
	auto pCurDoc = acDocManager->mdiActiveDocument();
	if (pCurDoc != NULL && !SessionCacheMgr().IsNxlFile(pCurDoc->fileName()))
	{
		// Update watermark if any protected ref file changed.
		// Ignore if host drawing is protected for better performance.
		WatermarkController().UpdateViewOverlay(pCurDoc);
	}
}

bool CAcRMXEditorReactor::ApplyUsageControlOnBeginInsert(AcDbDatabase* pFrom, AcDbDatabase* pTo)
{
	bool bAllowed = true;
	const wchar_t* szFName;
	Acad::ErrorStatus eErrorStatus = pFrom->getFilename(szFName);
	if (eErrorStatus == Acad::eOk)
	{
		// Check if this dialog is template select dialog based on filters
		bool addToCache = false;
		if (!SessionCacheMgr().IsNxlFile(szFName) && SessionCacheMgr().AddNxlFile(szFName))
		{
			addToCache = true;
		}
		
		LOG_INFO(L"beginInsert triggered on a protected file:" << szFName);
		LOG_DEBUG(L"\t-pFrom: " << (LPCWSTR)AcRMXUtils::dbToStr(pFrom));
		LOG_DEBUG(L"\t-pTo: " << (LPCWSTR)AcRMXUtils::dbToStr(pTo));
		if (!AccessController().CheckCommandRight(L"RMXINSERTBLOCK", szFName))
		{
			// Cancel currently running command
			// NOTE: veto not working for beginInsert
			
			acDocManagerPtr()->sendStringToExecute(acDocManager->mdiActiveDocument(), L"\x1B\x1B");
			LOG_INFO(L"beginInsert cancelled");
			bAllowed = false;
		}

		if (addToCache)
			SessionCacheMgr().RemoveNxlFile(szFName);
	}

	return bAllowed;
}

void CAcRMXEditorReactor::endDwgOpen(const ACHAR* szFileName, AcDbDatabase* pDB)
{
	AC_LOG_ENTER(L"");

	// Bug 69642: Don't use szFileName parameter, but retrieve  fileName from pDB
	// Sometimes, the szFileName is different from the real opened file.
	const wchar_t* szFileOrigin;
	Acad::ErrorStatus eErrorStatus = pDB->getFilename(szFileOrigin);
	if (eErrorStatus != Acad::eOk)
		return;

	LOG_DEBUG(L"\t-fileName" << szFileOrigin);

	if (SessionCacheMgr().AddNxlFile(szFileOrigin))
	{
		RMX_AddActivityLog(szFileOrigin, RMX_OView, RMX_RAllowed);
		if (!SessionCacheMgr().CheckRight(szFileOrigin, RMX_RIGHT_EDIT))
		{
			// Alert the no edit permission granted to save changes
			CString msg = AcRMXUtils::LoadString(IDS_WRAN_CANNOTSAVE);
			msg += szFileOrigin;
			LOG_ERROR((LPCTSTR)msg);

			AcRMXUtils::AlertWarn(msg);
		}
	}
}

void CAcRMXEditorReactor::docCloseWillStart(AcDbDatabase* pDwg)
{
	if (pDwg != NULL)
	{
		const wchar_t* szFileOrigin;
		Acad::ErrorStatus eErrorStatus = pDwg->getFilename(szFileOrigin);
		if (eErrorStatus != Acad::eOk)
			return;

		AC_LOG_ENTER(L"");
		LOG_DEBUG(L"\t-fileName" << szFileOrigin);
		SessionCacheMgr().RemoveNxlFile(szFileOrigin);
	}
}

void CAcRMXEditorReactor::commandWillStart(const ACHAR* szCmd)
{
	// QNEW command is ended before Nav Dialog opens. so cannot unhook in 
	// commandEnded callback. 
	AcRMXHooks::Unhook_NavDlgOK();

	// TODO: Refactory for codes reuse
	CString csCmd(szCmd);
	if (!csCmd.CompareNoCase(_T("_CLOSE")) || !csCmd.CompareNoCase(_T("CLOSE")))
	{
		WatermarkController().RemoveViewOverlay();

		LOG_INFO(L"File closing. Discard changes if no edit permission found...");
		const TCHAR* szFileName = NULL;
		acdbCurDwg()->getFilename(szFileName);

		if (!SessionCacheMgr().IsAllowSaveChanges(szFileName)) // If changes have been made
		{
			CString msg = AcRMXUtils::LoadString(IDS_WARN_DISCARD_CHANGE);
			msg += szFileName;
			LOG_ERROR((LPCTSTR)msg);

			AcRMXUtils::AlertWarn(msg);

			//Clear the save bit
			acdbSetDbmod(acdbCurDwg(), 0);
		}

	}
	else if (!csCmd.CompareNoCase(_T("_QUIT")) || !csCmd.CompareNoCase(_T("QUIT")) || !csCmd.CompareNoCase(_T("CLOSEALL")))
	{
		LOG_INFO(L"App quiting. Discarding changes if no edit permission found...");
		
		AcApDocumentIterator* pIt;
		pIt = acDocManager->newAcApDocumentIterator();
		if (pIt == NULL)
			return;

		bool discardChanges = false;
		CString msg = AcRMXUtils::LoadString(IDS_WARN_DISCARD_CHANGE);
		while (!pIt->done())
		{
			//For each open document ...
			AcApDocument* pDoc = pIt->document();
			acDocManager->setCurDocument(pDoc);
			const TCHAR* szFileName = pDoc->fileName();
			if (!SessionCacheMgr().IsAllowSaveChanges(szFileName)) // If changes have been made
			{
				discardChanges = true;	
				msg += szFileName;
				msg += L"\n";

				acDocManager->lockDocument(pDoc);//Lock
				acdbSetDbmod(pDoc->database(), 0);//clear changes flag
				acDocManager->unlockDocument(pDoc);//unlock
			}
			pIt->step();
		}

		delete pIt;

		if (discardChanges)
		{
			LOG_ERROR((LPCTSTR)msg);
			AcRMXUtils::AlertWarn(msg);
		}
			
		if (!csCmd.CompareNoCase(_T("CLOSEALL")))
		{
			WatermarkController().RemoveViewOverlay();
		}
		else
		{
			// For quit, no need to update 
			WatermarkController().DisableUpdate(true);
		}
	}
	else if (!csCmd.CompareNoCase(_T("CLOSEALLOTHER")))
	{
		LOG_INFO(L"Closing other docs. Discarding changes if no edit permission found...");

		const TCHAR* szCurFileName = NULL;
		acdbCurDwg()->getFilename(szCurFileName);

		AcApDocument* pCurDoc = acDocManager->curDocument();

		AcApDocumentIterator* pIt;
		pIt = acDocManager->newAcApDocumentIterator();
		if (pIt == NULL)
			return;

		bool discardChanges = false;
		CString msg = AcRMXUtils::LoadString(IDS_WARN_DISCARD_CHANGE);
		while (!pIt->done())
		{
			//For each open document ...
			AcApDocument* pDoc = pIt->document();
			acDocManager->setCurDocument(pDoc);
			const TCHAR* szFileName = pDoc->fileName();
			if (wcsicmp(szCurFileName, szFileName) != 0 && !SessionCacheMgr().IsAllowSaveChanges(szFileName)) // If changes have been made
			{
				discardChanges = true;
				msg += szFileName;
				msg += L"\n";

				acDocManager->lockDocument(pDoc);//Lock
				acdbSetDbmod(pDoc->database(), 0);//clear changes flag
				acDocManager->unlockDocument(pDoc);//unlock
			}
			pIt->step();
		}

		delete pIt;

		acDocManager->setCurDocument(pCurDoc);

		if (discardChanges)
		{
			LOG_ERROR((LPCTSTR)msg);
			AcRMXUtils::AlertWarn(msg);
		}
	}
	else
	{
		// Note: Some of commands can be called with "-" prefix, need to include such cases.
		AccessController().EnsureCommandName(csCmd);
		if ((g_attachCommandsToHook.find((LPCWSTR)csCmd) != g_attachCommandsToHook.end()) ||
			AccessController().IsImportCmd(csCmd)) {
			LOG_INFO_FMT(L"%s command will start", szCmd);
			AcRMXHooks::Hook_NavDlgOK();
		}
	}
}

void CAcRMXEditorReactor::commandEnded(const ACHAR* szCmd)
{
	// Note: Detach, unload, reload not working for non-dwg formats. Will not handle
	static std::set<std::wstring, ICaseKeyLess> s_refCommands = {
		L"ATTACH",
		L"XATTACH",
		L"IMAGEATTACH",
		L"DWFATTACH",
		L"DGNATTACH",
		L"PDFATTACH",
	};
	// Unhook nav dialog
	// Note: Some of commands can be called with "-" prefix, need to include such cases.
	CString csCmd(szCmd);
	AccessController().EnsureCommandName(csCmd);
	if ((g_attachCommandsToHook.find((LPCWSTR)csCmd) != g_attachCommandsToHook.end()) ||
		AccessController().IsImportCmd(csCmd)) {
		LOG_INFO_FMT(L"%s command ended: %s", szCmd);
		AcRMXHooks::Unhook_NavDlgOK();
	}

	// Update overlay and screen capture in case any protected ref attached.
	// Note: Don't support point cloud and navisworks, so no need to mornitor their commands here.
	if (s_refCommands.find((LPCWSTR)csCmd) != s_refCommands.end())
	{
		UpdateViewOverlay();
	}
}

void CAcRMXEditorReactor::beginInsert(AcDbDatabase* pTo, const ACHAR* pBlockName, AcDbDatabase* pFrom)
{
	LOG_DEBUG(L"beginInsert1: " << pBlockName);
	m_wsInsertingBlock.clear();
	if (!ApplyUsageControlOnBeginInsert(pFrom, pTo))
		m_wsInsertingBlock = pBlockName;
}

void CAcRMXEditorReactor::beginInsert(AcDbDatabase* pTo, const AcGeMatrix3d& xform, AcDbDatabase* pFrom)
{
	LOG_DEBUG(L"beginInsert2");
	m_wsInsertingBlock.clear();
	ApplyUsageControlOnBeginInsert(pFrom, pTo);
}

void CAcRMXEditorReactor::abortInsert(AcDbDatabase* pTo)
{
	LOG_DEBUG(L"abortInsert: " << (LPCWSTR)AcRMXUtils::dbToStr(pTo));
	m_wsInsertingBlock.clear();
}

void CAcRMXEditorReactor::endInsert(AcDbDatabase* pTo)
{
	LOG_DEBUG(L"endInsert: " << (LPCWSTR)AcRMXUtils::dbToStr(pTo));

	// Try to delete the newly created block definition as the way to cancel command in BeginInsert
	// doesn't cancel deepclone and block definition is still created in block table. 
	AcDbBlockTable* pBlkTbl;
	AcDbBlockTableRecord* pBlkRec;
	Acad::ErrorStatus es;
	es = pTo->getBlockTable(pBlkTbl, AcDb::kForRead);
	if ((pBlkTbl == NULL) || !pBlkTbl->has(m_wsInsertingBlock.c_str()))
	{
		LOG_ERROR(m_wsInsertingBlock.c_str() << L" block not found");
		pBlkTbl->close();
		return;
	}

	pBlkTbl->getAt(m_wsInsertingBlock.c_str(), pBlkRec, AcDb::kForWrite);
	if (pBlkRec == NULL)
		return;

	// get the blocks (block references)
    // which use the block definition
	AcDbObjectIdArray ids;
	pBlkRec->getBlockReferenceIds(ids);

	// iterate each block
	for (int i = 0; i < ids.length(); i++)
	{
		AcDbObject* pObj;
		AcDbObjectId id = ids[i];
		es = acdbOpenObject(pObj, id, AcDb::OpenMode::kForWrite);
		if (es != Acad::eOk)
			continue;

		AcDbBlockReference* pBlkRef = AcDbBlockReference::cast(pObj);
		if (pBlkRef == NULL)
			continue;

		es = pBlkRef->erase();
		if (es != Acad::eOk)
		{
			LOG_ERROR_FMT(L"Failed to erase AcDbBlockReference: %s(error: %s)", m_wsInsertingBlock.c_str(), AcRMXUtils::GetAcErrorMessage(es));
		}
		LOG_INFO(L"AcDbBlockReference erased:" << (LPCWSTR)AcRMXUtils::objToHandleStr(pObj));
		pBlkRef->close();
	}
	
	es = pBlkRec->erase();
	if (es != Acad::eOk)
	{
		LOG_ERROR_FMT(L"Failed to erase AcDbBlockTableRecord: %s(error: %s)", m_wsInsertingBlock.c_str(), AcRMXUtils::GetAcErrorMessage(es));
	}
	
	LOG_INFO(L"AcDbBlockTableRecord erased: " << m_wsInsertingBlock.c_str());
	
	pBlkRec->close();
	pBlkTbl->close();

	m_wsInsertingBlock.clear();
}

//void CAcRMXEditorReactor::xrefSubcommandAttachItem(AcDbDatabase* pHost, int activity, const ACHAR* szPath)
//{
//	if (activity == kEnd)
//	{
//		LOG_INFO_FMT(L"Xref attached. Updating view overlay...");
//		UpdateViewOverlay();
//	}
//}
//

void CAcRMXEditorReactor::xrefSubcommandDetachItem(AcDbDatabase* pHost, int activity, AcDbObjectId blockId)
{
	if (activity == kEnd)
	{
		LOG_INFO(L"Xref dettached. Updating view overlay...");
		UpdateViewOverlay();
	}
}


void CAcRMXEditorReactor::xrefSubcommandReloadItem(AcDbDatabase* pHost, int activity, AcDbObjectId blockId)
{
	if (activity == kEnd)
	{
		LOG_INFO(L"Xref reloaded. Updating view overlay...");
		UpdateViewOverlay();
	}
}

void CAcRMXEditorReactor::xrefSubcommandUnloadItem(AcDbDatabase* pHost, int activity, AcDbObjectId blockId)
{
	if (activity == kEnd)
	{
		LOG_INFO(L"Xref unloaded. Updating view overlay...");
		UpdateViewOverlay();
	}
}

Acad::ErrorStatus CAcRMXEditorReactor::xrefSubCommandStart(AcXrefSubCommand eSubCmd, const AcDbObjectIdArray& arrBtrIds, const ACHAR* const* szBtrNames, const ACHAR* const* szPaths)
{
	Acad::ErrorStatus ret = Acad::eOk;
	if (eSubCmd == AcXrefSubCommand::kBind)
	{
		LOG_INFO(L"Xref bind sub command starting...");
		for (int i = 0; i < arrBtrIds.length(); ++i)
		{
			AcDbObjectId objId = arrBtrIds[i];
			if (!objId.isNull()) {
				AcDbObject* tmpObj;
				// open up for read
				Acad::ErrorStatus es = acdbOpenObject(tmpObj, objId, AcDb::kForRead, true);
				if (es == Acad::eOk) {
					AcDbBlockTableRecord* pBTR = AcDbBlockTableRecord::cast(tmpObj);
					// If it's xref, check permission  for bind command
					if (pBTR != NULL && pBTR->isFromExternalReference())
					{
						const TCHAR* szXrefFullPath = acdbOriginalXrefFullPathFor(pBTR->xrefDatabase());
						LOG_INFO(szXrefFullPath << L" XRef found. Checking permission...");
						if (!AccessController().CheckCommandRight(/*MSGO*/L"RMXBINDXREF", szXrefFullPath))
						{
							// Veto the command for deny op
							ret = Acad::eVetoed;
							tmpObj->close();
							break;
						}
					}
					tmpObj->close();
				}
			}
		}
	}
	return ret;
}

void CAcRMXEditorReactor::dwgViewResized(Adesk::LongPtr hwndDwgView)
{
	LOG_DEBUG_FMT(L"dwgViewResized %p", hwndDwgView);
	WatermarkController().ApplyAntiScreenCapture();
}

CAcRMXEditorReactor* CAcRMXEditorReactor::getInstance()
{
	if (m_pInstance)
		return m_pInstance;

	m_pInstance = new CAcRMXEditorReactor;
	LOG_DEBUG(L"Editor Reactor Added");
	return m_pInstance;
}

void CAcRMXEditorReactor::destroyInstance()
{
	if (m_pInstance) {
		delete m_pInstance;
		m_pInstance = NULL;
		LOG_DEBUG(L"Editor Reactor destroyed");
	}
	else {
		ASSERT(0);
	}
}

void CAcRMXEditorReactor::beginSave(AcDbDatabase* pDwg, const TCHAR* szIntendedName)
{
	if (szIntendedName != NULL)
	{
		AC_LOG_ENTER(L"");
		LOG_DEBUG_FMT(L"\t-szIntendedName: %s", szIntendedName);
		LOG_DEBUG_FMT(L"\t-DB: %s", AcRMXUtils::dbToStr(pDwg));
		
		//
		//TODO: Re-design for other save callbacks, which are triggered by usage controlled commands?
		// Some commands will not open the file in session, but read dwg. 
		// We have to add it into session for next protection job.
	/*	const TCHAR* szFileOrigin;
		Acad::ErrorStatus eErrorStatus = pDwg->getFilename(szFileOrigin);
		if ((eErrorStatus == Acad::eOk) && !SessionCacheMgr().IsNxlFile(szFileOrigin))
			SessionCacheMgr().AddNxlFile(szFileOrigin);*/

		// Currently WBLOCK command requires Extract right, don't need to protect 
		// saved doc from it.
		// Note: CMDNAMES returns different value for different options, so we only check substring
		// 1. Entine drawing: WBLOCK-WBLOCK
		// 2: Selected objects: WBLOCK
		// 3: -WBLOCK w/o dialog: -WBLOCK
		CString csCmd;
		AcRMXUtils::GetSysVar(L"CMDNAMES", csCmd);
		LOG_DEBUG_FMT(L"\t-CMDNAMES: %s", (LPCWSTR)csCmd);
		if (csCmd.Find(L"WBLOCK") == -1)
			ProtectController().AddJob(pDwg, szIntendedName);
	}
}

void CAcRMXEditorReactor::saveComplete(AcDbDatabase* pDwg, const TCHAR* szActualName)
{
	if (szActualName != NULL)
	{
		AC_LOG_ENTER(L"");
		LOG_DEBUG_FMT(L"\t-szActualName: %s", szActualName);
		LOG_DEBUG_FMT(L"\t-DB: %s", AcRMXUtils::dbToStr(pDwg));

		//
		//TODO: Re-design for other save callbacks, which are triggered by usage controlled commands?
		// 
		//// When the saveComplete callback is triggered from other calls(e.g.:DWG Convert) which don't have usage control,
		//// we have to check permission here and delete the newly saved file
		//CString csCmd;
		//AcRMXUtils::GetSysVar(L"CMDNAMES", csCmd);
		//if (!AccessController().IsUCCmd(csCmd))
		//{
		//	const TCHAR* szFileOrigin;
		//	Acad::ErrorStatus eErrorStatus = pDwg->getFilename(szFileOrigin);
		//	if (eErrorStatus == Acad::eOk)
		//	{
		//		CString ucCmd;
		//		if ((wcsicmp(szActualName, szFileOrigin) == 0))
		//		{
		//			ucCmd = L"RMXSAVECOMPLETE";
		//		}
		//		else
		//		{
		//			ucCmd = L"RMXSAVEASCOMPLETE";
		//		}
		//		if (!AccessController().CheckCommandRight(ucCmd, szFileOrigin))
		//		{		
		//			// Delete the newly saved document
		//			if (!CPathEx::RemoveFile(szActualName))
		//			{
		//				DWORD errorCode = (errno == EACCES) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND;
		//				CSysErrorMessage sysErr(errorCode);
		//				LOG_ERROR_FMT(L"Cannot delete file: '%s' (error: %s)", szActualName, (LPCTSTR)sysErr);
		//			}
		//			else
		//				LOG_INFO(szActualName << L" deleted");
		//			
		//			return; 
		//		}
		//	}
		//}

		ProtectController().ExecuteJob(szActualName);
	}
}

