#include "SwUserAuthorization.h"
#include "SwRMXMgr.h"
#include "FileUtils.h"
#include "SwUtils.h"

CSwUserAuthorization* CSwUserAuthorization::m_UserAuthorizationInstance = NULL;

const map<pair<SwCommand, SwUserCommand>, FileRight> CSwUserAuthorization::SwCommandToFileRightMap = {
	{make_pair(swCommands_RapidPrototype,0), FileRight::RMX_RIGHT_PRINT},
	{make_pair(swCommands_Save_Template,0),FileRight::RMX_RIGHT_EXPORT},
	// {make_pair(swCommands_User_Toolbar_Min,swUserCommands_JtExportCommand),FileRight::RMX_RIGHT_EXPORT},
	{make_pair(swCommands_User_Toolbar_Min,swUserCommands_JtQuickExportCommand),FileRight::RMX_RIGHT_EXPORT },
	{make_pair(swCommands_Tools_Addins,0),FileRight::RMX_RIGHT_TOOLSADDIN },
	{make_pair(swCommands_File_Copy_Design,0),FileRight::RMX_RIGHT_EXPORT}
};

const map<FileRight, FileRightHandler> CSwUserAuthorization::FileRightHandlerMap = {
	{ FileRight::RMX_RIGHT_PRINT, &CSwUserAuthorization::OnFilePrintAccessCheck },
	{ FileRight::RMX_RIGHT_EXPORT, &CSwUserAuthorization::OnFileSaveAsAccessCheck },
	{ FileRight::RMX_RIGHT_EDIT	, &CSwUserAuthorization::OnFileSaveAsAccessCheck },
	{ FileRight::RMX_RIGHT_TOOLSADDIN ,&CSwUserAuthorization::OnToolsAddinAccessCheck},

};

CSwUserAuthorization::CSwUserAuthorization() {
	//CSwUserAuthorization Constructor
	return;
}

CSwUserAuthorization::~CSwUserAuthorization() {
	//CSwUserAuthorization Destructor
}


CSwUserAuthorization * CSwUserAuthorization::GetInstance() {
	if (m_UserAuthorizationInstance != nullptr) {
		return m_UserAuthorizationInstance;
	}
	else {
		m_UserAuthorizationInstance = new CSwUserAuthorization();
		return m_UserAuthorizationInstance;
	}
}

bool CSwUserAuthorization::CheckUserAuthorization(const wstring& fileName, const FileRight& opCode) {
	long right = static_cast<DWORD>(opCode);
	bool isAuthorized = true;
	LOG_INFO_FMT("FileName = %s , OpCode = %lu",fileName.c_str(), right);
	CSwRMXMgr * swRMXMgrInstance = CSwRMXMgr::GetInstance();
	bool isFileProtected = false;
	if (swRMXMgrInstance->RMX_RPMGetFileStatus(fileName, &isFileProtected)
		&& isFileProtected) {
		isAuthorized = false;
		//Retrieve the rights assigned to the file
		std::vector<int> fileRights;
		bool isRightsAssigned = swRMXMgrInstance->CheckRights(fileName, fileRights);
		if (!isRightsAssigned || fileRights.size() == 0) {
			LOG_WARN("Error while fetching the rights for the file = " << fileName.c_str());
		}
		if (fileRights.size() > 0) {
			for (auto r : fileRights)
			{
				if (right & r)
				{
					isAuthorized = true;
					break;
				}
			}
			if (!isAuthorized) {
				LOG_ERROR("User's requested operation not found in the rights Map, Operation Code :" << right);
			}
		}
	}
	return isAuthorized;
}

bool CSwUserAuthorization::PerformFileAccessCheck(CSwAuthResult &authResultObj,const FileRight& accessType) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO_FMT("FileName = %s , AccessType = %lu", fileName.c_str(), static_cast<DWORD>(accessType));
	bool isAuthorized = CheckUserAuthorization(fileName, accessType);
	if (!isAuthorized) {
		if (accessType == FileRight::RMX_RIGHT_EDIT && !CSwUtils::GetInstance()->IsDocumentDirty(fileName)) {
			isAuthorized = true;
		}
		else {
			wstring fileNameWithExt = FileUtils::GetFileNameWithExt(fileName);
			authResultObj.SetFileRight(accessType);
			authResultObj.AddFilesToDenyAccessList(fileNameWithExt);
			authResultObj.SetMessageType(MsgType::MSG_ERROR);
		}
	}
	CSwRMXMgr::GetInstance()->LogFileAccessResult(fileName, accessType, isAuthorized);
	return isAuthorized;
}


bool CSwUserAuthorization::PerformReferenceFileAccessCheck(const FileRight& accessType, CSwAuthResult & authResultObj) {
	wstring fileName = authResultObj.FileName();
	//Do access check for referenced components in the file.
	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	vector<wstring> referencedFileNames = CSwUtils::GetInstance()->GetReferencedModelDocuments(fileName);

	wstring delim = L"";
	for (auto refFileName : referencedFileNames) {
		bool isAuthorized = CheckUserAuthorization(refFileName, accessType);
		if (!isAuthorized) {
			wstring fileNameWithExt = FileUtils::GetFileNameWithExt(refFileName);
			authResultObj.AddFilesToDenyAccessList(fileNameWithExt);
		}
		CSwRMXMgr::GetInstance()->LogFileAccessResult(refFileName, accessType, isAuthorized);
	}	
	if (authResultObj.GetFileDenyAccessList().size()> 0) {
		authResultObj.SetFileRight(accessType);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
		return false;
	}
	return true;
}

bool CSwUserAuthorization::PerformReferenceFileAccessCheck2(const FileRight& accessType,const wstring& fileName) {
	//Do access check for referenced components in the file.
	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	vector<wstring> referencedFileNames = CSwUtils::GetInstance()->GetReferencedModelDocuments(fileName);
	for (auto refFileName : referencedFileNames) {
		bool isAuthorized = CheckUserAuthorization(refFileName, accessType);
		if (!isAuthorized) {
			return false;
		}
	}

	return true;
}


bool CSwUserAuthorization::PerformReferenceFileAccessCheck3(const FileRight& accessType,CSwAuthResult& authResultObj) {
	// Do access check for referenced components in the file.
	//For the files for which modification is not allowed, Check if they were edited at this point.
	wstring fileName = authResultObj.FileName();
	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	vector<wstring> referencedFileNames = CSwUtils::GetInstance()->GetReferencedModelDocuments(fileName);
	wstring delim = L"";
	for (auto refFileName : referencedFileNames) {
		bool isAuthorized = CheckUserAuthorization(refFileName, accessType);
		if (!isAuthorized) {
			if (CSwUtils::GetInstance()->IsDocumentDirty(refFileName)) {
				wstring fileNameWithExt = FileUtils::GetFileNameWithExt(refFileName);
				authResultObj.AddFilesToDenyAccessList(fileNameWithExt);	
			}
			else {
				isAuthorized = true;
			}
		}
		CSwRMXMgr::GetInstance()->LogFileAccessResult(refFileName, accessType, isAuthorized);
	}
	if (authResultObj.GetFileDenyAccessList().size()> 0) {
		authResultObj.SetFileRight(accessType);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
		return false;
	}
	return true;
}




bool CSwUserAuthorization::OnFileModifyAccessCheck(CSwAuthResult& authResultObj) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO("FileName = " << fileName.c_str());
	bool isAuthorized = CheckUserAuthorization(fileName, FileRight::RMX_RIGHT_EDIT);
	if (!isAuthorized) {
		LOG_INFO("User doesn't possess Edit rights on file = " << fileName.c_str());
		wstring message = L"You don't have Edit permission to perform this action. Changes made to the following read-only file cannot be saved:\r\n";
		wstring fileNameWithExt = FileUtils::GetFileNameWithExt(fileName);
		authResultObj.SetErrorMsg(message);
		authResultObj.SetFileRight(FileRight::RMX_RIGHT_MODIFY);
		authResultObj.AddFilesToDenyAccessList(fileNameWithExt);
		authResultObj.SetMessageType(MsgType::MSG_WARN);
		return false;
	}
	return true;
}


bool CSwUserAuthorization::OnFileSaveAccessCheck(CSwAuthResult &authResultObj) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO("FileName = " << fileName);
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EDIT)) {
		if (CSwUtils::GetInstance()->IsDocumentDirty(fileName)) {
			LOG_INFO("User doesn't possess Save rights on file = " << fileName.c_str());
			authResultObj.SetErrorMsg(L"Operation Denied.\r\n You do not have 'EDIT' permission to perform this action on the following file(s):\r\n");
			return false;
		}
	}
	return true;
}
bool CSwUserAuthorization::OnFileMakeVirtualAccessCheck(CSwAuthResult& authResultObj) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO("FileName = " << fileName);
	LOG_DEBUG("Currently always deny make virtual");
	authResultObj.SetErrorMsg(L"Operation Denied.\r\n Following files are made virtual from NextLabs protected file, currently this is not supported .\r\n");
	authResultObj.AddFilesToDenyAccessList(FileUtils::GetFileNameWithExt(fileName));
	return false;
	/*
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_DECRYPT))
	{
		LOG_INFO("User doesn't possess Extract rights on file = " << fileName.c_str());
		authResultObj.SetErrorMsg(L"Operation Denied.\r\n Following files are made virtual from NextLabs protected file, you need Extract right to save them.\r\n");
		return false;
	}
	return true;
	*/
}

bool CSwUserAuthorization::OnNewFileSaveAsAccessCheck(const wstring& fileName) {
	LOG_INFO("CSwUserAuthorization::OnNewFileSaveAsAccessCheck.");
	if (!PerformReferenceFileAccessCheck2(FileRight::RMX_RIGHT_EXPORT,fileName)) {
		LOG_INFO("CSwUserAuthorization::OnNewFileSaveAsAccessCheck.User doesn't possess 'Save As' or 'Download' rights for one of the referenced file");
		return false;
	}
	return true;
}


bool CSwUserAuthorization::OnFileSaveAsAccessCheck(CSwAuthResult &authResultObj) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO("FileName = " << fileName.c_str());
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EXPORT)) {
		LOG_INFO("User doesn't possess 'Save As' right on file = " << fileName.c_str());
		authResultObj.SetErrorMsg(L"Operation Denied.\r\n You do not have 'Save As' permission to perform this action on the following file(s):\r\n");
		return false;
	}
	
	long docType = CSwUtils::GetInstance()->GetModelDocType();
	if (docType == swDocNONE) {
		docType = CSwUtils::GetInstance()->GetModelDocTypeByName(fileName);
	}

	if (docType == swDocASSEMBLY || docType == swDocDRAWING) {
		if (!PerformReferenceFileAccessCheck(FileRight::RMX_RIGHT_EXPORT, authResultObj)) {
			LOG_INFO("User doesn't possess 'Save As' right for one of the referenced file");
			authResultObj.SetErrorMsg(L"Operation Denied.\r\n You do not have 'Save As' permission to perform this action on the following file(s):\r\n");
			return false;
		}

	}

	//Check if the source files(and any of its references) doesn't contain edit right and were modified.
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EDIT)) {
		LOG_INFO("User possess 'Save As' right on file but no 'EDIT' rights and the file was modified = " << fileName.c_str());
		authResultObj.SetErrorMsg(L"Operation Denied.\r\n You are not allowed to perform this action because the following files(s) with no 'EDIT' permission were modified :\r\n");
		return false;
	}

	if (docType == swDocASSEMBLY || docType == swDocDRAWING) {
		if (!PerformReferenceFileAccessCheck3(FileRight::RMX_RIGHT_EDIT, authResultObj)) {
			LOG_INFO("User possess 'Save As' right on the file but one of the referenced file doesn't contain EDIT rights and was modified.");
			authResultObj.SetErrorMsg(L"Operation Denied.\r\n You are not allowed to perform this action because the following files(s) with no 'EDIT' permission were modified:\r\n");
			return false;
		}

	}


	return true;
}

bool CSwUserAuthorization::OnFileViewAccessCheck(CSwAuthResult &authResultObj) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO("FileName = " << fileName.c_str());
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_VIEW)) {
		LOG_INFO("User doesn't possess 'View' rights on file = " << fileName.c_str());
		authResultObj.SetErrorMsg(L"Operation Denied.\r\n You do not have 'VIEW' permission to perform this action on the following file(s):\r\n");
		return false;
	}
	return true;

}

bool CSwUserAuthorization::OnFilePrintAccessCheck(CSwAuthResult &authResultObj) {
	wstring fileName = authResultObj.FileName();
	LOG_INFO("FileName = " << fileName.c_str());
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_PRINT)) {
		LOG_INFO("User doesn't possess 'PRINT' rights on file = " << fileName.c_str());
		authResultObj.SetErrorMsg(L"Operation Denied.\r\n You do not have 'PRINT' permission to perform this action on the following file(s):\r\n");
		return false;
	}
	
	long docType = CSwUtils::GetInstance()->GetModelDocType();
	if (docType == swDocNONE) {
		docType = CSwUtils::GetInstance()->GetModelDocTypeByName(fileName);
	}
	if (docType == swDocASSEMBLY || docType == swDocDRAWING) {
		if (!PerformReferenceFileAccessCheck(FileRight::RMX_RIGHT_PRINT, authResultObj)) {
			LOG_INFO("User doesn't possess 'PRINT' rights for one of the referenced file");
			authResultObj.SetErrorMsg(L"Operation Denied.\r\n You do not have 'PRINT' permission to perform this action on the following file(s):\r\n");
			return false;
		}

	}

	//Check if the source files(and any of its references) doesn't contain edit right and were modified.
	if (!PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EDIT)) {
		LOG_INFO("User possess 'PRINT' rights on file but no 'EDIT' rights and the file was modified = " << fileName.c_str());
		authResultObj.SetErrorMsg(L"Operation Denied.\r\n You are not allowed to perform this action because the following files(s) with no 'EDIT' permission were modified:\r\n");
		return false;
	}

	if (docType == swDocASSEMBLY || docType == swDocDRAWING) {
		if (!PerformReferenceFileAccessCheck3(FileRight::RMX_RIGHT_EDIT, authResultObj)) {
			LOG_INFO("User possess 'Print' rights on the file but one of the referenced file doesn't contain EDIT rights and was modified.");
			authResultObj.SetErrorMsg(L"Operation Denied.\r\n You are not allowed to perform this action because the following files(s) with no 'EDIT' permission were modified:\r\n");
			return false;
		}

	}

	return true;

}

bool CSwUserAuthorization::IsFileReadOnly(const wstring& fileName) {
	LOG_INFO("FileName = " << fileName.c_str());
	return CheckUserAuthorization(fileName, FileRight::RMX_RIGHT_EDIT);
}

bool CSwUserAuthorization::PerformCommandPreExecutionCheck(int command,int userCommand, CSwAuthResult& authResultObj) {
	LOG_DEBUG_FMT("Command = %d ,UserCommand = %d " , command,userCommand);

	if (command == swCommands_SendTo && userCommand == 0) {
		wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
		authResultObj.SetFileName(activeDoc);
		return OnSendEmailAccessCheck(authResultObj);
	}

	pair<SwCommand, SwUserCommand> key = make_pair(command, userCommand);
	if (SwCommandToFileRightMap.find(key) != SwCommandToFileRightMap.end()) {
		FileRight accessType = SwCommandToFileRightMap.at(key);
		if (FileRightHandlerMap.find(accessType) != FileRightHandlerMap.end()) {
			wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
			authResultObj.SetFileName(activeDoc);
			LOG_INFO_FMT("FileName = %s , Command = %d ,userCommand = %d", activeDoc.c_str(), command ,userCommand);
			FileRightHandler fileRightHandler = FileRightHandlerMap.at(accessType);
			bool authorizeStatus = (this->*fileRightHandler)(authResultObj);
			if (!authorizeStatus)
				return false;
		}
	}
	return true;
}


bool CSwUserAuthorization::OnToolsAddinAccessCheck(CSwAuthResult &authResultObj) {
	//Check if any protected document is opened in the current solidworks session
	bool isProtectedDocOpen = CSwUtils::GetInstance()->IsNXLDocOpenedInMemory();
	if (isProtectedDocOpen) {
		LOG_ERROR("Operation Denied. One or more protected document(or references) opened in SolidWorks session memory. Please close them and try again.");
		wstring msg = L"Operation Denied. \nOne or more protected document(or references) opened in SolidWorks session memory. Please close them and try again.";
		authResultObj.SetErrorMsg(msg);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
		return false;
	}
	return true;
}

bool CSwUserAuthorization::OnSendEmailAccessCheck(CSwAuthResult &authResultObj) {
	//Check if any protected document is opened in the current solidworks session
	wstring fileName = authResultObj.FileName();
	bool isProtectedDoc = CSwUtils::GetInstance()->IsFileHasNXLDoc(fileName);
	if (isProtectedDoc) {
		LOG_ERROR("Operation Denied. You are not allowed to send the protected object as attachment(s)");
		wstring msg = L"Operation Denied.\nYou are not allowed to directly send the object if it is protected or any dependent object is protected.\nPlease Save a Copy of object with RMX and attach to email.";
		authResultObj.SetErrorMsg(msg);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
		return false;
	}
	return true;
}

bool CSwUserAuthorization::OnPublishToEDrawingCheck(CSwAuthResult &authResultObj) {
	//Check if any protected document is opened in the current solidworks session
	wstring fileName = authResultObj.FileName();
	bool isProtectedDoc = CSwUtils::GetInstance()->IsFileHasNXLDoc(fileName);
	if (isProtectedDoc) {
		LOG_ERROR("Operation Denied. You are not allowed to perform \"Publish to eDrawings\" operation if a file or any of its references are protected. FileName = " << fileName.c_str());
		wstring msg = L"Operation Denied.\nYou are not allowed to perform \"Publish to eDrawings\" operation if a file or any of its references are protected.\n";
		authResultObj.SetErrorMsg(msg);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
		return false;
	}
	return true;

}

bool CSwUserAuthorization::DenyIfAnyProtected(CSwAuthResult& authResultObj) {
	//Check if any protected document is opened in the current solidworks session
	wstring fileName = authResultObj.FileName();
	bool isProtectedDoc = CSwUtils::GetInstance()->IsFileHasNXLDoc(fileName);
	if (isProtectedDoc) {
		LOG_ERROR("Operation Denied. You are not allowed to perform this operation if a file or any of its references are protected. FileName = " << fileName.c_str());
		wstring msg = L"Operation Denied.\nYou are not allowed to perform this operation if a file or any of its references are protected.\n";
		authResultObj.SetErrorMsg(msg);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
		return false;
	}
	return true;

}