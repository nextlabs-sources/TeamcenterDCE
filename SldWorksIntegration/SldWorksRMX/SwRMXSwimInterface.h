#pragma once
//This file is for providing the required utilies for the SwRMXSwimIntegration project.

#include "stdafx.h"
#include "SldWorksRMX.h"
#include "SwResult.h"
#include "SwimXMLParser.h"


class CSwRMXSwimInterface {

private:
	static CSldWorksRMX *userAddin;
	InputXMLObj inputXMLObj;
public:
	 CSwRMXSwimInterface();
	 ~CSwRMXSwimInterface();
	// int ProcessTCRequest(long operationType, string xmlpath, CSwAuthResult &authResultObj);
	 int ProcessSwimRequest(const string& fileName, const string& right, CSwAuthResult &authResultObj);
	 static void Init(CSldWorksRMX* addinPtr) {
		 userAddin = addinPtr;
	 }
	 bool GetAllDocumentFromXML(const string &xmlpath, InputXMLObj & inputObj);
	 bool GetTCFileFullPath(const wstring &fileName, wstring &fileNameWithPath);
	 //int PerformAccessCheckByTCSelection(const string& xmlpath, const int& operationType , const FileRight& fileRight, CSwAuthResult &authResultObj);
	 int CheckRightByTC(const wstring& fileName, FileRight fileRight, CSwAuthResult &authResultObj, bool checkEditOnDirty = true);
	 int ProcessSaveNewResponse(const string& xmlpath);
	 void FilterFileToBeProcessed(const int& operationType,vector<wstring>& fileNameList);
	 void PerformTCAccessCheck(const vector<wstring>& fileNameList, const FileRight& fileRight, CSwAuthResult &authResultObj);
	 bool IsSaveJTFilesEnabled() {
		 return inputXMLObj.saveJTFlag;
	 }
	 bool IsAnyFileBeingRevised(vector<wstring>& reviseFileList);
	 bool FindWorkspaceDir(const wstring& srcFileName, wstring& hint, wstring& workspaceDir);
};
