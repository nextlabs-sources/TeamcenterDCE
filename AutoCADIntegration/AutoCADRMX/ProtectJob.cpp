//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//
#include "StdAfx.h"
#include "ProtectJob.h"
#include "AcInc.h"

#include <LibRMX.h>
#include "AcRMXSessionCacheMgr.h"
#include "AcRMXRPMDirMgr.h"

using namespace std;

CProtectJob::CProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName)
	: m_pDwg(pDwg), m_eOpType(AcRMX::eUnsupportedOp)
{
	if (szFileName != NULL)
		m_strFileOrigin = szFileName;
}

CProtectJob::~CProtectJob()
{
}

void CProtectJob::Dump()
{
	LOG_INFO(L"\t->Projection job information:");
	LOG_INFO(L"\t\t-Job name: " << m_strJobDesc.c_str());
	LOG_INFO(L"\t\t-Operation: " << m_eOpType);
	LOG_INFO(L"\t\t-Origin: " << m_strFileOrigin.c_str());
	LOG_INFO(L"\t\t-Target: " << m_strFileDest.c_str());
	LOG_INFO(L"\t\t-Tags: " << RMXUtils::s2ws(m_strTags).c_str());
}

bool CProtectJob::Execute()
{
	LOG_INFO(L"Starting new protection job...");
	Dump();

	CPathEx destPath(m_strFileDest.c_str());
	if (destPath.GetExtension().compare(NXL_FILE_EXT) != 0)
		destPath += NXL_FILE_EXT;
	 
	bool succeeded = true;
	wstring resultMsg;
	// Case1: Backup on Save
	// a.dwg.nxl is renamed to a.bak.nxl
	// A temp file containing changes will be renamed to a.dwg which is unprotected
	// RMX has to protect a.dwg
	// 
	// Case 2: No backup on save
	// RMX has to reprotect a.dwg.nxl in order to sync changes back to nxl file.
	// 
	// That's why to check if origin file is protected in case of origin file is same to dest file as below.
	if (RMX_IsProtectedFile(m_strFileOrigin.c_str()) && wcsicmp(m_strFileOrigin.c_str(), m_strFileDest.c_str()) == 0)
	{
		if (!RMX_RPMEditSaveFile(m_strFileDest.c_str(), nullptr, false, RMX_EditSaveMode_SaveAndExit))
		{
			resultMsg = L"Protection job failed. File lost protection";
			succeeded = false;
		}
	}
	else if (!RMX_ProtectFile(m_strFileDest.c_str(), m_strTags.empty() ? nullptr : m_strTags.c_str(), destPath.c_str()))
	{
		resultMsg = L"Protection job failed. File lost protection";
		succeeded = false;
	}
	else
	{
		resultMsg = L"Protection job finished";
	}

	if (succeeded)
	{
		// Record nxl cache
		SessionCacheMgr().AddNxlFile(m_strFileDest.c_str(), m_strTags);
		LOG_INFO(resultMsg.c_str());
	}
	else
		LOG_ERROR(resultMsg.c_str());
	
	return succeeded;
}

CSaveProtectJob::CSaveProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName)
	: CProtectJob(pDwg, szFileName)
{
	m_eOpType= AcRMX::eSaveOp;
	m_strJobDesc = L"SaveProtectJob";
}

ProtectJobPtr CSaveProtectJob::Create(AcDbDatabase* pDwg, const TCHAR* szFileName)
{
	// TODO: Ignore in case file size is 0kb
	string tags;
	if (!SessionCacheMgr().GetTags(szFileName, tags)) {
		LOG_DEBUG(L"Ignore. File not protected.");
		return false;
	}

	auto pJob = std::make_shared<CSaveProtectJob>(pDwg, szFileName);
	pJob->SetFileDest(szFileName);
	pJob->SetTags(tags);
	
	return pJob;
}

CCopyProtectJob::CCopyProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName)
	: CProtectJob(pDwg, szFileName)
{
	m_eOpType = AcRMX::eCopyOp;
	m_strJobDesc = L"CopyProtectJob";
}

ProtectJobPtr CCopyProtectJob::Create(AcDbDatabase* pDwg, const TCHAR* szFileName)
{
	// TODO: Ignore in case file size is 0kb
	if (pDwg == nullptr)
		return nullptr;

	const TCHAR* szFileOrigin;
	Acad::ErrorStatus eErrorStatus = pDwg->getFilename(szFileOrigin);
	if (eErrorStatus != Acad::eOk)
		return nullptr;

	string tags;
	if (!SessionCacheMgr().GetTags(szFileOrigin, tags)) {
		LOG_DEBUG(L"Ignore. File not protected.");
		return false;
	}

	if (wcsicmp(szFileName, szFileOrigin) == 0)
		return false;
	auto pJob = std::make_shared<CCopyProtectJob>(pDwg, szFileOrigin);
	pJob->SetFileDest(szFileName);
	pJob->SetTags(tags);

	return pJob;
}

bool CCopyProtectJob::Execute()
{
	if (CProtectJob::Execute())
	{
		// Set target folder as rpm folder becuase save as command will open the newly copy file automatically.
		CPathEx destFilePath(m_strFileDest.c_str());
		LOG_INFO(L"Seting dest dir as rpm folder on protection job completed...");
		RPMDirMgr().AddRPMDir(destFilePath.GetParentPath());
		return true;
	}

	return false;
}

CPublishProtectJob::CPublishProtectJob()
	: CProtectJob(NULL, NULL)
{
	m_eOpType = AcRMX::eExportOp;
	m_strJobDesc = L"PublishProtectJob";
}

ProtectJobPtr CPublishProtectJob::Create(AcPublishReactorInfo* pInfo)
{
	if (pInfo == NULL)
		return nullptr;
	
	auto pCurDoc = acDocManager->mdiActiveDocument();
	if (pCurDoc == NULL)
		return nullptr;

	string tags;
	if (!SessionCacheMgr().GetTagsWithRefs(pCurDoc, tags)) {
		LOG_DEBUG(L"Ignore. File not protected.");
		return false;
	}

	auto pJob = std::make_shared<CPublishProtectJob>();
	pJob->SetFileOrigin(pCurDoc->fileName());
	pJob->SetFileDest(pInfo->dwfFileName());
	pJob->SetTags(tags);
	return pJob;
}

CCommandProtectJob::CCommandProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName)
	: CProtectJob(pDwg, szFileName)
{
	m_eOpType = AcRMX::eProtectCommandOp;
	m_strJobDesc = L"CommandProtectJob";
}

ProtectJobPtr CCommandProtectJob::Create(AcDbDatabase* pDwg, const TCHAR* szFileName, const std::string& tags)
{
	auto pJob = std::make_shared<CCommandProtectJob>(pDwg, szFileName);
	pJob->SetFileOrigin(szFileName);
	pJob->SetFileDest(szFileName);
	pJob->SetTags(tags);

	return pJob;
}

bool CCommandProtectJob::Execute()
{
	LOG_INFO(L"Starting new protection job...");
	Dump();

	CPathEx destPath(m_strFileDest.c_str());
	
	// Bug 71932 - Attempt to protect .bak file if it exists
	// However, treat job succeeded even it's failed, once the main file is protected.
	CPathEx bakPath(destPath.GetParentPath().c_str());
	bakPath /= destPath.GetFileName().c_str();
	bakPath += L".bak";
	if (CPathEx::IsFile(bakPath.ToString()))
	{
		LOG_INFO(bakPath.c_str() << L" found. Protecting it...");
		wstring destBakNxlFile = bakPath.ToString() + NXL_FILE_EXT;
		if (!RMX_ProtectFile(bakPath.c_str(), m_strTags.empty() ? nullptr : m_strTags.c_str(), destBakNxlFile.c_str()))
		{
			LOG_ERROR(L"Protection failed for .bak file");
		}
		else
		{
			LOG_INFO(bakPath.c_str() << L" protected.");
		}
	}

	if (destPath.GetExtension().compare(NXL_FILE_EXT) != 0)
		destPath += NXL_FILE_EXT;

	bool succeeded = true;
	if (!RMX_ProtectFile(m_strFileDest.c_str(), m_strTags.empty() ? nullptr : m_strTags.c_str(), destPath.c_str()))
	{
		LOG_ERROR(L"Protection job failed. File not protected");
		succeeded = false;
	}
	else
	{
		// Set target folder as rpm folder becuase save as command will open the newly copy file automatically.
		CPathEx destFilePath(m_strFileDest.c_str());
		LOG_INFO(L"Setting dest dir as rpm folder on protection job completed...");
		RPMDirMgr().AddRPMDir(destFilePath.GetParentPath());

		LOG_INFO(L"Protection job finished");

	}

	return succeeded;
}

CExportProtectJob::CExportProtectJob()
	: CProtectJob(NULL, NULL)
{
	m_eOpType = AcRMX::eExportOp;
	m_strJobDesc = L"ExportProtectJob";
}

ProtectJobPtr CExportProtectJob::Create(const TCHAR* szFileToProtect)
{
	auto pCurDoc = acDocManager->mdiActiveDocument();
	if (pCurDoc == NULL)
		return nullptr;

	string tags;
	if (!SessionCacheMgr().GetTagsWithRefs(pCurDoc, tags)) {
		LOG_DEBUG(L"Ignore. File not protected.");
		return false;
	}

	auto pJob = std::make_shared<CPublishProtectJob>();
	pJob->SetFileOrigin(pCurDoc->fileName());
	pJob->SetFileDest(szFileToProtect);
	pJob->SetTags(tags);
	return pJob;
}
