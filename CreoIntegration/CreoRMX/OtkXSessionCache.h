#pragma once

#include <map>
#include <string>

#include "..\common\CommonTypes.h"

#include "OtkXTypes.h"
#include "OtkXUtils.h"
#include <pfcModel.h>

class COtkXModel;

class COtkXFile
{
public:
	COtkXFile() { m_mdlType = pfcModelType_nil; };
	explicit COtkXFile(const std::wstring& filePath);
	bool operator == (const COtkXFile& other) const;
	bool operator == (const std::wstring& filePath) const;

	std::wstring GetName() const { return m_nameData.GetBaseName(); }
	std::wstring GetExt() const { return m_nameData.GetExtension(); }
	std::wstring GetDir() const { return m_nameData.GetDirectoryPath(); }
	std::wstring GetFullPath() const { return m_nameData.GetFullPath(); }
	std::wstring GetId() const { return m_id; }
	pfcModelType GetType() const { return m_mdlType; }

	bool IsAsm() const { return m_mdlType == pfcMDL_ASSEMBLY; }
	bool IsDrw() const { return m_mdlType == pfcMDL_DRAWING; }
	bool IsPrt() const { return m_mdlType == pfcMDL_PART; }
	bool IsJT() const { return wcsicmp(m_nameData.GetExtension().c_str(), L"jt") == 0; }

	bool IsVersionFile(const COtkXFile& other);

private:
	std::wstring m_id;
	pfcModelType m_mdlType;
	OtkXUtils::OtkXFileNameData m_nameData;
};

class COtkXSessionCache: public RMXControllerBase<COtkXSessionCache>
{
public:
	void Start();
	void Stop();

// Models in session
public:
	bool IsNxlModel(const COtkXModel& xMdl) const;
	bool IsNxlFile(const wstring& filePath) const;
	bool HasNxlDepModel(const COtkXModel& xMdl) const;
	void EraseNxlModel(const COtkXModel& xMdl);
	void AddNxlModel(const TModelId& id, const wstring& filePath, bool rpmReadTags = true);
	void RenameNxlModel(const COtkXModel& oldXMdl, const COtkXModel& newXMdl);
	void CopyNxlModel(const COtkXModel& srcXMdl, const COtkXModel& newXMdl);
	std::wstring FindNxlOrigin(const COtkXModel& xMdl);

	void AddDirtyNxlToReload(const TModelId& id);

	bool IsDirtyToReload(const TModelId& id) const {
		return (m_dirtyToReloadNxl.find(id) != m_dirtyToReloadNxl.end());
	}

	// Tags in session
public:
	void SaveTags(const std::wstring& filePath, const string& tags);
	std::string GetTags(const std::wstring& filePath);
private:
	typedef std::map<std::wstring, std::string, ICaseKeyLess> TFileTagMap;
	std::map<TModelId, COtkXFile> m_nxlModelCache;
	TFileTagMap m_nxlFileTagCache;
	std::set<TModelId> m_dirtyToReloadNxl; // Record the failure when erasing the new protected file
};

DEFINE_RMXCONTROLLER_GET(OtkXSessionCache)
