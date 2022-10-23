#include "stdafx.h"
#include "ProtectControl.h"
#include "CatSession.h"
#include <LibRMX.h>

void CProtectControl::Start()
{
	LOG_INFO(L"Starting protection control...");

	// Register event listeners
	curEventMgr.AddListener((CCatEventLisener*)this, eCatNotify_AfterSave);
}

void CProtectControl::Stop()
{
	LOG_INFO(L"Stopping protection control...");
}

void CProtectControl::OnAfterSave(LPCatSaveNotify pParam)
{
	LOG_ENTER_CALL(pParam->wsFileFullName);
	if (!CPathEx::IsFile(pParam->wsFileFullName))
	{
		LOG_DEBUG(L"Skip. File not existed");
		return;
	}

	if (curCatSession.IsNxlFile(pParam->wsFileFullName))
	{
		// CatSession::IsNxlFile only checks if the original file is protected, but the current file in disk
		// may becomes unprotected after saving. Need to protect again. 
		if (!RMX_IsProtectedFile(pParam->wsFileFullName.c_str()))
		{
			// Reprotect saved file which is overwriten by normal file
			CPathEx destPath(pParam->wsFileFullName.c_str());
			if (destPath.GetExtension().compare(NXL_FILE_EXT) != 0)
				destPath += NXL_FILE_EXT;
			const string& strTags = curCatSession.GetTags(pParam->wsFileFullName);

			LOG_INFO(L"Starting new protection job...");
			LOG_INFO(L"\t->Projection job information:");
			LOG_INFO(L"\t\t-Origin: " << pParam->wsFileFullName.c_str());
			LOG_INFO(L"\t\t-Target: " << destPath.c_str());
			LOG_INFO(L"\t\t-Tags: " << StringHlp::s2ws(strTags).c_str());
			if (RMX_ProtectFile(pParam->wsFileFullName.c_str(), strTags.empty() ? nullptr : strTags.c_str(), destPath.c_str()))
			{
				pParam->bPostSaveDone = true; // Mark post save is done.
				LOG_INFO(L"Protection job finished");
			}
			else
			{
				LOG_ERROR(L"Protection job failed. File lost protection");
			}
		}
	}
	else
		LOG_DEBUG(L"Skip. File not protected.");
}
