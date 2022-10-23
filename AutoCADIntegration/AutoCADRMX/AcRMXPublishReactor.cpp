//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//
#include "StdAfx.h"
#include "AcInc.h"
#include <acpublishreactors.h>

#include "AcRMXPublishReactor.h"
#include "AcRXMProtectControl.h"

#define AC_LOG_ENTER(msg) AC_LOG_ENTER_BODY(L"PUBLISHREACTOR", msg) // log guard helper

ACRX_NO_CONS_DEFINE_MEMBERS(CAcRMXPublishReactor, AcPublishReactor);

CAcRMXPublishReactor* CAcRMXPublishReactor::m_pInstance = NULL;

CAcRMXPublishReactor::CAcRMXPublishReactor()
{

}

CAcRMXPublishReactor::~CAcRMXPublishReactor()
{
	
}

CAcRMXPublishReactor* CAcRMXPublishReactor::getInstance()
{
	if (m_pInstance)
		return m_pInstance;

	m_pInstance = new CAcRMXPublishReactor;
	HINSTANCE hInst = ::GetModuleHandle(/*MSG0*/L"AcPublish.crx");
	if ((hInst)) {
		ACGLOBADDPUBLISHREACTOR pAcGlobAddPublishReactor =
			(ACGLOBADDPUBLISHREACTOR)GetProcAddress(hInst,
				/*MSG0*/"AcGlobAddPublishReactor");

		if (pAcGlobAddPublishReactor != NULL)
			pAcGlobAddPublishReactor(m_pInstance);
	}

	LOG_DEBUG(L"Publish Reactor Added");
	return m_pInstance;
}

void CAcRMXPublishReactor::destroyInstance()
{
	if (m_pInstance) {
		HINSTANCE hInst = ::GetModuleHandle(/*MSG0*/L"AcPublish.crx");
		if ((hInst)) {
			auto pAcGlobRemovePublishReactor =
				(ACGLOBREMOVEPUBLISHREACTOR)GetProcAddress(
					hInst, /*MSG0*/"AcGlobRemovePublishReactor");
			if (pAcGlobRemovePublishReactor != NULL)
				pAcGlobRemovePublishReactor(m_pInstance);
		}
		delete m_pInstance;
		m_pInstance = NULL;
	}
	else {
		ASSERT(0);
	}
}

void CAcRMXPublishReactor::OnAboutToBeginPublishing(AcPublishBeginJobInfo* pInfo)
{
	AC_LOG_ENTER(L"Begin publishing");
}

void CAcRMXPublishReactor::OnEndPublish(AcPublishReactorInfo* pInfo)
{
	AC_LOG_ENTER(L"End publish");
	if (pInfo != NULL && ProtectController().AddJob(pInfo))
		ProtectController().ExecuteJob(pInfo->dwfFileName());
}


