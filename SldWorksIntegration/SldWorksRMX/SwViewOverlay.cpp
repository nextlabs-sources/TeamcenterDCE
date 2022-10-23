#include "SwViewOverlay.h"
#include "SwUtils.h"
#include "SwRMXMgr.h"
#include "FileUtils.h"
#include "SwUserAuthorization.h"

HWND CSwViewOverlay::_topWindow = nullptr;


CSwViewOverlay::CSwViewOverlay() {

}

CSwViewOverlay::CSwViewOverlay(wstring fileName): _fileName(fileName) {
	if (_topWindow == nullptr) {
		_topWindow = (const HWND)CSwUtils::GetInstance()->GetMainFrameWindowHandle();
	}
	//At present, RPM doesn't support overlay for child windows.
	//if (!_fileName.empty()) {
	//	_childWindow = (HWND)CSwUtils::GetInstance()->GetModelWindowHandle(FileUtils::ws2bstr(_fileName));
	//}
}

CSwViewOverlay::~CSwViewOverlay() {
	_childWindow = nullptr;
}



void CSwViewOverlay::SetViewOverlay() {
	CSwRMXMgr* swRMXMgrInstance = CSwRMXMgr::GetInstance();
	//Retrieve the tags assigned to the file
	vector<std::wstring> nxlFiles;
	bool isProtected = false;
	bool isScreenCaptureDisabled = false;
	CComPtr<IModelDoc2> modelDoc = NULL;
	if (!_fileName.empty())
	{
		modelDoc = theSwUtils->GetModelDocByFileName(_fileName);
		if (modelDoc == NULL) {
			LOG_ERROR_FMT("Failed find model doc for file-'%s'", _fileName.c_str());
		}
	}
	if (modelDoc == NULL) {
		modelDoc = theSwUtils->GetCurrentlyActiveModelDoc();
		if (modelDoc != NULL && _fileName.empty()) {
			CComBSTR activeFile;
			modelDoc->GetPathName(&activeFile);
			_fileName = FileUtils::bstr2ws(activeFile);
		}
		LOG_ERROR_FMT("Active Doc='%s'", _fileName.c_str());
	}

	if (modelDoc != NULL)
	{
		wstring realFile;
		theSwUtils->TryFindRootFileOfVirtualModel(modelDoc, realFile);
		if (!realFile.empty()
			&& CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(realFile, &isProtected)
			&& isProtected) 
		{
			nxlFiles.push_back(realFile);
			if (DisableScreenCapture(realFile))
				isScreenCaptureDisabled = true;
		}

		long docType;
		modelDoc->GetType(&docType);
		if (docType == swDocASSEMBLY || docType == swDocDRAWING) {
			//Find the tags of the referenced component also.
			vector<wstring> referencedComponents = CSwUtils::GetInstance()->GetReferencedFilesOfModelDoc(modelDoc);
			for (auto refComp : referencedComponents) {
				isProtected = false;
				if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(refComp, &isProtected) && isProtected) {
					nxlFiles.push_back(refComp);
					if (!isScreenCaptureDisabled && DisableScreenCapture(refComp))
						isScreenCaptureDisabled = true;
				}
			}
		}
	}

	if (nxlFiles.size() > 0) {
		std::vector<TEvalAttribute> contextAttrs;
		contextAttrs.push_back(make_pair(L"fileName", FileUtils::GetFileNameWithExt(_fileName)));
		if (!swRMXMgrInstance->RMX_SetViewOverlay(nxlFiles, contextAttrs, _topWindow)) {
			LOG_ERROR("Error while applying overlay to the file = " << _fileName.c_str());
			return;
		}
		LOG_INFO("Successfully applied overlay to the file = " << _fileName.c_str());
	}
	else {
		LOG_INFO("Neither file nor any of its components are protected . FileName = " << _fileName.c_str());
	}
}


void CSwViewOverlay::ClearViewOverlay() {
	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	swRMXMgrInstance->RMX_ClearViewOverlay(_topWindow);	
	RMXUtils::DisableScreenCapture(_topWindow, false);
}


void CSwViewOverlay::ResetViewOverlay() {
	ClearViewOverlay();
	SetViewOverlay();
}
void CSwViewOverlay::RefreshActiveWindow() {
	CSwViewOverlay overlay(L"");
	overlay.ResetViewOverlay();
}

bool CSwViewOverlay::DisableScreenCapture(const wstring& fileName)
{
	CSwAuthResult authResultObj(fileName);
	bool status = CSwUserAuthorization::GetInstance()->PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_DECRYPT);
	if (!status && RMXUtils::DisableScreenCapture(_topWindow, true)) {
		//Control will only reach here if SolidWorks fails to trigger swPartFileSaveNotify and file has not edit permission but was modified.
		LOG_WARN("Screen capture disabled. Extract permission not found on " << fileName.c_str());
		return true;
	}

	return false;
}
