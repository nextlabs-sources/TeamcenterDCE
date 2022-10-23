#pragma once

#include <memory>
#include <string>

#include "OtkXTypes.h"

#include <pfcModel.h>

class COtkXCacheModel;
class CProtectJob;
typedef std::shared_ptr<CProtectJob> ProtectJobPtr;

//*****************************************************************************
class CProtectJob
{
public:
	explicit CProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	virtual ~CProtectJob();
	std::wstring GetInstanceName() const;
	std::wstring GetFileName() const;
	std::wstring GetFileOrigin() const;	
	std::wstring GetFileDest() const;
	ProtectSourceAction GetSourceActionType() const;
	std::string GetTags() const;
	pfcModelDescriptor_ptr GetModelDescr() const { return m_pMdlDescr; }

	void SetFileDest(const std::wstring& destFilePath);
	void SetFileOrigin(const std::wstring& fileOriginPath);
	void SetTags(const std::string& tags);
	void SetActionType(ProtectSourceAction actionType);
	void SetJobName(const std::wstring& name) { m_jobName = name; };

	bool IsFileZeroKB();

public:
	virtual bool Execute();
	void Dump();

private:
	void Init(pfcModelDescriptor_ptr pDescr);

protected:
	std::wstring m_instanceName;
	std::wstring m_fileName;
	std::wstring m_fileOrigin;
	std::wstring m_fileDest;
	ProtectSourceAction m_actionType;
	std::string m_tags;
	pfcModelDescriptor_ptr m_pMdlDescr;
	std::wstring m_jobName;
};

//*****************************************************************************
class CSaveProtectJob : public CProtectJob
{
public:
	explicit CSaveProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(pfcModelDescriptor_ptr pMdlDescr);
};

//*****************************************************************************
class CCopyProtectJob : public CProtectJob
{
public:
	explicit CCopyProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(pfcModelDescriptor_ptr pToDescr, const wstring& targetFile);
};

//*****************************************************************************
class CExportProtectJob : public CProtectJob
{
public:
	explicit CExportProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(pfcModelDescriptor_ptr pMdlDescr, const wstring& targetFile);
};

//*****************************************************************************
class CJTCurrentProtectJob : public CProtectJob
{
public:
	explicit CJTCurrentProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(pfcModelDescriptor_ptr pMdlDescr, const wstring& targetFile);
};

//*****************************************************************************
class CJTBatchProtectJob : public CProtectJob
{
public:
	explicit CJTBatchProtectJob();
	static ProtectJobPtr Create(const COtkXCacheModel& cadFile, const wstring& targetFile);
};

//*****************************************************************************
class CAuxiImageFileProtectJob : public CProtectJob
{
public:
	explicit CAuxiImageFileProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(ProtectJobPtr pMdlJob, const wstring& imageFilePath);
	static std::wstring BuildImageFilePath(const wstring & mdlFilePath, const wstring& imageExt);
};

//*****************************************************************************
class CTCAuxiFileProtectJob : public CProtectJob
{
public:
	explicit CTCAuxiFileProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(const wstring& auxiFilePath);
};

//*****************************************************************************
class CMotionEnvlpProtectJob : public CProtectJob
{
public:
	explicit CMotionEnvlpProtectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(pfcModelDescriptor_ptr pMdlDescr, const wstring& targetFile);
};

//*****************************************************************************
class CFileCopyProtectJob : public CProtectJob
{
public:
	explicit CFileCopyProtectJob();
	static ProtectJobPtr Create(const wstring& sourceFile, const wstring& targetFile);
};


//*****************************************************************************
class CCommandProjectJob : public CProtectJob
{
public:
	explicit CCommandProjectJob(pfcModelDescriptor_ptr pMdlDescr);
	static ProtectJobPtr Create(pfcModel_ptr pMdl, const string& tags);
	virtual bool Execute();
};