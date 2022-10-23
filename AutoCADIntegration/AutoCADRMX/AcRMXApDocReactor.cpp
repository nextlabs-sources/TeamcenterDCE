//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Aug 2021
//////////////////////////////////////////////////////////////////////////////
//

#include "StdAfx.h"
#include <tchar.h>

#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include "AcRMXApDocReactor.h"
#include "AcRMXUtils.h"
#include <LibRMX.h>
#include "AcRMXAccessControl.h"
#include "AcRMXWatermarkControl.h"
#include "AcRMXRPMDirMgr.h"
#include "AcRMXSessionCacheMgr.h"
#include "AcRMXHooks.h"

#define AC_LOG_ENTER(msg)  AC_LOG_ENTER_BODY(L"DOCREACTOR", msg) // log guard helper

ACRX_NO_CONS_DEFINE_MEMBERS(CAcRMXApDocReactor, AcApDocManagerReactor);

CAcRMXApDocReactor* CAcRMXApDocReactor::m_pInstance = NULL;

CAcRMXApDocReactor::CAcRMXApDocReactor()
{
	acDocManager->addReactor(this);
}

CAcRMXApDocReactor::~CAcRMXApDocReactor()
{
	acDocManager->removeReactor(this);
}

CAcRMXApDocReactor* CAcRMXApDocReactor::getInstance()
{
	if (m_pInstance)
		return m_pInstance;

	m_pInstance = new CAcRMXApDocReactor;
	LOG_DEBUG(L"Document Reactor Added");
	return m_pInstance;
}

void CAcRMXApDocReactor::destroyInstance()
{
	if (m_pInstance) {
		delete m_pInstance;
		m_pInstance = NULL;
		LOG_DEBUG(L"Document Reactor removed");
	}
	else {
		ASSERT(0);
	}
}

void CAcRMXApDocReactor::documentCreateStarted(AcApDocument* pDocCreating)
{
	AC_LOG_ENTER(L"");
	AcRMXHooks::Hook_NavDlgOK(true);
}

void CAcRMXApDocReactor::documentCreated(AcApDocument* pDocCreating)
{
	AcRMXHooks::Unhook_NavDlgOK();
}

void CAcRMXApDocReactor::documentCreateCanceled(AcApDocument* pDocCreateCancelled)
{
	AcRMXHooks::Unhook_NavDlgOK();
}

void CAcRMXApDocReactor::documentToBeDestroyed(AcApDocument* pDoc)
{
	AC_LOG_ENTER(L"");
	if (pDoc)
	{
		SessionCacheMgr().RemoveNxlFile(pDoc->fileName());
	}
}

void CAcRMXApDocReactor::documentLockModeChanged(AcApDocument* pDoc,
	AcAp::DocLockMode eMyPreviousMode,
	AcAp::DocLockMode eMyCurrentMode,
	AcAp::DocLockMode eCurrentMode,
	const TCHAR* szGlobalCmdName)
{
	if (pDoc == NULL || wcslen(szGlobalCmdName) == 0 || 
		wcscmp(szGlobalCmdName, L"#") == 0)
		return;

#ifndef NDEBUG
	/*AC_LOG_ENTER(szGlobalCmdName);
	LOG_DEBUG_FMT(L"\t myPreviousMode: %s", AcRMXUtils::DocLockModeToStr(eMyPreviousMode));
	LOG_DEBUG_FMT(L"\t myCurrentMode: %s", AcRMXUtils::DocLockModeToStr(eMyCurrentMode));
	LOG_DEBUG_FMT(L"\t currentMode: %s", AcRMXUtils::DocLockModeToStr(eCurrentMode));

	LOG_DEBUG_FMT(L"\t pGlobalCmdName: %s", szGlobalCmdName);*/
#endif // !NDEBUG

	// Format the command name if required here. e.g.: SAVEALL
	CString csCmd(szGlobalCmdName);
	if (wcsicmp(szGlobalCmdName, L"(C:SAVEALL)") == 0)
		csCmd = L"SAVEALL";
	if (!AccessController().CheckCommandRight(csCmd, pDoc))
	{
		// Cancel the command if right is not granted.
		veto();
	}
	else if (wcsicmp(szGlobalCmdName, L"CRASHSAVE") == 0)
	{
		AcRMXHooks::Hook_CreateFile();
	}
	else if (wcsicmp(szGlobalCmdName, L"#CRASHSAVE") == 0)
	{
		AcRMXHooks::Unhook_CreateFile();
	}
}

void CAcRMXApDocReactor::documentActivated(AcApDocument* pActivatedDoc)
{
	AC_LOG_ENTER(L"");
	if (pActivatedDoc != NULL)
	//	LOG_DEBUG_FMT(L"\t-ActivatedDoc: %s", pActivatedDoc->docTitle());
		WatermarkController().UpdateViewOverlay(pActivatedDoc);

}