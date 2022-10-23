//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//
#include "StdAfx.h"
#include "AcRXMProtectControl.h"
#include "AcRMXUtils.h"
#include <LibRMX.h>
#include "resource.h"
#include "AcInc.h"

#include "AcRMXSessionCacheMgr.h"
extern int acdbSetDbmod(class AcDbDatabase*, int);

void CAcRXMProtectControl::Start()
{
	LOG_INFO(L"Starting protect control...");
}

void CAcRXMProtectControl::Stop()
{
	LOG_INFO(L"Stopping protect control...");
	m_jobPool.clear();
}

bool CAcRXMProtectControl::AddJob(AcDbDatabase* pDwg, const TCHAR* szFileName)
{
	if (pDwg == NULL || szFileName == NULL) return false;
	LOG_DEBUG_FMT(L"Scheduling protect job(DB=%s, fileName=%s)...", AcRMXUtils::dbToStr(pDwg), szFileName);

	const TCHAR* szFileOrigin;
	Acad::ErrorStatus eErrorStatus = pDwg->getFilename(szFileOrigin);
	if (eErrorStatus != Acad::eOk)
		return false;

	ProtectJobPtr pJob = nullptr;
	if (wcsicmp(szFileOrigin, szFileName) == 0)
	{
		pJob = CSaveProtectJob::Create(pDwg, szFileName);
	}
	else
	{
		pJob = CCopyProtectJob::Create(pDwg, szFileName);
	}

	if (pJob) {
		m_jobPool[pJob->FileDest()] = pJob;
		return true;
	}

	return false;
}

bool CAcRXMProtectControl::AddJob(AcPublishReactorInfo* pInfo)
{
	if (pInfo == NULL) return false;

	LOG_DEBUG_FMT(L"Scheduling protect job(fileName=%s)...", pInfo->dwfFileName());

	ProtectJobPtr pJob = CPublishProtectJob::Create(pInfo);;

	if (pJob) {
		m_jobPool[pJob->FileDest()] = pJob;
		return true;
	}

	return false;
}

bool CAcRXMProtectControl::AddExportJob(const TCHAR* szFileToProtect)
{
	if (szFileToProtect == NULL) return false;

	LOG_DEBUG_FMT(L"Scheduling protect job(fileName=%s)...", szFileToProtect);

	ProtectJobPtr pJob = CExportProtectJob::Create(szFileToProtect);;

	if (pJob) {
		m_jobPool[pJob->FileDest()] = pJob;
		return true;
	}

	return false;
}

bool CAcRXMProtectControl::ExecuteJob(const TCHAR* szFileName)
{
	const std::wstring fileName(szFileName);
	auto foundJob = m_jobPool.find(fileName);
	if (foundJob != m_jobPool.end())
	{
		foundJob->second->Execute();
		m_jobPool.erase(szFileName);
		return true;
	}

	return false;
}

void SendCommandToCAD(CString cmd)
{
	cmd.Format(L"%s", cmd);

	COPYDATASTRUCT cmdMsg;

	cmdMsg.dwData = (DWORD)1;

	cmdMsg.cbData = (DWORD)_tcslen(cmd) + 1;

	cmdMsg.lpData = cmd.GetBuffer(cmd.GetLength() + 1);

	SendMessage(adsw_acadMainWnd(), WM_COPYDATA, (WPARAM)adsw_acadMainWnd(), (LPARAM)&cmdMsg);

}

bool saveChangesForCurrent(AcApDocument* pDoc, bool promptToSave, bool includeXrefs)
{
	if (!AcRMXUtils::IsDocModified()) return true;

	// Ask to save changes before protect file, if file exists.
	LOG_DEBUG_FMT(L"SaveChangesForCurrent. FileName=%s", pDoc->fileName());
	
	// Check if it's a new document which is never saved to disk
	// If so, execute "save" command before protect command. "Save" command will open save as dialog
	// to speicfy path and filename to save. 
	// rmxqprotect command will be called sequencely if file is avalaible in disk.
	// NOTE: sendStringToExecute is asynchronous
	CPathEx xFilePath(pDoc->fileName());
	if (promptToSave && xFilePath.GetParentPath().empty()) // The new doc fileName doesn't cotain directory part.
	{
		acDocManager->sendStringToExecute(acDocManager->mdiActiveDocument(), L"SAVE\n", false, true);
		CString csProtectCmd;
		csProtectCmd.Format(L"%s\n", includeXrefs ? L"rmxqprotectwithxrefs" : L"rmxqprotect");
		acDocManager->sendStringToExecute(acDocManager->mdiActiveDocument(), csProtectCmd, false, true);
		return false;
	}
	else if (!SessionCacheMgr().IsAllowSaveChanges(pDoc->fileName())) // Usage control before saving.
	{
		CString msg = AcRMXUtils::LoadString(IDS_WARN_DISCARD_CHANGE);
		msg += pDoc->fileName();
		LOG_ERROR((LPCTSTR)msg);

		AcRMXUtils::AlertWarn(msg);

		//Clear the save bit
		acdbSetDbmod(acdbCurDwg(), 0);
	}
	else
	{
		const CString& msg = AcRMXUtils::LoadString(IDS_WARN_SAVE_CHANGE);
		int nextAction = MessageBox(adsw_acadMainWnd(), msg, ACRMX_MESSAGEDLG_LABEL, MB_ICONQUESTION | MB_YESNOCANCEL);
		if (nextAction == IDCANCEL)
		{
			LOG_DEBUG(L"Save changes cancelled by user");
			return false;
		}
		else if (nextAction == IDYES)
		{
			LOG_DEBUG(L"Saving modification....");
			auto ret = pDoc->database()->saveAs(pDoc->fileName());
			CHK_ERROR_RETURN(ret != Acad::eOk, false, L"Failed to save changes (error: %s)", AcRMXUtils::GetAcErrorMessage(ret));
		}
		else // User choose "no" that means to discard changes.
		{
			LOG_DEBUG(L"Discarding changes....");
			acDocManager->lockDocument(pDoc);//Lock
			acdbSetDbmod(pDoc->database(), 0);//clear changes flag
			acDocManager->unlockDocument(pDoc);//unlock
		}
	}
	return true; // return false in case of any error or process terminated by user from prompt.
}

bool CAcRXMProtectControl::ProtectCurDocument(bool promptToSave /*= true*/)
{
	auto pCurDoc = acDocManager->mdiActiveDocument();
	if (pCurDoc == nullptr)
	{
		LOG_ERROR(L"RMXProtect - Current drawing not found. Ignore");
		return false;
	}
	std::wstring wstrFileName = pCurDoc->fileName();
	if (SessionCacheMgr().IsNxlFile(wstrFileName.c_str()))
	{
		LOG_WARN(L"RMXProtect - Protected already. Ignore");
		acutPrintf(L"\nCommand cancelled. Document protected already.");
		return false;
	}
	
	// Ask to save changes before protect file, if file exists.
	if (!saveChangesForCurrent(pCurDoc, promptToSave, false))
		return false;

	// Sanity test again in case of the new doc is not saved to disk sucessfully
	wstrFileName = pCurDoc->fileName();
	if (!CPathEx::IsFile(wstrFileName))
	{
		LOG_ERROR(L"RMXProtect - File not exists. Ignore: " << pCurDoc->fileName());
		acutPrintf(L"\nCommand cancelled: File not found in disk");
		return false;
	}

	LOG_DEBUG(L"RMXProtect - Opening protect dialog...");
	wchar_t* szTags = nullptr;
	if (RMX_ShowProtectDialog(wstrFileName.c_str(), L"Protect", &szTags))
	{
		LOG_DEBUG_FMT(L"Scheduling protect job(DB=%s, fileName=%s)...", AcRMXUtils::dbToStr(pCurDoc->database()), wstrFileName.c_str());
		ProtectJobPtr pJob = CCommandProtectJob::Create(pCurDoc->database(), wstrFileName.c_str(), RMXUtils::ws2s(szTags));
		RMX_ReleaseArray((void*)szTags);

		if (pJob && pJob->Execute()) {
			// Report to command line
			acutPrintf(L"\nCommand completed. Current document protected succesfully");

			// Try to reopen the file
			LOG_DEBUG(L"RMXProtect - Trying to reopen the file...");
			
			if (acDocManager->isApplicationContext())
			{
				Acad::ErrorStatus es = acDocManager->appContextCloseDocument(pCurDoc);
				if (es != Acad::eOk)
				{
					const TCHAR* szErrMsg = AcRMXUtils::GetAcErrorMessage(es);
					LOG_ERROR_FMT(L"RMXProtect - Failed to close current document (error: %s)", szErrMsg);
					acutPrintf(L"\nRMX: Failed to close current document (error: %s)", szErrMsg);
				}
				else
				{
					es = acDocManager->appContextOpenDocument(wstrFileName.c_str());
					if (es != Acad::eOk)
					{
						const TCHAR* szErrMsg = AcRMXUtils::GetAcErrorMessage(es);
						LOG_ERROR_FMT(L"RMXProtect - Failed to reopen document (error: %s): %s", szErrMsg, wstrFileName.c_str());
						acutPrintf(L"\nRMX:  Failed to reopen document (error: %s): %s", szErrMsg, wstrFileName.c_str());
					}
				}
			}
			else
			{
				LOG_ERROR(L"RMXProtect - Failed to reopen doc (error: Not in app context)");
				acutPrintf(L"\nRMX: Failed to reopen current document. (error: Not in application context)");
			}
		}	
	}

	return false;
}

bool CAcRXMProtectControl::ProtectCurDocumentWithXrefs(bool promptToSave /*= true*/)
{
	auto pCurDoc = acDocManager->mdiActiveDocument();
	if ((pCurDoc == nullptr) || (pCurDoc->fileName() == nullptr))
	{
		LOG_ERROR(L"RMXProtectWithXrefs - Current drawing not found. Ignore");
		return false;
	}
		
	// Ask to save changes before protect file, if file exists.
	if (!saveChangesForCurrent(pCurDoc, promptToSave, true))
		return false;
		
	std::wstring wstrCurFileName = pCurDoc->fileName();
	// Sanity test again in case of the new doc is not saved to disk sucessfully
	if (!CPathEx::IsFile(wstrCurFileName))
	{
		LOG_ERROR(L"RMXProtectWithXrefs - File not exists. Ignore: " << wstrCurFileName.c_str());
		return false;
	}

	std::set<std::wstring> filesToProtect;

	// Protect current doc
	if (!SessionCacheMgr().IsNxlFile(wstrCurFileName.c_str()))
	{
		filesToProtect.insert(wstrCurFileName);
	}

	// Scan all xrefs to protect if any
	std::wstring protectedFilesLog;
	AcRMXUtils::TraverseXrefsForCurrentDoc([&](const TCHAR* szXrefFullPath) -> bool {
		if (szXrefFullPath && !RMX_IsProtectedFile(szXrefFullPath))
		{
			filesToProtect.insert(szXrefFullPath);
		}

		return false;
	});

	if (filesToProtect.empty())
	{
		LOG_WARN(L"RMXProtectWithXrefs - No unprotected found. Ignore");
		acutPrintf(L"\nCommand cancelled. No file found.");
		return false;
	}

	wchar_t* szTags = nullptr;
	LOG_DEBUG(L"RMXProtectWithXrefs - Opening protect dialog...");
	if (RMX_ShowProtectDialog(wstrCurFileName.c_str(), L"Protect", &szTags))
	{
		const std::string& tagsToApply = RMXUtils::ws2s(szTags);
		RMX_ReleaseArray((void*)szTags);

		ULONGLONG tStartTime = GetTickCount64();

		std::set<std::wstring> newProtectedFiles;
		for (const auto& file : filesToProtect)
		{
			LOG_DEBUG_FMT(L"Scheduling protect job(DB=%s, fileName=%s)...", AcRMXUtils::dbToStr(pCurDoc->database()), file.c_str());
			ProtectJobPtr pJob = CCommandProtectJob::Create(pCurDoc->database(), file.c_str(), tagsToApply);
			if (pJob && pJob->Execute()) {
				protectedFilesLog += file;
				protectedFilesLog += L"\n";
				newProtectedFiles.insert(file);
			}
		}
		ULONGLONG tEndTime = GetTickCount64();

		
		LOG_DEBUG(L"Protect with Xrefs completed:");
		LOG_DEBUG(L"\t- Files: " << protectedFilesLog.c_str());
		LOG_DEBUG_FMT(L"\t- Time elapsed: %llu ms", tEndTime - tStartTime);

		// Try to reopen the file
		LOG_DEBUG(L"RMXProtect - Trying to reopen the file...");

		auto reopenFunc = [&](AcApDocument* pDoc, const TCHAR* fileName) -> void {
			if (acDocManager->isApplicationContext())
			{
				Acad::ErrorStatus es = acDocManager->appContextCloseDocument(pDoc);
				if (es != Acad::eOk)
				{
					LOG_ERROR_FMT(L"RMXProtect - Failed to close document (error: %s)", AcRMXUtils::GetAcErrorMessage(es));
					acutPrintf(L"\nRMX: Failed to close document");
				}
				else
				{
					es = acDocManager->appContextOpenDocument(fileName);
					if (es != Acad::eOk)
					{
						const TCHAR* szErrMsg = AcRMXUtils::GetAcErrorMessage(es);
						LOG_ERROR_FMT(L"RMXProtect - Failed to reopen document (error: %s): %s", szErrMsg, fileName);
						acutPrintf(L"\nRMX: Failed to reopen document (error: %s): %s", szErrMsg, fileName);
					}
				}
			}
			else
			{
				LOG_ERROR(L"RMXProtect - Failed to reopen doc (error: Not in app context)");
				acutPrintf(L"\nRMX: Failed to reopen  document. (error: Not in application context)");
			}

		};

		// In case any xrefs are openning, need to reopen individually after protection.
		AcApDocumentIterator* pIt;
		pIt = acDocManager->newAcApDocumentIterator();
		while (!pIt->done())
		{
			//For each open document ...
			AcApDocument* pDoc = pIt->document();
			const TCHAR* szFileName = pDoc->fileName();
			if (wstrCurFileName.compare(szFileName) != 0 &&
				(newProtectedFiles.find(szFileName) != newProtectedFiles.end()))
			{
				reopenFunc(pDoc, szFileName);
			}
			pIt->step();
		}
		delete pIt;

		// Finally, reopen current document
		reopenFunc(pCurDoc, wstrCurFileName.c_str());

		return true;
	}

	return false;
}

