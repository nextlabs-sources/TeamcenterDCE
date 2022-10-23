#pragma once

#include "stdafx.h"
#include "SldWorksRMX.h"
#include "SwResult.h"


class CSwUserAuthorization;
using FileRightHandler =  bool(CSwUserAuthorization::*)(CSwAuthResult &);
using SwCommand = int;
using SwUserCommand = int ;

#define swUserCommands_JtExportCommand 41668
#define swUserCommands_JtQuickExportCommand 41669

class CSwUserAuthorization {

private:
	static CSwUserAuthorization * m_UserAuthorizationInstance;
	CSwUserAuthorization();
	~CSwUserAuthorization();

	static const std::map<pair<SwCommand, SwUserCommand>, FileRight> SwCommandToFileRightMap;
	static const std::map<FileRight, FileRightHandler> FileRightHandlerMap;
public :	
	CSwUserAuthorization(const CSwUserAuthorization &) = delete;
	CSwUserAuthorization & operator=(const CSwUserAuthorization &) = delete;
	static CSwUserAuthorization * GetInstance();
	bool CheckUserAuthorization(const wstring& fileName, const FileRight& opCode);
	bool OnFileSaveAccessCheck(CSwAuthResult &authResultObj);
	bool OnFileMakeVirtualAccessCheck(CSwAuthResult& authResultObj);
	bool OnNewFileSaveAsAccessCheck(const wstring& fileName);
	bool OnFileModifyAccessCheck(CSwAuthResult& authResultObj);
	bool OnFileSaveAsAccessCheck(CSwAuthResult &authResultObj);
	bool OnFileViewAccessCheck(CSwAuthResult &authResultObj);
	bool OnFilePrintAccessCheck(CSwAuthResult &authResultObj);
	bool OnToolsAddinAccessCheck(CSwAuthResult &authResultObj);
	bool OnSendEmailAccessCheck(CSwAuthResult &authResultObj);
	bool PerformFileAccessCheck(CSwAuthResult &authResultObj, const FileRight& accessType);
	bool PerformReferenceFileAccessCheck(const FileRight& accessType, CSwAuthResult & authResultObj);
	bool PerformReferenceFileAccessCheck2(const FileRight& accessType,const wstring& fileName);
	bool PerformReferenceFileAccessCheck3(const FileRight& accessType, CSwAuthResult& authResultObj);
	bool IsFileReadOnly(const wstring& fileName);
	bool PerformCommandPreExecutionCheck(int command,int userCommand, CSwAuthResult& authResultObj);
	bool OnPublishToEDrawingCheck(CSwAuthResult &authResultObj);
	bool DenyIfAnyProtected(CSwAuthResult& authResultObj);
	static void ReleaseInstance() {
		LOG_INFO("CSwUserAuthorization::ReleaseInstance");
		if(m_UserAuthorizationInstance != nullptr)
			delete m_UserAuthorizationInstance;
			m_UserAuthorizationInstance = nullptr;
	}
};

#define theUserAuthorization CSwUserAuthorization::GetInstance()