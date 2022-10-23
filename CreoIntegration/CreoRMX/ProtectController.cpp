#include "ProtectController.h"

#include <regex>
#include <shlwapi.h>

#include "..\common\CreoRMXLogger.h"
#include <SysErrorMessage.h>

#include <LibRMX.h>

#include "OtkXHooks.h"
#include "OtkXUIStrings.h"
#include "OtkXUtils.h"
#include <PathEx.h>
#include <RMXUtils.h>

#include <pfcGlobal.h>

#include "CustomizeUsageControl.h"
#include "FileClosedHooks.h"
#include "OtkXSessionCache.h"
#include "ProtectJob.h"
#include "RPMDirectoryMgr.h"

using namespace std;
using namespace OtkXUtils;

static const wchar_t* FILETYPES_TO_FIXNAME[] = {
	L"stl", L"iv", L"slp", L"obj", L"gbf", L"asc", L"facet", L"amf"
};

#pragma region otkxlistener
void COtkXModelActionListener::OnAfterModelSaveAll(pfcModelDescriptor_ptr pDescr)
{
	PC_LOG_ENTER
	pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pDescr);
	if (pMdl == nullptr)
		return;
	OTKX_MODELDESCRIPTOR_DUMP(pDescr);
	pfcModelDescriptors_ptr pDescrs = pfcModelDescriptors::create();
	pDescrs->append(pDescr);
	CProtectController::GetInstance().Protect(pDescrs, SOURCEACTION_SAVE);
}

void COtkXModelActionListener::OnAfterModelRetrieveAll(pfcModelDescriptor_ptr pDescr)
{
	OTKCALL_LOG_ENTER
	pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pDescr);
	if (pMdl != nullptr)
	{
		COtkXModel xMdl(pMdl);
		if (xMdl.IsProtected())
		{
			if (!RMX_Initialize())
			{
				LOG_ERROR(L"RMX is not running. Protection may be lost after editing the file: " << xMdl.GetOrigin().c_str());
			}
			else
			{
				LOG_INFO(L"Protected file opened: " << xMdl.GetOrigin().c_str());
				OtkXSessionCacheHolder().AddNxlModel(xMdl.GetId(), xMdl.GetOrigin());

				OtkX_DisplayMessageForNxlFileInfo(pMdl);

				RMX_AddActivityLog(xMdl.GetOrigin().c_str(), RMX_OView, RMX_RAllowed);
			}
		}
		else
		{
			OtkXSessionCacheHolder().EraseNxlModel(xMdl);
		}

		if (CProtectController::GetInstance().IsWatchingJTExport())
		{
			CProtectController::GetInstance().AddJTCADFile(pMdl);
		}	
	}
}

void COtkXModelActionListener::OnAfterModelEraseAll(pfcModelDescriptor_ptr pDescr)
{
	OTKCALL_LOG_ENTER
	pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pDescr);
	if (pMdl != nullptr)
	{
		COtkXModel xMdl(pMdl);
		OtkXSessionCacheHolder().EraseNxlModel(xMdl);
	}
}

void COtkXModelEventActionListener::OnAfterModelCopy(pfcModelDescriptor_ptr pFromDescr, pfcModelDescriptor_ptr pToDescr)
{
	// This is the entry point to distinguish between copy and export.
	// Both of them will trigger OnBeforeModelCopy, that activate the both export and copy watches. 
	// But export does not trigger OnAfterModelCopy, so the export file watcher need to be terminated.
	OTKCALL_LOG_ENTER
	CProtectController::GetInstance().RemoveFileWatcher(PROTECT_WATCHER_EXPORT);
	CProtectController::GetInstance().StopFileWatcher(PROTECT_WATCHER_COPY);
}

void COtkXModelEventActionListener::OnAfterModelCopyAll(pfcModelDescriptor_ptr pFromDescr, pfcModelDescriptor_ptr pToDescr)
{
	PC_LOG_ENTER
	auto pFromMdl = OtkX_GetModelFromDescr(pFromDescr);
	auto pToMdl = OtkX_GetModelFromDescr(pToDescr);
	if (pFromMdl == nullptr || pToMdl == nullptr)
		return;
	// Monitor the nxl file copy action
	OTKX_MODELDESCCOPY_DUMP(pFromDescr, pToDescr);

	if (!CProtectController::NeedToProtect(pFromMdl))
	{
		//CProtectController::GetInstance().RemoveNxlParam(pFromMdl);
		LOG_INFO_FMT(L"Skip protection control. Source mode(%s) that was copied not protected", CXS2W(pFromMdl->GetInstanceName()));
		return;
	}
	
	COtkXModel fromXMdl(pFromMdl);
	COtkXModel toXMdl(pToMdl);
	OtkXSessionCacheHolder().CopyNxlModel(fromXMdl, toXMdl);

	pfcModelDescriptors_ptr pDescrs = pfcModelDescriptors::create();
	pDescrs->append(pFromDescr);
	pDescrs->append(pToDescr);
	CProtectController::GetInstance().Protect(pDescrs, SOURCEACTION_COPY);
}

void COtkXModelEventActionListener::OnAfterModelRename(pfcModelDescriptor_ptr pFromDescr, pfcModelDescriptor_ptr pToDescr)
{
	OTKCALL_LOG_ENTER
	auto pFromMdl = OtkX_GetModelFromDescr(pFromDescr);
	auto pToMdl = OtkX_GetModelFromDescr(pToDescr);
	if (pFromMdl == nullptr || pToMdl == nullptr)
		return;
	COtkXModel fromXMdl(pFromMdl);
	COtkXModel toXMdl(pToMdl);
	OtkXSessionCacheHolder().RenameNxlModel(fromXMdl, toXMdl);
	//CProtectController::GetInstance().RenameMxlModelInSession(pFromMdl, pToMdl);
}

void COtkXBeforeModelSaveAllListener::OnBeforeModelSave(pfcModel_ptr pMdl)
{
	PC_LOG_ENTER
	if (pMdl == nullptr) return;

	COtkXModel xMdl(pMdl);
	// Monitor the nxl file saving action
	if (!OtkXSessionCacheHolder().IsNxlFile(xMdl.GetOrigin()))
	{
		//CProtectController::GetInstance().RemoveNxlParam(pMdl);
		LOG_INFO_FMT(L"Skip protection control. Model(%s) to be saved not protected", CXS2W(pMdl->GetInstanceName()));
		return;
	}
	// Sometimes, OnBeforeModelSave is triggered even if the model is not modified.
	// But OnAfterModelSave will not be triggered. Skip such case.
	
	if (pMdl->GetIsModified() && xMdl.IsValid())
	{
		OtkXSessionCacheHolder().AddNxlModel(xMdl.GetId(), xMdl.GetOrigin());
		// HACK. Back up the file if the right is not applied, file modification
		// will be discarded later by replacing the backup file.
		CProtectController::GetInstance().TryToBackupFile(xMdl.GetOrigin());
	}
	else
	{
		LOG_INFO(L"Skip protection control. Model not modified");
	}
}

#pragma endregion otkxlistener

void CProtectController::Start()
{
	LOG_INFO(L"Starting protection control...");
	m_runningFileWatches = 0;
	m_isInProcess = false;
	try
	{
		wfcWSession_ptr pWSession = wfcWSession::cast(pfcGetProESession());
		if (pWSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}

		// Register model level listeners for 'Save' actions
		pfcActionListener_ptr mdlActionListener = new COtkXModelActionListener();
		pWSession->AddActionListener(mdlActionListener);
		m_listeners.push_back(mdlActionListener);
		LOG_INFO(L"\t-COtkXModelActionListener added");

		// Register model event listeners for 'Copy' actions
		pfcActionListener_ptr mdlEventActionListener = new COtkXModelEventActionListener();
		pWSession->AddActionListener(mdlEventActionListener);
		m_listeners.push_back(mdlEventActionListener);
		LOG_INFO(L"\t-COtkXModelEventActionListener added");

		// Register listener for "BeforeSave" action
		wfcBeforeModelSaveAllListener_ptr pPreSaveAllListener = new COtkXBeforeModelSaveAllListener();
		pWSession->AddModelSavePreAllListener(pPreSaveAllListener);
		m_listeners.push_back(pfcActionListener::cast(pPreSaveAllListener));
		LOG_INFO(L"\t-COtkXBeforeModelSaveAllListener added");

		pfcActionListener_ptr pSessListener = new COtkXSessionActionListener();
		pWSession->AddActionListener(pSessListener);
		m_listeners.push_back(pSessListener);
		LOG_INFO(L"\t-COtkXSessionActionListener added");

		// Check if the image file will be created when performing a save on a model
		// If so, image file needs to be protected.
		xstring saveBitmapOpt = pWSession->GetConfigOption("save_bitmap");
		LOG_INFO(L"\t-save_bitmap: " << CXS2W(saveBitmapOpt));
	
		xstring bitmapType = pWSession->GetConfigOption("save_bitmap_type");
		LOG_INFO(L"\t-save_bitmap_type: " << CXS2W(bitmapType));
		if (saveBitmapOpt.ToLower() != "none")
		{
			m_imageExtOnSave = OtkX_GetImageExtention(bitmapType);	
		}
		
		for (const auto fType : FILETYPES_TO_FIXNAME)
		{
			m_fileTypesToFixName.insert(fType);
		}
		xstring saveFileIterations = pWSession->GetConfigOption("save_file_iterations");
		LOG_INFO(L"\t-save_file_iterations: " << CXS2W(saveFileIterations));
	}
	OTKX_EXCEPTION_HANDLER();
}

void CProtectController::Stop()
{
	if (m_listeners.empty())
		return;

	LOG_INFO(L"Stopping protection control...");
	StopFileWatcher(m_runningFileWatches);

	// Remove listener from command
	try
	{
		pfcSession_ptr pSession = pfcGetProESession();
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}
		for (const auto& lis : m_listeners)
		{
			if (lis != nullptr)
				pSession->RemoveActionListener(lis);
		}

		m_listeners.clear();
		
		for (const auto& file : m_backupFiles)
		{
			RMX_DeleteNxlFile(file.second.c_str());
		}
		m_backupFiles.clear();
	}
	OTKX_EXCEPTION_HANDLER();

	m_fileTypesToFixName.clear();
}

void CProtectController::Protect(pfcModelDescriptors_ptr pMdlDescrs, ProtectSourceAction srcAction)
{
	RMX_SCOPE_GUARD_ENTER(&m_isInProcess);
	ProtectJobPtr pJob = nullptr;
	if (srcAction == SOURCEACTION_SAVE)
	{
		auto pDescr = pMdlDescrs->get(0);
		pJob = CSaveProtectJob::Create(pDescr);

		// Special case for Shrinkwrap to export a part or lightweight part
		// Check if new saved part has Geom copy feature
		if (pJob == nullptr && pDescr->GetType() == pfcMDL_PART && IsWatchingExport())
		{	
			auto pMdl = OtkX_GetModelFromDescr(pDescr);
			try
			{
				auto pSolid = pfcSolid::cast(pMdl);
				if (pSolid != nullptr)
				{
					auto pFeats = pSolid->ListFeaturesByType(false, pfcFEATTYPE_GEOM_COPY);
					if (pFeats && pFeats->getarraysize() > 0)
					{
						// Set marker to remember this export type
						StartFileWatcher(PROTECT_WATCHER_SHRINKWRAP);
						LOG_DEBUG_FMT(L"GeomCopyHandler: %s", CXS2W(pMdl->GetOrigin()));
						// Trigger to protect exported files
						ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
					}
				}
			}
			OTKX_EXCEPTION_HANDLER();
		}
	}		
	else if (srcAction == SOURCEACTION_COPY)
	{	
		if (m_watchedFilesForCopy.empty())
			return;
		
		// It's not able to retrieve the target file path from the pfcModelDescriptor
		// Try to grab it from the watching file which was newly created during copy process.
		auto pToDescr = pMdlDescrs->get(1);
		wstring path = CXS2W(pToDescr->GetPath());
		wstring fileName = CXS2W(pToDescr->GetFullName());
		wstring ext = CXS2W(pToDescr->GetExtension());
		OtkXFileNameData destFNameData(path, fileName, ext, pToDescr->GetFileVersion());
		auto findIter = std::find_if(m_watchedFilesForCopy.cbegin(), m_watchedFilesForCopy.cend(),
			[&destFNameData](const wstring &wFile)
		{ 
			OtkXFileNameData watchedFNameData(wFile);
			return watchedFNameData.IsSameVersionFile(destFNameData);
		});
		if (findIter != m_watchedFilesForCopy.end())
		{
			if (CPathEx::IsFile(*findIter))
			{
				pJob = CCopyProtectJob::Create(pToDescr, *findIter);
			}
				
			m_watchedFilesForCopy.erase(findIter);
		}		
	}

	ExecuteJob(pJob);

	if (pJob != nullptr)
	{
		// Protect auxiliary file
		const wstring& imageFilePath = CAuxiImageFileProtectJob::BuildImageFilePath(pJob->GetFileDest(), m_imageExtOnSave);
		auto pAuxiImageJob = CAuxiImageFileProtectJob::Create(pJob, imageFilePath);
		if (pAuxiImageJob != nullptr)
		{
			pAuxiImageJob->Execute();

			// Skip image file generated as auxiliary file.
			// It will be protected after the main file gets protected.
			m_watchedFilesForCopy.erase(imageFilePath);
			m_watchedFilesForSave.erase(imageFilePath);
		}

		if (pJob != nullptr && IsWatcherRunning(PROTECT_WATCHER_BACKUP))
		{
			// Special handling to mark rpm folder for backup
			// to ensure newly nxl file can be  opened
			OtkXFileNameData destNameData(pJob->GetFileDest());
			CRPMDirectoryMgr::GetInstance().AddRPMDir(destNameData.GetDirectoryPath());
		}
	}
}

bool CProtectController::ProcessCopyFile(const std::wstring & sourceFile, const std::wstring & targetFile)
{
	auto pJob = CFileCopyProtectJob::Create(sourceFile, targetFile);
	if (pJob != nullptr && pJob->Execute())
	{
		const wstring& imageFilePath = CAuxiImageFileProtectJob::BuildImageFilePath(targetFile, m_imageExtOnSave);
		auto pAuxiImageJob = CAuxiImageFileProtectJob::Create(pJob, imageFilePath);
		if (pAuxiImageJob != nullptr)
		{
			pAuxiImageJob->Execute();
		}

		return true;
	}

	return false;
}

void CProtectController::ProtectCurrentCommand()
{
	PC_LOG_ENTER
	auto pCurMdl = OtkX_GetCurrentModel();
	if (pCurMdl == nullptr)
	{
		LOG_ERROR(L"Abort. No current model found");
		return;
	}
	COtkXModel xMdlTmp(pCurMdl);
	if (!xMdlTmp.IsSaveAllowed())
	{
		OtkX_ShowMessageDialog(IDS_ERROR_INCOMPLETE_OP, pfcMESSAGE_ERROR);
		LOG_ERROR(L"Abort. Incomplete operation found in the sesssion");
		return;
	}
	else if (!xMdlTmp.FileExists())
	{
		// Save file to disk if it's new model
		xstring msg(IDS_WARN_SAVE_CURRENT_MODEL);
		LOG_ERROR(RMXUtils::s2ws(msg).c_str());
		pfcMessageButton askToSave = OtkX_ShowMessageDialog2(msg, pfcMESSAGE_WARNING);
		if (askToSave == pfcMESSAGE_BUTTON_CANCEL)
		{
			LOG_INFO(L"Abort. Protect command cancelled by user");
			return;
		}

		// Save new file to disk
		// TODO: Should call SaveAs to ask file name ? 
		pCurMdl->Save();
		LOG_INFO(L"Save current model to disk" << CXS2W(pCurMdl->GetOrigin()));

	}
	else if (pCurMdl->GetIsModified())
	{
		xstring msg(IDS_WARN_SAVE_CURRENT_CHANGES);
		LOG_ERROR(RMXUtils::s2ws(msg).c_str());
		pfcMessageButton askToSave = OtkX_ShowMessageDialog3(msg, pfcMESSAGE_WARNING);
		if (askToSave == pfcMESSAGE_BUTTON_CANCEL)
		{
			LOG_INFO(L"Abort. Protect command cancelled by user");
			return;
		}
		else if (askToSave == pfcMESSAGE_BUTTON_YES)
		{
			// Save changes
			pCurMdl->Save();
			LOG_INFO(L"Save changes in the model to disk: " << xMdlTmp.GetOrigin().c_str());
		}
	}

	// Validate model again.
	COtkXModel xCurMdl(pCurMdl);
	if (!xCurMdl.FileExists())
	{
		LOG_ERROR(L"Abort. Model file not found in disk");
		return;

	}

	bool result = false;
	wchar_t* szTags = nullptr;
	if (RMX_ShowProtectDialog(xCurMdl.GetOrigin().c_str(), L"Protect", &szTags))
	{
		string tagsToApply = RMXUtils::ws2s(szTags);
		RMX_ReleaseArray((void*)szTags);
		auto pJob = CCommandProjectJob::Create(pCurMdl, tagsToApply);
		result = ExecuteJob(pJob);

		if (result)
		{
			// Reopen the file
			if (!OtkX_EraseModel(pCurMdl, false))
			{
				// Models used by other models cannot be erased until the models dependent upon them are erased
				// For this case, prompt to user for manual reopen request.
				OtkXSessionCacheHolder().AddDirtyNxlToReload(xCurMdl.GetId());
				LOG_ERROR(IDS_ERROR_CANNOT_ERASE_AFTER_PROTECT);
				OtkX_ShowMessageDialog(IDS_ERROR_CANNOT_ERASE_AFTER_PROTECT, pfcMESSAGE_ERROR);
				return;
			}
			OtkX_ReopenFile(pJob->GetFileOrigin());
		}
	}
}

void CProtectController::ProtecCurrentWithDepsCommand()
{
	PC_LOG_ENTER
	auto pCurMdl = OtkX_GetCurrentModel();
	if (pCurMdl == nullptr)
	{
		LOG_ERROR(L"Abort. No current model found");
		return;
	}

	wstring protectedFilesLog;
	std::vector<pfcModel_ptr> modelsToSaveNew;
	std::vector<pfcModel_ptr> modelsToSaveChanges;
	std::vector<pfcModel_ptr> modelsToProtect;
	xstring saveNewNames, saveChangesNames;

	COtkXModel xMdlTmp(pCurMdl);
	if (!xMdlTmp.IsSaveAllowed())
	{
		OtkX_ShowMessageDialog(IDS_ERROR_INCOMPLETE_OP, pfcMESSAGE_ERROR);
		LOG_ERROR(L"Abort. Incomplete operation found in the sesssion");
		return;
	}
	else if (!xMdlTmp.FileExists())
	{
		saveNewNames = RMXUtils::ws2s(xMdlTmp.GetId()).c_str();
		saveNewNames += "\n";
		modelsToSaveNew.push_back(pCurMdl);
		modelsToProtect.push_back(pCurMdl);
	}
	else if (!OtkXSessionCacheHolder().IsNxlModel(xMdlTmp))
	{
		modelsToProtect.push_back(pCurMdl);
		if (pCurMdl->GetIsModified())
		{
			modelsToSaveChanges.push_back(pCurMdl);
			saveChangesNames = RMXUtils::ws2s(xMdlTmp.GetId()).c_str();
			saveChangesNames += "\n";
		}
	}
	xMdlTmp.TraverseDependencies([&](COtkXModel& xDepMdl) -> bool {
		if (!xDepMdl.FileExists())
		{
			saveNewNames += RMXUtils::ws2s(xDepMdl.GetId()).c_str();
			saveNewNames += "\n";
			modelsToSaveNew.push_back(xDepMdl.GetModel());
			modelsToProtect.push_back(xDepMdl.GetModel());
		}
		else if (!OtkXSessionCacheHolder().IsNxlModel(xDepMdl))
		{
			modelsToProtect.push_back(xDepMdl.GetModel());
			if (xDepMdl.GetModel()->GetIsModified())
			{
				modelsToSaveChanges.push_back(xDepMdl.GetModel());
				saveChangesNames += RMXUtils::ws2s(xDepMdl.GetId()).c_str();
				saveChangesNames += "\n";
			}
		}
	
		return false;
	});

	if (modelsToProtect.empty())
	{
		return; // Impossible. Command access callback should have already check this to disable command.
	}

	if (modelsToSaveNew.size() > 0)
	{
		// Save file to disk if it's new model
		xstring msg = xstring::Printf(IDS_WARN_SAVE_NEW_MODELS, (cStringT)saveNewNames);
		LOG_ERROR(RMXUtils::s2ws(msg).c_str());
		pfcMessageButton askToSave = OtkX_ShowMessageDialog2(msg, pfcMESSAGE_WARNING);
		if (askToSave == pfcMESSAGE_BUTTON_CANCEL)
		{
			LOG_INFO(L"Abort. Protect command cancelled by user");
			return;
		}

		// Save new file to disk
		// TODO: Should call SaveAs to ask file name ? 
		for (auto pMdl : modelsToSaveNew)
		{
			pCurMdl->Save();
			LOG_INFO(L"New model saved: " << CXS2W(pCurMdl->GetOrigin()));
		}
	}
	if (modelsToSaveChanges.size() > 0)
	{
		// Save file to disk if it's new model
		xstring msg = xstring::Printf(IDS_WARN_SAVE_CHANGES, (cStringT)saveChangesNames);
		LOG_ERROR(RMXUtils::s2ws(msg).c_str());
		pfcMessageButton askToSave = OtkX_ShowMessageDialog3(msg, pfcMESSAGE_WARNING);
		if (askToSave == pfcMESSAGE_BUTTON_CANCEL)
		{
			LOG_INFO(L"Abort. Protect command cancelled by user");
			return;
		}
		else if (askToSave == pfcMESSAGE_BUTTON_YES)
		{
			for (auto pMdl : modelsToSaveChanges)
			{
				pCurMdl->Save();
				LOG_INFO(L"Model changes saved: " << CXS2W(pCurMdl->GetOrigin()));
			}
		}
		else
		{
			LOG_INFO(L"Model changes discarded: " << CXS2W(saveChangesNames));
		}
	}

	COtkXModel xCurMdl(pCurMdl);
	wchar_t* szTags = nullptr;
	if (!RMX_ShowProtectDialog(xCurMdl.GetOrigin().empty()? xCurMdl.GetId().c_str() : xCurMdl.GetOrigin().c_str(), L"Protect", &szTags))
	{
		LOG_ERROR(L"Abort. Classification not specified.");
		return;
	}

	string tagsToApply = RMXUtils::ws2s(szTags);
	RMX_ReleaseArray((void*)szTags);

	ULONGLONG tStart = GetTickCount64();
	for (auto iter = modelsToProtect.rbegin(); iter != modelsToProtect.rend(); ++iter)
	{
		// Valid file again in case above save call failed
		COtkXModel xMdl(*iter);
		if (!xMdl.FileExists())
		{
			LOG_ERROR(L"Skip. Model file not found: " << xMdl.GetId().c_str());
			continue;
		}

		auto pJob = CCommandProjectJob::Create(xMdl.GetModel(), tagsToApply);
		if (ExecuteJob(pJob))
		{
			// CloseModel
			OtkX_CloseModel(xMdl.GetModel());
			protectedFilesLog += xMdl.GetOrigin();
			protectedFilesLog += L"\n";
		}

	}
	ULONGLONG tEnd = GetTickCount64();

	LOG_DEBUG(L"Protect with dependencies report:");
	LOG_DEBUG(L"\t- Files: " << protectedFilesLog.c_str());
	LOG_DEBUG_FMT(L"\t- Time elapsed: %llu ms", tEnd - tStart);

	if (!OtkX_EraseModel(pCurMdl, true))
	{
		// Record the failure for future blocking
		OtkXSessionCacheHolder().AddDirtyNxlToReload(xCurMdl.GetId());
		OtkX_ShowMessageDialog(IDS_ERROR_CANNOT_ERASE_AFTER_PROTECT, pfcMESSAGE_ERROR);
		return;
	}
	OtkX_ReopenFile(xCurMdl.GetOrigin());
}

bool CProtectController::ExecuteJob(ProtectJobPtr pJob)
{
	if (pJob == nullptr)
		return false;

	RevertBackupFile(pJob);

	bool ret = pJob->Execute();
	if (ret && pJob->GetSourceActionType() != SOURCEACTION_EXPORT)
	{
		pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pJob->GetModelDescr());
		if (pMdl == nullptr)
			return ret;

		
		// Update the cache in session in case the new file is loaded without AfterModelRetrieved API triggered.
		// e.g.: Save file will automatically redirect the new file path to existing model object in session, without retrieve model again.
		COtkXModel xMdl(pMdl);
		if (wcsicmp(xMdl.GetOrigin().c_str(), pJob->GetFileDest().c_str()) == 0)
		{
			OtkXSessionCacheHolder().AddNxlModel(xMdl.GetId(), pJob->GetFileDest(), false);
			// Copy cached tags for new file path
			// Note: Don't save tags by pJob->GetTags, due to it may contains combined tags for drawing and assembly.
			OtkXSessionCacheHolder().SaveTags(pJob->GetFileDest(), OtkXSessionCacheHolder().GetTags(pJob->GetFileOrigin()));
		}
	}

	return ret;
}

bool CProtectController::NeedToProtect(pfcModel_ptr pMdl)
{
	COtkXModel xModel(pMdl);
	return OtkXSessionCacheHolder().IsNxlModel(xModel);
}

/*
This function is used to report all files whose changes are discarded by RMX, 
due to no edit permission. 
All records will be cleaned after report gets generated.
*/
void CProtectController::ShowEditDenyDialog()
{
	if (!m_editDenyFiles.empty())
	{
		xstring msg(IDS_ERR_DENY_DISCARD_CHANGE);
		xstring fileInfo;
		for (const auto& file : m_editDenyFiles)
		{
			fileInfo = xstring::Printf("\n- %s", RMXUtils::ws2s(file).c_str());
			msg += fileInfo;
		}
		m_editDenyFiles.clear();
		for (const auto& file : m_backupFiles)
		{
			RMX_DeleteNxlFile(file.second.c_str());
		}
		m_backupFiles.clear();
		OtkX_ShowMessageDialog(msg, pfcMESSAGE_WARNING);
	}
}

void CProtectController::TryToBackupFile(const wstring& fileOrigin)
{
	// Usage control for ipem will be done by ipemrmx differently
	if (IsWatcherRunning(PROTECT_WATCHER_SAVE_TC))
		return;

	// HACK.There's no way to prevent save action of dependencies when saving the assembly.
	// Try to backup the old dependency model and revert the saved new one back after finishing saving.
	if (RMX_CheckRights(fileOrigin.c_str(), RMX_RIGHT_EDIT, true))
		return;

	CPathEx origFile(fileOrigin.c_str());
	CPathEx backupFile(origFile.GetParentPath().c_str());
	backupFile /= RMXUtils::GenerateUUID().c_str();
	backupFile += L"_";
	backupFile += origFile.GetFileName().c_str();
	backupFile += origFile.GetExtension().c_str();
	backupFile += NXL_FILE_EXT;
	if (RMX_CopyNxlFile(fileOrigin.c_str(), backupFile.c_str(), false))
	{
		LOG_INFO_FMT(L"Backup file created: '%s'", backupFile.c_str());
		auto oldBackupIter = m_backupFiles.find(fileOrigin);
		if (oldBackupIter != m_backupFiles.end())
		{
			RMX_DeleteNxlFile((*oldBackupIter).second.c_str());
		}

		m_backupFiles[fileOrigin] = wstring(backupFile.c_str());

		const wstring& imageFile = CAuxiImageFileProtectJob::BuildImageFilePath(fileOrigin, m_imageExtOnSave);
		if (CPathEx::IsFile(imageFile))
		{
			// Backup auxiliary file
			CPathEx imageFileParser(imageFile.c_str());
			backupFile = imageFileParser.GetParentPath().c_str();
			backupFile /= RMXUtils::GenerateUUID().c_str();
			backupFile += L"_";
			backupFile += imageFileParser.GetFileName().c_str();
			backupFile += imageFileParser.GetExtension().c_str();
			if (RMX_CopyNxlFile(imageFile.c_str(), backupFile.c_str(), false))
			{
				LOG_INFO_FMT(L"Backup file created: '%s'", backupFile.c_str());
				oldBackupIter = m_backupFiles.find(imageFile);
				if (oldBackupIter != m_backupFiles.end())
				{
					RMX_DeleteNxlFile((*oldBackupIter).second.c_str());
				}
				m_backupFiles[imageFile] = wstring(backupFile.c_str());
			}
		}
	}
	else
	{
		LOG_ERROR_FMT(L"Failed to backup file: '%s'", fileOrigin.c_str());
	}
}

wstring CProtectController::GetNxlOrigin(pfcModel_ptr pMdl)
{
	COtkXModel xMdl(pMdl);
	return OtkXSessionCacheHolder().FindNxlOrigin(xMdl);
}

void CProtectController::RevertBackupFile(ProtectJobPtr pJob)
{
	if (pJob == nullptr)
		return;

	// HACK. Discard the modification on the nxl files which do not have 'EDIT' rights, by
	// replacing original backup.
	// There's no way to prevent save action for the dependency models respectively when saving assembly.
	auto backupFileIter = m_backupFiles.find(pJob->GetFileOrigin());
	bool saveToSameFile = false;
	if (backupFileIter != m_backupFiles.end())
	{
		// Only prevent save action if the target file is same to origin. If the target file is new copy, allow the aciton w/o edit permisson.
		OtkXFileNameData originNameData(pJob->GetFileOrigin());
		OtkXFileNameData destNameData(pJob->GetFileDest());
		if (originNameData.IsSameVersionFile(destNameData))
		{
			saveToSameFile = true;
			if (RMX_CopyNxlFile((*backupFileIter).second.c_str(), pJob->GetFileDest().c_str(), true))
			{
				LOG_WARN_FMT(L"No 'EDIT' permission found. The modifications will be discarded on the saved file: '%s'", pJob->GetFileDest().c_str());
				wstring reportMsg(pJob->GetFileDest());
				reportMsg += L" (Model: " + pJob->GetFileName() + L")";
				m_editDenyFiles.insert(reportMsg);
			}
			else
			{
				LOG_ERROR_FMT(L"No 'EDIT' permission found, but file is modified: '%s'", pJob->GetFileDest().c_str());
			}
		}
		
		m_backupFiles.erase(backupFileIter);

		// Revert the auxiliary image file as well
		bool imageFileReverted = false;
		const wstring& imageFilePath = CAuxiImageFileProtectJob::BuildImageFilePath(pJob->GetFileOrigin(), m_imageExtOnSave);
		backupFileIter = m_backupFiles.find(imageFilePath);
		if (backupFileIter != m_backupFiles.end())
		{
			if (saveToSameFile && RMX_CopyNxlFile((*backupFileIter).second.c_str(), imageFilePath.c_str(), true))
			{
				LOG_WARN_FMT(L"No 'EDIT' permission found. The modifications on auxiliary file will be discarded: '%s'", imageFilePath.c_str());
				imageFileReverted = true;
			}
			
			m_backupFiles.erase(backupFileIter);
		}

		// Do not allow to have mismatched image file with backup model file
		if (!imageFileReverted && CPathEx::IsFile(imageFilePath))
			RMX_DeleteNxlFile(imageFilePath.c_str());
	}
}

void CProtectController::WatchFile(const std::wstring & newFile)
{
	// Monitor the file in case it's relevant to certain action
	if (IsWatchingExport())
		m_watchedFilesForExport.insert(newFile);
	if (IsWatchingCopy())
		m_watchedFilesForCopy.insert(newFile);
	if (IsWatchingSave())
		m_watchedFilesForSave.insert(newFile);
}

void CProtectController::AddJTCADFile(pfcModel_ptr pMdl)
{
	COtkXModel xMdl(pMdl);
	if (!xMdl.IsValid()) return;

	if (m_JTCADFiles.find(xMdl.GetOrigin()) == m_JTCADFiles.end())
	{
		COtkXCacheModel cacheMdl(pMdl);
		m_JTCADFiles[cacheMdl.GetOrigin()] = cacheMdl;
	}
}

void CProtectController::GetExportTagsForModel(pfcModel_ptr pMdl, string & tags, bool & needProtect)
{
	COtkXModel xMdl(pMdl);
	needProtect = NeedToProtect(pMdl);
	if (!needProtect && xMdl.IsProtected())
	{
		// Just in case the tag has not been cached.
		OtkXSessionCacheHolder().AddNxlModel(xMdl.GetId(), xMdl.GetOrigin());
		//CProtectController::GetInstance().AddNxlParam(pMdl, pMdl->GetOrigin());
		needProtect = true;
	}
		
	if (needProtect)
	{
		tags = OtkXSessionCacheHolder().GetTags(xMdl.GetOrigin());
	}
}


void CProtectController::GetExportTagsFromDepedencies(pfcModel_ptr pMdl, string& tags, bool& needProtect)
{
	string selfTags;
	bool selfNeedProtect = false;
	GetExportTagsForModel(pMdl, selfTags, selfNeedProtect);
	// Protect the root file once any child dependency is protected
	if (!needProtect && selfNeedProtect)
		needProtect = true;

	tags += selfTags;
	
	COtkXModel xMdl(pMdl);
	xMdl.TraverseDependencies([&](COtkXModel& depXMdl) -> bool {
		if (depXMdl.IsValid())
		{
			string selfTags;
			bool selfNeedProtect = false;
			CProtectController::GetInstance().GetExportTagsForModel(depXMdl.GetModel(), selfTags, selfNeedProtect);
			// Protect the root file once any child dependency is protected
			if (!needProtect && selfNeedProtect)
				needProtect = true;
			tags += selfTags;		
		}
		return false;

	});
}

wstring CProtectController::GetFileWatcherNames(DWORD types)
{
	if (types == 0)
		return wstring(L"none");

	static std::map<DWORD, wstring> s_supportedWatchers = {
		{ PROTECT_WATCHER_COPY, L"PROTECT_WATCHER_COPY"},
		{ PROTECT_WATCHER_SAVE, L"PROTECT_WATCHER_SAVE" },
		{ PROTECT_WATCHER_EXPORT, L"PROTECT_WATCHER_EXPORT" },
		{ PROTECT_WATCHER_EXPORT_JTBATCH, L"PROTECT_WATCHER_EXPORT_JTBATCH" },
		{ PROTECT_WATCHER_EXPORT_JTCURRENT, L"PROTECT_WATCHER_EXPORT_JTCURRENT" },
		{ PROTECT_WATCHER_BACKUP, L"PROTECT_WATCHER_BACKUP" },
		{ PROTECT_WATCHER_SAVE_TC, L"PROTECT_WATCHER_SAVE_TC" },
		{ PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE, L"PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE" },
		{ PROTECT_WATCHER_SHRINKWRAP, L"PROTECT_WATCHER_SHRINKWRAP" },
		{ PROTECT_WATCHER_TC_CACHEMANAGER, L"PROTECT_WATCHER_TC_CACHEMANAGER" },
		{ PROTECT_WATCHER_TC, L"PROTECT_WATCHER_TC" }
	};
	wstring watchers;
	for (const auto& w : s_supportedWatchers)
	{
		if (HAS_BIT(types, w.first))
		{
			watchers = watchers + w.second + L"|";
		}
	}
	
	if (!watchers.empty())
		watchers.pop_back();

	return watchers;
}

void CProtectController::ParseJTCommandFiles()
{
	for (const auto& cmdFile : m_JTBatchCommandFiles)
	{
		// Command file is a text file. Formats for each line as below: 
		// 1. <cadFile path> <translate configFile>
		// 2. <cadFile path>
		// 3. #<comments>
		bool isCmdFile = false;
		wifstream  ifs(cmdFile);
		wstring line;
		while (ifs.good() && std::getline(ifs, line)) {

			if (line.empty() || line.at(0) == L'#')
				continue;
			
			RMXUtils::ToLower<wchar_t>(line);
			auto pos = line.find(L".asm");
			if ((pos != wstring::npos) || ((pos = line.find(L".prt")) != wstring::npos))
			{
				auto separatorPos = line.find_first_of(L' ', pos + 4);
				if (separatorPos != wstring::npos)
				{
					if (!isCmdFile)
					{
						LOG_DEBUG_FMT(L"EAI Batch command file: %s", cmdFile.c_str());
						isCmdFile = true;
					}
					const wstring& prevLine = line.substr(0, separatorPos);
					//AddJTCADFile(prevLine);
					LOG_DEBUG(L"\t" << line.c_str());
				}
			}
		}
	}
}

void CProtectController::TakeSnapshotForMotionEnvlp()
{
	m_pSrcModelForMotionEnvlp = OtkX_GetCurrentModel();
}

void CProtectController::GetExportTags(pfcModel_ptr pMdl, bool recursive, string& tags, bool& needProtect)
{
	if (recursive)
	{
		COtkXModel xMdl(pMdl);
		const wstring& fileOrigin = OtkXSessionCacheHolder().FindNxlOrigin(xMdl);
		// Improve the performance with cache
		if (!fileOrigin.empty() && (m_exportDependencyTagCache.find(fileOrigin.c_str()) != m_exportDependencyTagCache.end()))
		{
			tags = m_exportDependencyTagCache[fileOrigin.c_str()];
			needProtect = true;
		}
		else
		{
			GetExportTagsFromDepedencies(pMdl, tags, needProtect);
		}
	}
	else
	{
		GetExportTagsForModel(pMdl, tags, needProtect);
	}
}

void CProtectController::StartFileWatcher(DWORD watcherType)
{
	LOG_INFO_FMT(L"StartFileWatcher : %s", GetFileWatcherNames(watcherType).c_str());
	if ((m_runningFileWatches & watcherType) != watcherType)
		m_runningFileWatches |= watcherType;

	if (IsExportWatcher(watcherType))
	{
		m_watchedFilesForExport.clear();
		m_exportDependencyTagCache.clear();
		m_JTCADFiles.clear();
		m_exportDenyFiles.clear();
		m_pSrcModelForMotionEnvlp = nullptr;

		// Since 5.4
		if (IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTCURRENT))
		{
			CustomizeUsageControl::Start();
		}
		else if (IsWatcherRunning(PROTECT_WATCHER_EXPORT))
		{
			HookInitializer::InstallHook_Export();
		}

		//Special case  to export motion envlp as part. 
		// Current model in the session will be switchted to newly generated part file
		// So cannot retreive original source assembly to determine tags when protection.
		if (IsWatcherRunning(PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE))
		{
			TakeSnapshotForMotionEnvlp();
		}
	}
	if (IsCopyWatcher(watcherType))
	{
		m_watchedFilesForCopy.clear();
	}
	if (IsSaveWatcher(watcherType))
	{
		m_watchedFilesForSave.clear();
		// TC special: When save_bitmap_type is different from jpg, jpg cannot be handled by CAuxiImageFileProtectJob. 
		// They will generated and deleted by ipem after checking-in. Need to handle them immediately after creation.
		if (IsWatcherRunning(PROTECT_WATCHER_SAVE_TC) && JPGGeneratedByIpem())
		{
			FileClosedHooks::Install_IpemJPG();
		}
	}
	if (IsWatcherRunning(PROTECT_WATCHER_TC))
	{
		IpemRMX::RefreshCurrentRegistryFile();
	}
}

void CProtectController::StopFileWatcher(DWORD watcherType)
{
	LOG_INFO_FMT(L"StopFileWatcher : %s", GetFileWatcherNames(watcherType).c_str());
	if (m_runningFileWatches == 0)
		return;

	if ((m_runningFileWatches & watcherType) != watcherType)
		return;

	// Since 5.4
	if (HAS_BIT(watcherType, PROTECT_WATCHER_EXPORT_JTCURRENT))
		CustomizeUsageControl::Stop();

	if (HAS_BIT(watcherType, PROTECT_WATCHER_SAVE_TC))
		FileClosedHooks::Uninstall_IpemJPG();

	if (IsExportWatcher(watcherType))
		HookInitializer::UnstallHook_Export();

	if (HAS_BIT(watcherType, PROTECT_WATCHER_TC_CACHEMANAGER))
	{		
		IpemRMX::EnsureNxlExtInWorkspace();
	}

	ProcessWatchedFiles(watcherType);

	m_pSrcModelForMotionEnvlp = nullptr;

	m_runningFileWatches &= ~watcherType;
}

void CProtectController::RemoveFileWatcher(DWORD watcherType)
{
	LOG_INFO_FMT(L"RemoveFileWatcher : %s", GetFileWatcherNames(watcherType).c_str());
	if (IsExportWatcher(watcherType))
	{
		m_watchedFilesForExport.clear();
		m_exportDependencyTagCache.clear();
		m_runningFileWatches &= PROTECT_WATCHER_EXPORT;
	}
	if (IsCopyWatcher(watcherType))
	{
		m_watchedFilesForCopy.clear();
	}
	if (IsSaveWatcher(watcherType))
	{
		m_watchedFilesForSave.clear();
	}
	m_runningFileWatches &= ~watcherType;
}

void CProtectController::ProcessFile(const wstring& filePath, DWORD watcherType)
{
	PC_LOG_ENTER
	RMX_SCOPE_GUARD_ENTER(&m_isInProcess);
	if (HAS_BIT(watcherType, PROTECT_WATCHER_SAVE_TC))
	{
		// TC special: When save_bitmap_type is different from jpg, jpg cannot be handled by CAuxiImageFileProtectJob. 
		// They will generated and deleted by ipem after checking-in. Need to handle them immediately after creation.
		// TODO: Support other auxiliary file type according to the ipem.xml settings, like pdf
		if (!JPGGeneratedByIpem())
			return;

		LOG_INFO(L"ProtectControl triggered to process new ipem preview(.jpg) files...");
		auto pJPGAuxiJob = CTCAuxiFileProtectJob::Create(filePath);
		if (pJPGAuxiJob != nullptr)
		{
			pJPGAuxiJob->Execute();
		}
	}
	else if (IsExportWatcher(watcherType))
	{
		LOG_INFO_FMT(L"Processing export file: '%s'", filePath.c_str());

		if (!CPathEx::IsFile(filePath))
		{
			LOG_INFO_FMT(L"Skip protection control. File not found: '%s'", filePath.c_str());
			return;
		}

		auto pMdl = OtkX_GetCurrentModel();
		if (pMdl == nullptr)
		{
			LOG_ERROR(L"Ignored. Current model not found");
			return;
		}

		auto pJob = CExportProtectJob::Create(pMdl->GetDescr(), filePath);
		if (ExecuteJob(pJob))
		{
			m_watchedFilesForExport.erase(filePath);
			if (IsWatcherRunning(PROTECT_WATCHER_EXPORT_DWGPUBPREVIEW))
			{
				// Special handling to mark rpm folder for dwg preview
				// to ensure newly preview file can be opened
				OtkXFileNameData destNameData(pJob->GetFileDest());
				CRPMDirectoryMgr::GetInstance().AddRPMDir(destNameData.GetDirectoryPath());
			}
		}
	}
}

void CProtectController::ProcessWatchedFiles(DWORD watcherType)
{
	PC_LOG_ENTER
	RMX_SCOPE_GUARD_ENTER(&m_isInProcess);

	if (IsExportWatcher(watcherType))
	{
		if (m_watchedFilesForExport.empty())
			return;

		LOG_INFO(L"ProtectControl triggered after exporting model...");
		auto pMdl = OtkX_GetCurrentModel();
		while (m_watchedFilesForExport.size() > 0)
		{
			auto exportFileIter = m_watchedFilesForExport.cbegin();
			wstring exportFilePath(*exportFileIter);
			LOG_INFO_FMT(L"Processing export file: '%s'", exportFilePath.c_str());

			OtkXFileNameData fNameData(exportFilePath);
			// Some of export files will be initially created with XXX__temp.[extension] file name.
			// And then renamed to target file name without "__temp" suffix. 
			// The watcher only can monitor the file creation(CreateFile Hook). 
			// Fix the file name manually so that protection job can find out target file.			
			if (m_fileTypesToFixName.find(fNameData.GetExtension().c_str()) != m_fileTypesToFixName.end())
			{
				static wstring suffixToRemove(L"__temp");
				static size_t lenToRemove = suffixToRemove.length();
				size_t nPos = exportFilePath.rfind(suffixToRemove + L"." + fNameData.GetExtension());
				if (nPos != std::wstring::npos)
				{
					exportFilePath.replace(nPos, lenToRemove, L"");
					LOG_INFO_FMT(L"Fix export file name to '%s'", exportFilePath.c_str());
				}
			}

			if (!CPathEx::IsFile(exportFilePath))
			{
				LOG_INFO_FMT(L"Skip protection control. File not found: '%s'", exportFilePath.c_str());
				m_watchedFilesForExport.erase(exportFileIter);
				continue;
			}

			// Process JT exported file
			if (wcsicmp(fNameData.GetExtension().c_str(), L"jt") == 0)
			{
				if (ProcessJTExportFile(exportFilePath) ||
					ProcessJTExportModel(pMdl, exportFilePath))
				{
					m_watchedFilesForExport.erase(exportFileIter);
					continue;
				}
			}

			ProtectJobPtr pJob = nullptr;
			if (IsWatcherRunning(PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE) &&
				m_pSrcModelForMotionEnvlp != nullptr)
			{
				pJob = CMotionEnvlpProtectJob::Create(m_pSrcModelForMotionEnvlp->GetDescr(), exportFilePath);
			}
			else if (pMdl != nullptr)
			{
				pJob = CExportProtectJob::Create(pMdl->GetDescr(), exportFilePath);
			}		
			if (ExecuteJob(pJob))
			{
				if (IsWatcherRunning(PROTECT_WATCHER_EXPORT_DWGPUBPREVIEW))
				{
					// Special handling to mark rpm folder for dwg preview
					// to ensure newly preview file can be opened
					OtkXFileNameData destNameData(pJob->GetFileDest());
					CRPMDirectoryMgr::GetInstance().AddRPMDir(destNameData.GetDirectoryPath());
				}
			}

			m_watchedFilesForExport.erase(exportFileIter);

			const wstring& imageFilePath = CAuxiImageFileProtectJob::BuildImageFilePath(exportFilePath, m_imageExtOnSave);
			auto pAuxiImageJob = CAuxiImageFileProtectJob::Create(pJob, imageFilePath);
			if (pAuxiImageJob != nullptr)
			{
				pAuxiImageJob->Execute();
			}

			// Skip image file generated as auxiliary file.
			// It will be protected after the main file gets protected.
			if (!imageFilePath.empty())
			{
				m_watchedFilesForExport.erase(imageFilePath);
			}
		}

		// For JT export/playback->create motion envelop, if the cad file has not rights to export, automatically delete newly
		// exported file, and prompt to user.
		if (!m_exportDenyFiles.empty() && (IsWatchingJTExport() || IsWatcherRunning(PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE)))
		{
			xstring msg(IDS_ERR_DENY_DELETE_EXPORTFILE);
			xstring fileInfo;		
			
			// Generate the error report for below types:
			// 1. File is lost in disk
			// 2. No export right
			// 3. No edit right but file gets modified
			auto range = m_exportDenyFiles.equal_range(RMX_RIGHT_NONE);
			auto handleError = [&]() -> void {
				for (auto i = range.first; i != range.second; ++i)
				{
					for (const auto& r : i->second)
					{
						// Populate error message for source file
						// And delete the exported files generated based on this source file.
						fileInfo = xstring::Printf("\n- %s", RMXUtils::ws2s(r.first).c_str());
						msg += fileInfo;
						for (const auto& f : r.second)
						{
							if (CPathEx::IsFile(f) && !CPathEx::RemoveFile(f))
							{
								LOG_ERROR_FMT(L"Cannot delete file: %s", f.c_str());
							}
							const wstring& imageFilePath = CAuxiImageFileProtectJob::BuildImageFilePath(f, m_imageExtOnSave);
							if (CPathEx::IsFile(imageFilePath) && !CPathEx::RemoveFile(imageFilePath))
								LOG_ERROR_FMT(L"Cannot delete file: %s", imageFilePath.c_str());
						}
					}
				}
			};
			
			handleError();

			range = m_exportDenyFiles.equal_range(RMX_RIGHT_EXPORT);
			handleError();

			range = m_exportDenyFiles.equal_range(RMX_RIGHT_EDIT);
			handleError();

			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
		}
	}
	//if (IsSaveWatcher(watcherType))
	//{
	//	// TC special: When save_bitmap_type is different from jpg, jpg cannot be handled by CAuxiImageFileProtectJob. 
	//	// They will generated and deleted by ipem after checking-in. Need to handle them immediately after creation.
	//	// TODO: Support other auxiliary file type according to the ipem.xml settings, like pdf
	//	if (m_watchedFilesForSave.empty() || m_imageExtOnSave.compare(L"jpg") == 0)
	//		return;

	//	LOG_INFO(L"ProtectControl triggered to process new preview(.jpg) files...");
	//	for(const auto& watchedFile : m_watchedFilesForSave)
	//	{	
	//		LOG_INFO_FMT(L"Processing '%s'...", watchedFile.c_str());
	//		auto pJPGAuxiJob = CTCAuxiFileProtectJob::Create(watchedFile);
	//		if (pJPGAuxiJob != nullptr)
	//		{
	//			pJPGAuxiJob->Execute();
	//			m_watchedFilesForSave.erase(watchedFile);
	//			break;
	//		}
	//	}
	//}
}

/*!
 * Attempt to protect the jt file according the the certain CAD file
 * 
 * \param jtFilePath
 * \return True if succeeded
 */
bool CProtectController::ProcessJTExportFile(const wstring& jtFilePath)
{
	if (!IsWatchingJTExport())
		return false;

	//ParseJTCommandFiles();

	COtkXCacheModel srcFile;
	auto findCADFileByName = [&](const wstring& searchName) -> bool {
		LOG_DEBUG(L"Search CAD file containing name: " << searchName.c_str());
		for (const auto& cadFile : m_JTCADFiles)
		{
			OtkXFileNameData cadFNameData(cadFile.first);
			if (wcsicmp(searchName.c_str(), cadFNameData.GetBaseName().c_str()) != 0)
				continue;

			LOG_INFO_FMT(L"CAD file found for JT file:\n\t- Source: '%s'\n\t- Export: '%s'",
				cadFile.first.c_str(), jtFilePath.c_str());
			srcFile = cadFile.second;

			return true;
		}

		return false;
	};

	auto findModelInSession = [&](const string& searchName) -> bool {
		try
		{
			LOG_DEBUG(L"Search model in session containing name: " << searchName.c_str());
			auto pSess = pfcGetProESession();
			auto pSrcMdl = pSess->GetModel(searchName.c_str(), pfcMDL_PART);
			if (pSrcMdl == nullptr)
				pSrcMdl = pSess->GetModel(searchName.c_str(), pfcMDL_ASSEMBLY);

			if (pSrcMdl != nullptr)
			{
				srcFile = COtkXCacheModel(pSrcMdl);
				LOG_INFO_FMT(L"CAD file found for JT file:\n\t- Source: '%s'\n\t- Export: '%s'",
					srcFile.GetOrigin().c_str(), jtFilePath.c_str());

				return true;
			}
		}
		
		OTKX_EXCEPTION_HANDLER();
		
		return false;
	};

	// Algorithm to find out the CAD file according the jt file name.
	// Case1: File base name is same
	// Case2: Treat underscore as dash
	// Case3: JT file name contains CAD extension suffix like _prt/asm
	// Case4: JT file with _Quilt suffix.
	// TODO: Enhance to support more name patterns. (ipem allows to configure any name pattern).
	static wchar_t* jtNamePatterns[] = { L"(.*)(.jt)", L"(.*)(_(asm|prt))(.jt)", L"(.*)(_Quilt)(.jt)", L"(.*)(_(asm_Quilt|prt_Quilt))(.jt)" };
	OtkXFileNameData jtFNameData(jtFilePath);
	const wstring& jtFileName = jtFNameData.GetFileName();
	for (const auto& p : jtNamePatterns)
	{
		std::wsmatch mResults;
		if (std::regex_match(jtFileName, mResults, std::wregex(p, std::regex::icase)) && mResults.size() > 2)
		{
			wstring jtSearchName = mResults[1].str();
			if (findCADFileByName(jtSearchName))
				break;

			if (jtSearchName.find(L'_') != wstring::npos)
			{
				std::replace(jtSearchName.begin(), jtSearchName.end(), L'_', L'-');
				if (findCADFileByName(jtSearchName))
					break;
			}

			// In some case, the model has been opened in session, CAD file not opened again, which is 
			// not collected in m_JTCADFiles. Find out from memory.
			string mdlName = RMXUtils::ws2s(mResults[1].str());
			if (findModelInSession(mdlName))
				break;

			if (mdlName.find(L'_') != string::npos)
			{
				std::replace(mdlName.begin(), mdlName.end(), '_', '-');
				if (findModelInSession(mdlName))
					break;
			}
		}
	}

	if (!srcFile.IsValid())
	{
		LOG_ERROR(L"Cannot find CAD file for the exported jt: " << jtFilePath.c_str());
		return false;
	}

	bool needToProtect = false;
	// Special case for IPEM jt disptach. nxl file is decrypted on same folder. Look for original nxl file to determine
	// if jt needs to be protected.
	if (IpemRMX::JTDispRunning() && PathIsNetworkPath(srcFile.GetOrigin().c_str())) {
		LOG_DEBUG(L"Process network nxl file...");
		string tags;

		auto isNxlProtected = [&](const wstring& fileName) -> bool {
			OtkXFileNameData nameData(fileName);
			if (wcsicmp(nameData.GetExtension().c_str(), NXL_FILE_EXT_WITHOUT_DOT) != 0)
			{
				wstring nxlFilePath = fileName + NXL_FILE_EXT;
				if (CPathEx::IsFile(nxlFilePath))
				{
					if (RMX_IsProtectedFile(nxlFilePath.c_str()))
					{
						char* szTags = nullptr;
						RMX_GetTags(nxlFilePath.c_str(), &szTags);
						if (szTags != nullptr)
						{
							tags = szTags;
							RMX_ReleaseArray((void*)szTags);
						}
						return true;
					}
				}
			}

			return false;
		};

		auto isDepProtected = [&](const COtkXCacheModel& xMdl) -> bool {
			auto deps = xMdl.GetDeps();
			bool anyProtected = false;
			for (const auto& dep : deps) {
				if (isNxlProtected(dep))
				{
					anyProtected = true;
				}
			}

			return anyProtected;
		};

		if (isNxlProtected(srcFile.GetOrigin()) || isDepProtected(srcFile))
		{
			needToProtect = true;
			CPathEx jtPath(jtFilePath.c_str());
			wstring tempDir = RMXUtils::getEnv(L"LOCALAPPDATA");
			if (!tempDir.empty())
			{
				wstring jtFileName = jtPath.GetFileName() + jtPath.GetExtension();
				CPathEx tempPath(IpemRMX::GetTempFolder().c_str());
				if (!tempPath.ToString().empty())
				{
					// Due to RPM does not support to protect network file, temporarily copy to local and protect. 
					// Then copy back to shared folder. 			
					tempPath /= jtFileName.c_str();
					auto copyJTFile = [&](const wstring& existFile, const wstring& newFile) -> bool {
						if (CopyFile(existFile.c_str(), newFile.c_str(), FALSE) == 0)
						{
							LOG_ERROR_FMT(L"Failed to copy file from '%s' to '%s' (error: %s)", existFile.c_str(), newFile.c_str(), CSysErrorMessage(GetLastError()));
							return false;
						}
						else
						{
							LOG_DEBUG_FMT(L"File copied from '%s' to '%s'", existFile.c_str(), newFile.c_str());
						}
						return true;
					};

					if (copyJTFile(jtFilePath.c_str(), tempPath.c_str()))
					{
						auto pJob = std::make_shared<CJTBatchProtectJob>();
						pJob->SetFileDest(tempPath.ToString());
						pJob->SetFileOrigin(srcFile.GetOrigin());
						pJob->SetTags(tags);
						if (pJob != nullptr && pJob->Execute())
						{
							// Move nxl file to results stage folder 
							tempPath += NXL_FILE_EXT;
							jtPath += NXL_FILE_EXT;
							// Copy from RPM via RMX_CopyNxlFile
							RMX_CopyNxlFile(tempPath.c_str(), jtPath.c_str(), true);
							// Remove nxl extension, so that dispatch can recgonize and handle properly
							CPathEx::RenameFile(jtPath.ToString(), jtFilePath);
						}
					}
				}
				
			}
			
			return true;
		}
	}
	else if (srcFile.IsProtected() || srcFile.IsDepProtected())
	{
		needToProtect = true;
		
	}
	if (!needToProtect)
	{
		LOG_INFO_FMT(L"Skip protection control. Model does not contain any protected object:\n\t- Source: '%s'\n\t- Export: '%s'",
			srcFile.GetOrigin().c_str(), jtFilePath.c_str());
		return false;
	}

	// 1. Usage control for JT Current export is done differently in CustomizeUsageControl already
	// 2. Usage control for Ipem JT batch export is done in ipemrmx
	if (!IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTCURRENT) && !IsWatcherRunning(PROTECT_WATCHER_TC))
	{
		std::set<wstring> denyFiles;
		if (!srcFile.CheckRights(RMX_RIGHT_EXPORT) || !srcFile.CheckDepRights(RMX_RIGHT_EXPORT, denyFiles))
		{
			LOG_ERROR_FMT(L"Operation Denied. You do not have 'Save As' permission to export the following file(s). Deleting generated JT.",
				jtFilePath.c_str());
			if (denyFiles.size() == 0)
			{
				LOG_ERROR_FMT(L"\t%s", srcFile.GetOrigin().c_str());
				CProtectController::GetInstance().NotifyActionDeny(RMX_RIGHT_EXPORT, srcFile.GetOrigin(), jtFilePath);
			}
			else
			{
				for (const auto& denyFile : denyFiles)
				{
					LOG_ERROR_FMT(L"\t%s", denyFile.c_str());
					CProtectController::GetInstance().NotifyActionDeny(RMX_RIGHT_EXPORT, denyFile.c_str(), jtFilePath);
				}
			}

			if (!CPathEx::RemoveFile(jtFilePath))
			{
				LOG_ERROR_FMT(L"Cannot delete JT file: %s", jtFilePath.c_str());
			}
			else
			{
				LOG_INFO_FMT(L"JT file deleted. %s", jtFilePath.c_str());
			}
			return true; //Handled as deny
		}
	}

	ProtectJobPtr pJob = CJTBatchProtectJob::Create(srcFile, jtFilePath);
	if (pJob != nullptr)
	{
		pJob->Execute();
	}
	
	return true;
}

/*!
 * Attempt to protect jt file according to the specified model
 * 
 * \param pMdl
 * \param jtFilePath
 * \return True if succeeded
 */
bool CProtectController::ProcessJTExportModel(pfcModel_ptr pMdl, const std::wstring& jtFilePath)
{
	if (pMdl == nullptr ||
		!IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTCURRENT))
		return false;

	auto pJob = CJTCurrentProtectJob::Create(pMdl->GetDescr(), jtFilePath);
	if (pJob != nullptr)
	{
		pJob->Execute();
	}

	return true;	
}

void CProtectController::NotifyActionDeny(DWORD dwRights, const std::wstring& srcFilePath, const std::wstring& targetFilePath)
{
	// TODO: For copy, edit?
	//if (HAS_BIT(dwRights, RMX_RIGHT_EXPORT))
	auto range = m_exportDenyFiles.equal_range(dwRights);
	for (auto i = range.first; i != range.second; ++i)
	{
		if (i->second.find(srcFilePath) != i->second.end())
		{
			i->second[srcFilePath].insert(targetFilePath);
			return;
		}
	}
	TExportDenyResult result;
	result[srcFilePath].insert(targetFilePath);
	m_exportDenyFiles.insert(std::make_pair(dwRights, result));
}

// TODO: Solution to be completed
//void CProtectController::ProtectDrawingPreviewFile(const wstring & imageFileName)
//{
//	pfcModel_ptr pMdl = OtkX_GetCurrentModel();
//	if (pMdl->GetType() != pfcMDL_DRAWING || !RMX_IsProtectedFile(CXS2W(pMdl->GetOrigin())))
//		return;
//
//	CPathEx imageFilePath(imageFileName.c_str());
//	wstring drwFileName = imageFilePath.GetFileName();
//	// Remove suffix "d"
//	drwFileName = drwFileName.erase(drwFileName.size() -1);
//	
//	wstring mdlName = CXS2W(pMdl->GetInstanceName());
//	if (wcsicmp(mdlName.c_str(), drwFileName.c_str()) == 0)
//	{
//		// File pattern is [ModelInstanceName]d.[bitmapExt]. e.g.: test.drw -> testd.jpg
//		wstring mdlOrigin = CXS2W(pMdl->GetOrigin());
//		CPathEx mdlPathEx(mdlOrigin.c_str());
//		CPathEx imageFilePathEx(mdlPathEx.GetParentPath().c_str());
//		imageFilePathEx /= imageFileName.c_str();
//
//		if (CPathEx::IsFile(imageFilePathEx.c_str()) && !RMX_IsProtectedFile(imageFilePathEx.c_str()))
//		{
//			auto pJob = std::make_shared<CProtectJob>(pMdl->GetDescr());
//			pJob->SetFileOrigin(mdlOrigin);
//			pJob->SetFileDest(imageFilePathEx.ToString());
//			pJob->SetTags(ReadTags(mdlOrigin));
//			pJob->Dump();
//			pJob->Execute();
//		}
//	}
//}

void COtkXSessionActionListener::OnBeforeModelCopy(pfcDescriptorContainer2_ptr pContainer)
{
	PC_LOG_ENTER
	
	CProtectController::GetInstance().StartFileWatcher(PROTECT_WATCHER_COPY);
}
