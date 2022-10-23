#include "SwRMXSwimInterface.h"
#include "SwUserAuthorization.h"
#include "SwUtils.h"
#include "SwUtils.h"
#include "FileUtils.h"

enum TC_OPCODE {
    TC_SAVE = 0,
	TC_SAVEALL = 1,
	TC_SAVEAS = 2,
	TC_CLONE = 3,
	TC_SAVENEW = 4,
	TC_SAVEREPLACE = 5,
	TC_PROCESS_SAVENEW = 6,
	TC_PROCESS_SAVEREPLACE = 7
};

enum TC_ERRCODE {
	TC_SUCCESS = 0,
	TC_AUTHFAIL_RIGHTEDIT = 1,
	TC_AUTHFAIL_RIGHTDOWNLOAD = 2,
	TC_AUTHFAIL_DOWNLOAD_NORIGHTEDIT = 3,
	TC_FILE_PARSE_ERROR = 4,
	TC_AUTHFAIL_EDIT_NOJTSAVERIGHT = 5,
	TC_AUTHFAIL_REVISE_NOEDITRIGHT = 6,
	TC_AUTHFAIL_REVISE_NOSAVEASRIGHT = 7
};

CSldWorksRMX * CSwRMXSwimInterface::userAddin = NULL;

CSwRMXSwimInterface::CSwRMXSwimInterface() {
	//CSwRMXSwimInterface Constructor
	return;
}

CSwRMXSwimInterface::~CSwRMXSwimInterface() {
	//CSwRMXSwimInterface Destructor
}


//int CSwRMXSwimInterface::ProcessTCRequest(long operationType, string xmlpath, CSwAuthResult &authResultObj) {
//	
//	CSwUserAuthorization * userAuthorizationInstance = CSwUserAuthorization::GetInstance();
//	LOG_INFO_FMT("OperationType = %d, xmlpath = %s", operationType,xmlpath.c_str());
//	int status = 0;
//	switch (operationType) {
//	case TC_SAVE:
//		status = PerformAccessCheckByTCSelection(xmlpath, TC_SAVE, FileRight::RMX_RIGHT_EDIT, authResultObj);
//		break;
//	case TC_SAVEAS:
//		status = PerformAccessCheckByTCSelection(xmlpath, TC_SAVEAS, FileRight::RMX_RIGHT_EDIT, authResultObj);
//		break;
//	case TC_SAVEALL:
//		status = PerformAccessCheckByTCSelection(xmlpath, TC_SAVEALL, FileRight::RMX_RIGHT_EDIT, authResultObj);
//		break;
//	case TC_CLONE:
//		status = PerformAccessCheckByTCSelection(xmlpath, TC_CLONE, FileRight::RMX_RIGHT_EXPORT, authResultObj);
//		break;
//	
//	case TC_SAVENEW: 
//		status = PerformAccessCheckByTCSelection(xmlpath, TC_SAVENEW,FileRight::RMX_RIGHT_EXPORT,authResultObj);
//		break;
//	case TC_SAVEREPLACE: 
//		status = PerformAccessCheckByTCSelection(xmlpath, TC_SAVEREPLACE,FileRight::RMX_RIGHT_EXPORT,authResultObj);
//		break;
//	case TC_PROCESS_SAVENEW:
//	case TC_PROCESS_SAVEREPLACE:
//		status = ProcessSaveNewResponse(xmlpath);
//		break;
//	default:
//		break;
//	}
//	LOG_INFO_FMT("OperationType:%d ,authorizeStatus:%d" , operationType, status);
//	return status;
//
//}

int CSwRMXSwimInterface::ProcessSwimRequest(const string& fileName, const string& right, CSwAuthResult& authResultObj)
{
	CSwUserAuthorization* userAuthorizationInstance = CSwUserAuthorization::GetInstance();
	const wstring& wFileName = RMXUtils::s2ws(fileName);
	LOG_INFO_FMT("fileName = %s, right = %s", fileName.c_str(), right.c_str());
	int status = 0;
	CSldWorksRMX::ClearTCDestSrcDocMap();
	if (_stricmp(right.c_str(), "save") == 0) {	
		status = CheckRightByTC(wFileName, FileRight::RMX_RIGHT_EDIT, authResultObj);
	}
	else if (_stricmp(right.c_str(), "saveas") == 0) {	
		status = CheckRightByTC(wFileName, FileRight::RMX_RIGHT_DOWNLOAD, authResultObj);
	}
	else if (_stricmp(right.c_str(), "export") == 0) {
		status = CheckRightByTC(wFileName, FileRight::RMX_RIGHT_DOWNLOAD, authResultObj, false);
	}
	else if (_stricmp(right.c_str(), "saveasresponse") == 0) {
		
		status = ProcessSaveNewResponse(fileName);
	}

	return status;
}

bool CSwRMXSwimInterface::GetAllDocumentFromXML(const string &xmlpath, InputXMLObj & inputObj) {
	SwimXMLParser parser;
	bool status = parser.ParseSwimInputXML(xmlpath, inputObj);
	if (!status)
		return false;
	return true;
}

bool CSwRMXSwimInterface::GetTCFileFullPath(const wstring &fName, wstring &fileNameWithPath) {
	wstring wsFileLocation = L"";
	
	CSwUtils * swUtilsInstance = CSwUtils::GetInstance();
	bool status = swUtilsInstance->GetSwUserPreferenceStringValue(swFileLocationsDocuments,wsFileLocation);
	if (!status) {
		LOG_ERROR("Error reading file location for documents");
	}
	wstring fileName = wsFileLocation + L"\\" + fName;
	if (!FileUtils::IsFileExists(fileName)) {
		LOG_ERROR("File pointed by fileName couldn't be found = " << fileName);
		fileName = L"";
		//Check if the file pointed by fName is opened in the memory and fetch the full path of the fileName
		CComPtr<IModelDoc2> modelDocPtr = swUtilsInstance->IsFileOpened2(fName);
		if (modelDocPtr != NULL) {
			LOG_INFO_FMT("File %s is opened in current SolidWorks session", fName.c_str());
			fileName = swUtilsInstance->GetModelNameForModelDoc(modelDocPtr);
			if (fileName.empty()) {
				LOG_ERROR("File pointed by fileName couldn't be searched in open documents in memory" << fName.c_str());
			}
		}
		else {
			LOG_ERROR("File pointed by fileName couldn't be found. This could be in the case of new files" << fName.c_str());
		}
	}
	fileNameWithPath = fileName;
	return true;
}

void CSwRMXSwimInterface::PerformTCAccessCheck(const vector<wstring>& fileNameList, const FileRight& fileRight, CSwAuthResult &authResultObj) {
	bool isAuthorized = false;
	CSwUserAuthorization *userAuthorizationInstance = CSwUserAuthorization::GetInstance();
	for (auto fileName : fileNameList) {
		authResultObj.SetFileName(fileName);
		isAuthorized = userAuthorizationInstance->PerformFileAccessCheck(authResultObj, fileRight);
		if (!isAuthorized) {
			LOG_ERROR("User doesn't possess 'Save As' permission this action on File = " << fileName);
		}
	}
}

void CSwRMXSwimInterface::FilterFileToBeProcessed(const int& operationType,vector<wstring>& fileNameList) {
	wstring hintLoc = L"";
	for (auto modelDoc : inputXMLObj.modelDocList) {
		bool flag = false;
		if (operationType == TC_SAVENEW || operationType == TC_SAVEREPLACE) {
			flag = (!_stricmp(modelDoc.selectedFlag.c_str(), "yes")) ? true : false;
		}
		else if (operationType == TC_SAVEALL || operationType == TC_SAVE || operationType == TC_SAVEAS || operationType == TC_CLONE) {
			flag = (!_stricmp(modelDoc.saveFlag.c_str(), "yes")) ? true : false;
		}

		if (flag) {
			wstring wsFileLocation = L"";
			wstring modelName = RMXUtils::s2ws(modelDoc.file_name);
			FindWorkspaceDir(modelName, hintLoc, wsFileLocation);
			wstring filePath = wsFileLocation + L"\\" + modelName;
			if (FileUtils::IsFileExists(filePath)) {
				fileNameList.push_back(filePath);
			}
		}
	}
}


bool CSwRMXSwimInterface::IsAnyFileBeingRevised(vector<wstring>& reviseFileList) {
	wstring hintLoc = L"";
	for (auto modelDoc : inputXMLObj.modelDocList) {
		if (!_stricmp(modelDoc.saveFlag.c_str(), "yes") && !_stricmp(modelDoc.first_save.c_str(), "yes")
			&& !modelDoc.item_rev_uid.empty() && !_stricmp(modelDoc.item_name_enablement.c_str(), "no")) {
			wstring wsFileLocation = L"";
			wstring modelName = RMXUtils::s2ws(modelDoc.file_name);
			FindWorkspaceDir(modelName, hintLoc, wsFileLocation);
			wstring filePath = wsFileLocation + L"\\" + modelName;
			if (FileUtils::IsFileExists(filePath)) {
				reviseFileList.push_back(filePath);
			}
		}
	}
	return reviseFileList.size() > 0 ? true : false;
}

//int CSwRMXSwimInterface::PerformAccessCheckByTCSelection(const string& xmlpath, const int& operationType, const FileRight& fileRight, CSwAuthResult &authResultObj) {
//	LOG_INFO_FMT("xmlpath = %s , operationType = %d", xmlpath.c_str(),operationType);
//	bool status = GetAllDocumentFromXML(xmlpath, inputXMLObj); //inputXMLObj.modelDocList will contain all the entries from dialog box whether selected or not.
//	if (!status) {
//		LOG_ERROR("Error parsing document from xmlpath = " << xmlpath.c_str());
//		return TC_FILE_PARSE_ERROR;
//	}
//
//	vector<wstring> fileNameList; //Stores only those entries on which user wanted to perform action.
//
//	//Filter out only those entries from modelDocList which were selected/checked for the operation.
//	
//	FilterFileToBeProcessed(operationType,fileNameList);
//
//	int err_code = TC_SUCCESS;
//	if (operationType == TC_SAVEALL || operationType == TC_SAVE || operationType == TC_SAVEAS) {
//		//Check if any file being revised
//		vector<wstring> reviseFileList;
//		if (IsAnyFileBeingRevised(reviseFileList)) {
//			CSwUserAuthorization *userAuthorizationInstance = CSwUserAuthorization::GetInstance();
//			for (wstring fName : reviseFileList) {
//				bool isAuthorized = userAuthorizationInstance->CheckUserAuthorization(fName, fileRight);
//				if (!isAuthorized) {
//					wstring fileNameWithExt = FileUtils::GetFileNameWithExt(fName);
//					authResultObj.SetFileRight(fileRight);
//					authResultObj.AddFilesToDenyAccessList(fileNameWithExt);
//					authResultObj.SetMessageType(MsgType::MSG_ERROR);
//				}
//				CSwRMXMgr::GetInstance()->LogFileAccessResult(fName, fileRight, isAuthorized);
//			}
//			if (!authResultObj.GetAuthResult()) {
//				err_code = TC_AUTHFAIL_REVISE_NOEDITRIGHT;
//			}
//			else {
//				PerformTCAccessCheck(reviseFileList, FileRight::RMX_RIGHT_EXPORT, authResultObj);
//				if (!authResultObj.GetAuthResult()) {
//					err_code = TC_AUTHFAIL_REVISE_NOSAVEASRIGHT;
//				}
//			}
//		}
//		if (authResultObj.GetAuthResult())
//		{
//			PerformTCAccessCheck(fileNameList, fileRight, authResultObj);
//			if (!authResultObj.GetAuthResult()) {
//				err_code = TC_AUTHFAIL_RIGHTEDIT;
//			}
//			else {
//				if (IsSaveJTFilesEnabled()) {
//					PerformTCAccessCheck(fileNameList, FileRight::RMX_RIGHT_EXPORT, authResultObj);
//					if (!authResultObj.GetAuthResult()) {
//						err_code = TC_AUTHFAIL_EDIT_NOJTSAVERIGHT;
//					}
//				}
//			}
//		}
//	}
//	else if (operationType == TC_SAVENEW || operationType == TC_SAVEREPLACE || operationType == TC_CLONE) {
//		PerformTCAccessCheck(fileNameList,fileRight,authResultObj);
//		if (!authResultObj.GetAuthResult()) {
//			err_code = TC_AUTHFAIL_RIGHTDOWNLOAD;
//		}
//		else {
//			PerformTCAccessCheck(fileNameList, FileRight::RMX_RIGHT_EDIT, authResultObj);
//			if (!authResultObj.GetAuthResult()) {
//				err_code = TC_AUTHFAIL_DOWNLOAD_NORIGHTEDIT;
//			}
//		}
//	}
//
//	if (!authResultObj.GetAuthResult()) {
//		authResultObj.SetFileRight(fileRight);
//		return err_code;
//	}
//	return err_code;
//}
//
int CSwRMXSwimInterface::CheckRightByTC(const wstring & fileName, FileRight fileRight, 
	CSwAuthResult & authResultObj, bool checkEditOnDirty /*= true*/)
{
	int err_code = TC_SUCCESS;
	CSwUserAuthorization *userAuthInstance = CSwUserAuthorization::GetInstance();
	// TODO: Support revise
	/*bool isAuthorized = userAuthInstance->CheckUserAuthorization(fileName, fileRight);
	if (!isAuthorized) {
		wstring fileNameWithExt = FileUtils::GetFileNameWithExt(fileName);
		authResultObj.SetFileRight(fileRight);
		authResultObj.AddFilesToDenyAccessList(fileNameWithExt);
		authResultObj.SetMessageType(MsgType::MSG_ERROR);
	}
	CSwRMXMgr::GetInstance()->LogFileAccessResult(fileName, fileRight, isAuthorized);
	if (!authResultObj.GetAuthResult()) {
		err_code = TC_AUTHFAIL_REVISE_NOEDITRIGHT;
	}
	else {
		authResultObj.SetFileName(fileName);
		isAuthorized = userAuthInstance->PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EXPORT);
		if (!isAuthorized) {
			LOG_ERROR("User doesn't have 'SaveAs or Download' permission this action on File = " << fileName);
		}
		if (!authResultObj.GetAuthResult()) {
			err_code = TC_AUTHFAIL_REVISE_NOSAVEASRIGHT;
		}
	}*/
	authResultObj.SetFileName(fileName);
	if (fileRight == FileRight::RMX_RIGHT_EDIT)
	{
		bool isAuthorized = userAuthInstance->PerformFileAccessCheck(authResultObj, fileRight);
		if (!isAuthorized) {
			LOG_ERROR("User doesn't have 'Save As' permission this action on File = " << fileName);
		}
		if (!authResultObj.GetAuthResult()) {
			err_code = TC_AUTHFAIL_RIGHTEDIT;
		}
		/*else {
			if (IsSaveJTFilesEnabled()) {
				isAuthorized = userAuthInstance->PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EXPORT);
				if (!authResultObj.GetAuthResult()) {
					err_code = TC_AUTHFAIL_EDIT_NOJTSAVERIGHT;
				}
			}
		}*/
	}
	else
	{
		bool isAuthorized = userAuthInstance->PerformFileAccessCheck(authResultObj, fileRight);
		if (!isAuthorized) {
			LOG_ERROR("User doesn't have 'Save As' permission this action on File = " << fileName);
		}
		if (!authResultObj.GetAuthResult()) {
			err_code = TC_AUTHFAIL_RIGHTDOWNLOAD;
		}
		else if(checkEditOnDirty) {
			isAuthorized = userAuthInstance->PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EDIT);
			if (!authResultObj.GetAuthResult()) {
				err_code = TC_AUTHFAIL_DOWNLOAD_NORIGHTEDIT;
			}
		}
	}
	if (!authResultObj.GetAuthResult()) {
		authResultObj.SetFileRight(fileRight);
	}

	return err_code;
}

int CSwRMXSwimInterface::ProcessSaveNewResponse(const string& xmlpath) {
	LOG_INFO("Response XML Path" << xmlpath.c_str());
	SwimXMLParser parser;
	vector<SwimResponseXMLObject> responseObjList;
	bool status = parser.ParseSwimResponseXML(xmlpath, responseObjList);
	if (!status) {
		LOG_ERROR("Error parsing document from xmlpath = " << xmlpath.c_str());
		return TC_FILE_PARSE_ERROR;
	}
	
	wstring hintLoc = L"";
	for (auto respObj : responseObjList) {
		wstring wsFileLocation = L"";
		wstring srcModelName = RMXUtils::s2ws(respObj.name) + L"." + RMXUtils::s2ws(respObj.type);
		status = FindWorkspaceDir(srcModelName, hintLoc, wsFileLocation);
		wstring srcFilePath = wsFileLocation + L"\\" + srcModelName;
		wstring destFilePath = wsFileLocation + L"\\" + RMXUtils::s2ws(respObj.new_model_name) + L"." + RMXUtils::s2ws(respObj.type);
		CSldWorksRMX::AddToTCDestSrcDocMap(make_pair(destFilePath, srcFilePath));
	}
	return TC_SUCCESS;
}

bool CSwRMXSwimInterface::FindWorkspaceDir(const wstring& srcFileName, wstring& hintLoc, wstring& workspaceDir) {
	//1. Check if hintLoc (populated in previous call to this function) is valid workspace for current file
	if (!hintLoc.empty()) {
		wstring filePath = hintLoc + L"\\" + srcFileName;
		if (FileUtils::IsFileExists(filePath)) {
			workspaceDir = hintLoc;
			return true;
		}
	}
	
	//2. if hintLoc is empty or not valid workspace dir for current file, then iterate through the list of reference Document Locations to find one.
	wstring referencedDocumentsLoc = L"";
	CSwUtils::GetInstance()->GetSwUserPreferenceStringValue(swFileLocationsDocuments, referencedDocumentsLoc);

	if (!referencedDocumentsLoc.empty()) {
		vector<wstring> refDocLoc;
		FileUtils::wsTokenizer(referencedDocumentsLoc, L";", refDocLoc);
		for (const auto& loc : refDocLoc) {
			wstring srcFilePath = loc + L"\\" + srcFileName;
			if (FileUtils::IsFileExists(srcFilePath)) {
				workspaceDir = loc;
				hintLoc = loc;
				return true;
			}
		}
	}

	//3. Check if the file pointed by srcFileName is opened in the memory and fetch the full path of the fileName
	wstring fileName = L"";
	CComPtr<IModelDoc2> modelDocPtr = CSwUtils::GetInstance()->IsFileOpened2(srcFileName);
	if (modelDocPtr != NULL) {
		LOG_INFO_FMT("File %s is opened in current SolidWorks session", srcFileName.c_str());
		fileName = CSwUtils::GetInstance()->GetModelNameForModelDoc(modelDocPtr);
		if (!fileName.empty()) {
			workspaceDir = FileUtils::GetParentPath(fileName);
			hintLoc = workspaceDir;
			return true;
		}
		LOG_ERROR("File pointed by fileName couldn't be searched in open documents in memory" << srcFileName.c_str());
	}
	else {
		LOG_ERROR("File pointed by fileName couldn't be found. This could be in the case of new files" << srcFileName.c_str());
	}

	return false;
}