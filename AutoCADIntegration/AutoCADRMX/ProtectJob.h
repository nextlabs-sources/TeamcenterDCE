//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//

#pragma once
#include <memory>
#include <string>

#include "AcRMXTypes.h"

class AcDbDatabase;
class CProtectJob;
typedef std::shared_ptr<CProtectJob> ProtectJobPtr;

class CProtectJob
{
public:
	explicit CProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName);
	virtual ~CProtectJob();

	void SetTags(const std::string& strTags) { m_strTags = strTags; }
	void SetFileDest(const std::wstring& strFileDest) { m_strFileDest = strFileDest; }
	void SetFileOrigin(const std::wstring& strFileOrigin) { m_strFileOrigin = strFileOrigin; }

	std::wstring FileOrign() const { return m_strFileOrigin; }
	std::wstring FileDest() const { return m_strFileDest; }

public:
	virtual bool Execute();

protected:
	void Dump();

protected:
	std::wstring m_strFileName;
	std::wstring m_strFileOrigin;
	std::wstring m_strFileDest;
	AcRMX::OpType m_eOpType;
	std::string m_strTags;
	AcDbDatabase* m_pDwg;
	std::wstring m_strJobDesc;
};

//*****************************************************************************
class CSaveProtectJob : public CProtectJob
{
public:
	explicit CSaveProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName);
	static ProtectJobPtr Create(AcDbDatabase* pDwg, const TCHAR* szFileName);
};

//*****************************************************************************
class CCopyProtectJob : public CProtectJob
{
public:
	explicit CCopyProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName);
	static ProtectJobPtr Create(AcDbDatabase* pDwg, const TCHAR* szFileName);

	virtual bool Execute();
};

//*****************************************************************************
class CPublishProtectJob : public CProtectJob
{
public:
	explicit CPublishProtectJob();
	static ProtectJobPtr Create(AcPublishReactorInfo* pInfo);
};

//*****************************************************************************
class CCommandProtectJob : public CProtectJob
{
public:
	explicit CCommandProtectJob(AcDbDatabase* pDwg, const TCHAR* szFileName);
	static ProtectJobPtr Create(AcDbDatabase* pDwg, const TCHAR* szFileName, const std::string& tags);

	virtual bool Execute();
};


//*****************************************************************************
class CExportProtectJob : public CProtectJob
{
public:
	explicit CExportProtectJob();
	static ProtectJobPtr Create(const TCHAR* szFileToProtect);
};