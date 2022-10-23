#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>
#include <windows.h>

#include "..\common\CommonTypes.h"
#include "OtkXModel.h"
#include "OtkXTypes.h"

#include <pfcModel.h>
#include <wfcSession.h>

#include "IpemRMX.h"

class CProtectJob;
typedef std::shared_ptr<CProtectJob> ProtectJobPtr;

//*****************************************************************************
class COtkXModelActionListener : public pfcDefaultModelActionListener
{
public:
	void OnAfterModelSaveAll(pfcModelDescriptor_ptr pDescr);
	void OnAfterModelRetrieveAll(pfcModelDescriptor_ptr pDescr);
	void OnAfterModelEraseAll(pfcModelDescriptor_ptr pDescr);
};

//*****************************************************************************
class COtkXModelEventActionListener : public pfcDefaultModelEventActionListener
{
public:
	void OnAfterModelCopy(pfcModelDescriptor_ptr pFromDescr, pfcModelDescriptor_ptr pToDescr);
	void OnAfterModelCopyAll(pfcModelDescriptor_ptr pFromDescr, pfcModelDescriptor_ptr pToDescr);
	void OnAfterModelRename(pfcModelDescriptor_ptr pFromDescr, pfcModelDescriptor_ptr pToDescr);
};

//*****************************************************************************
class COtkXSessionActionListener : public pfcDefaultSessionActionListener
{
public:
	void OnBeforeModelCopy(pfcDescriptorContainer2_ptr pContainer);
};

//*****************************************************************************
class COtkXBeforeModelSaveAllListener : public virtual wfcBeforeModelSaveAllListener
{
public:
	void  OnBeforeModelSave(pfcModel_ptr pMdl);
};

class CProtectController : public RMXControllerBase<CProtectController>
{
public:
	void Start();
	void Stop();
	void Protect(pfcModelDescriptors_ptr pMdlDescr, ProtectSourceAction srcAction);
	
	static bool NeedToProtect(pfcModel_ptr pMdl);	
	void ShowEditDenyDialog();
	void TryToBackupFile(const std::wstring& fileOrigin);
	wstring GetNxlOrigin(pfcModel_ptr pMdl);
	
	//
	// For export
	//
	void GetExportTags(pfcModel_ptr pMdl, bool recursive, string& tags, bool& needProtect);
	void GetExportTagsForModel(pfcModel_ptr pMdl, string& tags, bool& needProtect);
	DWORD GetFileWatchers() const { return m_runningFileWatches; };

	bool IsWatcherRunning() const { return (m_runningFileWatches != 0);  }
	bool IsWatcherRunning(DWORD watcherType) const { return HAS_BIT(m_runningFileWatches, watcherType); }
	bool IsWatchingExport() const { return IsWatcherRunning(PROTECT_WATCHER_EXPORT); }
	bool IsWatchingJTExport() const { return IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTBATCH) || IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTCURRENT); }
	bool IsWatchingSave() const { return  IsWatcherRunning(PROTECT_WATCHER_SAVE); }
	bool IsWatchingCopy() const { return IsWatcherRunning(PROTECT_WATCHER_COPY); }

	bool IsInProcess() const { return m_isInProcess; };
	
	void StartFileWatcher(DWORD watcherType);
	void StopFileWatcher(DWORD watcherType);
	void RemoveFileWatcher(DWORD watcherType);

	void ProcessFile(const wstring & filePath, DWORD watcherType);

	void ProcessWatchedFiles(DWORD watcherType);
	
	//void ProtectDrawingPreviewFile(const wstring& imageFileName);
	
	void WatchFile(const std::wstring& newFile);
	void AddJTCADFile(pfcModel_ptr pMdl);
	void AddJTBatchCommandFile(const std::wstring& filePath) { m_JTBatchCommandFiles.insert(filePath); };
	bool ProcessJTExportFile(const std::wstring& jtFilePath);
	bool ProcessJTExportModel(pfcModel_ptr pMdl, const std::wstring& jtFilePath);

	void NotifyActionDeny(DWORD rights, const std::wstring& srcFilePath, const std::wstring& targetFilePath);
	
	void RemoveWatchedExportFile(const std::wstring& filePath) { m_watchedFilesForExport.erase(filePath); }

	bool ProcessCopyFile(const std::wstring& sourceFile, const std::wstring& targetFile);

	void ProtectCurrentCommand();
	void ProtecCurrentWithDepsCommand();

private:
	bool ExecuteJob(ProtectJobPtr pJob);

	void RevertBackupFile(ProtectJobPtr pJob);

	// For export
	void GetExportTagsFromDepedencies(pfcModel_ptr pMdl, string& tags, bool& needProtect);
	bool IsExportWatcher(DWORD watcherType) const { return HAS_BIT(watcherType, PROTECT_WATCHER_EXPORT); }
	bool IsSaveWatcher(DWORD watcherType) const { return HAS_BIT(watcherType, PROTECT_WATCHER_SAVE); }
	bool IsCopyWatcher(DWORD watcherType) const { return HAS_BIT(watcherType, PROTECT_WATCHER_COPY); }
	wstring CProtectController::GetFileWatcherNames(DWORD types);
	void ParseJTCommandFiles();
	// Ipem will generate jpg file by itself if Creo does not create jpg preview files
	bool JPGGeneratedByIpem() const { return m_imageExtOnSave.empty() || m_imageExtOnSave.compare(L"jpg") != 0; }

	void TakeSnapshotForMotionEnvlp();

private:
	typedef std::set<std::wstring, ICaseKeyLess> IWStringSet;
	typedef std::map<std::wstring, std::string, ICaseKeyLess> TFileTagMap;
	typedef std::map<std::wstring, IWStringSet, ICaseKeyLess> TExportDenyResult;
	typedef TExportDenyResult::const_iterator TExportDenyResultCIter;

	std::vector<pfcActionListener_ptr> m_listeners;
	std::map<std::wstring, std::wstring, ICaseKeyLess> m_backupFiles;
	IWStringSet m_editDenyFiles;
	IWStringSet m_watchedFilesForExport;
	IWStringSet m_watchedFilesForCopy;
	IWStringSet m_watchedFilesForSave;
	TFileTagMap m_exportDependencyTagCache;
	DWORD m_runningFileWatches;
	std::wstring m_imageExtOnSave;
	std::map<std::wstring, COtkXCacheModel, ICaseKeyLess> m_JTCADFiles;
	IWStringSet m_JTBatchCommandFiles;
	std::multimap<DWORD, TExportDenyResult> m_exportDenyFiles;
	bool m_isInProcess;
	IWStringSet m_fileTypesToFixName;

	pfcModel_ptr m_pSrcModelForMotionEnvlp;

};

DEFINE_RMXCONTROLLER_GET(ProtectController)