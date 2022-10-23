#include "stdafx.h"
#include <windows.h>
#include <madCHook.h>
#include "SwHooks.h"
#include "FileUtils.h"
#include "SwRMXMgr.h"
#include "SldWorksRMX.h"
#include "SwFileTypes.h"
#include "SwUtils.h"
#include "SwUserAuthorization.h"
#include "SwResult.h"
#include "SwRMXDialog.h"
#include <gdiplus.h>
#include "SwimUtils.h"
#include "SwWndHooks.h"

using namespace Gdiplus;

#pragma region HOOK_DEFS
//the original api address which has not be hooked
#define VAR_ORIGINAL(api) api##_original
//the original api address which has be hooked
#define VAR_NEXT(api) api##_next
//the customized api address which is going to replace original api
#define VAR_OURS(api) api##_hooked

#define DEFINE_HOOK_CALLBACK(api) \
static pf##api api##_original = NULL; \
static pf##api api##_next = NULL; \
static const char* api##_FULLNAME = _STRINGIZE(api); \
static bool api##InHook = false;

//check the status of the api hook
#define IS_HOOKED(api) (NULL != VAR_NEXT(api))
#define NOT_HOOKED(api) (NULL == VAR_NEXT(api))
#define IS_INIT(api) (NULL != VAR_ORIGINAL(api))
#define NOT_INIT(api) (NULL == VAR_ORIGINAL(api))

//unhook api
#define UNHOOK(api)		\
	if( IS_HOOKED(api) && UnhookCode((PVOID*)&VAR_NEXT(api))) {	\
		LOG_INFO_FMT(L"API unhooked: %s(0x%p)", _W(_STRINGIZE(api)), (PVOID)&VAR_NEXT(api)); \
		VAR_NEXT(api) = NULL;	\
	}

//hook api that we know the signature and have the corresponding lib file in which contains the ProcAddres
#define HOOK(api)	\
	if(NOT_HOOKED(api)) {	\
		VAR_ORIGINAL(api) = api;	\
		if(HookCode((PVOID)VAR_ORIGINAL(api), (PVOID)VAR_OURS(api), (PVOID*)&VAR_NEXT(api), 0))	{ \
			LOG_INFO_FMT(L"API hooked: %s(0x%p)", _W(_STRINGIZE(api)), (PVOID)&VAR_NEXT(api)); \
		} \
	}

//hook API by module name, this is used for the dll which is not loaded on SW starts
#define HOOK_API(libName, apiName)	\
	if(NOT_HOOKED(apiName)) {	\
		if(HookAPI(libName, apiName##_FULLNAME, (PVOID)VAR_OURS(apiName), (PVOID*)&VAR_NEXT(apiName), 0)) {	\
			LOG_INFO_FMT(L"API hooked: %s %s(0x%p)", _W(_STRINGIZE(libName)), _W(_STRINGIZE(apiName)), (PVOID)&VAR_NEXT(apiName)); \
		} else {	\
			LOG_INFO_FMT(L"Failed to hook API(error: %lu): %s %s", GetLastError(), _W(_STRINGIZE(libName)), _W(_STRINGIZE(apiName))); \
		}	\
	}

#pragma endregion HOOK_DEFS

#pragma region WINAPI_DEFS
/*
CreateFileA and CreateFileW
*/
using pfCreateFileA = HANDLE(WINAPI *)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

using pfCreateFileW = HANDLE(WINAPI *)(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

//
// CreateProcess
//
using pfCreateProcessA = BOOL(WINAPI *)(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);

using pfCreateProcessW = BOOL(WINAPI *)(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);


using pfCopyFileExW = BOOL (WINAPI *)(
	LPCWSTR            lpExistingFileName,
	LPCWSTR            lpNewFileName,
	LPPROGRESS_ROUTINE lpProgressRoutine,
	LPVOID             lpData,
	LPBOOL             pbCancel,
	DWORD              dwCopyFlags


);

using pfGdipSaveImageToFile = GpStatus(WINAPI *)(
	GpImage *image,
	GDIPCONST WCHAR* filename, 
	GDIPCONST CLSID* clsidEncoder, 
	GDIPCONST EncoderParameters* encoderParams
);

using pfShellExecuteExW = BOOL(WINAPI *)(
	SHELLEXECUTEINFOW *pExecInfo
);

using pfWriteFile = BOOL(WINAPI *) (
	_In_ HANDLE hFile,
		_In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer,
		_In_ DWORD nNumberOfBytesToWrite,
		_Out_opt_ LPDWORD lpNumberOfBytesWritten,
		_Inout_opt_ LPOVERLAPPED lpOverlapped
);

#pragma endregion WINAPI_DEFS

CSldWorksRMX * HookInitializer::userAddin = nullptr;


static pfCreateFileW CreateFileW_original = NULL;
static pfCreateFileW CreateFileW_next = NULL;
#define CreateFileW_FULLNAME "CreateFileW"
static HANDLE CreateFileW_hooked(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	/*if (lpFileName != nullptr) {
		LOG_INFO_FMT("lpFileName = %s, dwDesiredAccess = %d , dwShareMode = %d ,dwCreationDisposition = %d , dwFlagsAndAttributes = %d", lpFileName,
			dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes);
	}*/
	if (((dwDesiredAccess & GENERIC_WRITE) == GENERIC_WRITE && (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == CREATE_NEW))) {
		auto activeModelDoc = CSwUtils::GetInstance()->GetCurrentlyActiveModelDoc();
		if (activeModelDoc)
		{
			wstring fileName = lpFileName;
			wstring sourceExtn;
			wstring destExtn = FileUtils::GetFileExt(wstring(fileName));
			long fromDocType;
			SwFileHandlerMethod handlerType;
			activeModelDoc->GetType(&fromDocType);
			sourceExtn = CSwUtils::GetInstance()->GetModelDocTypeByEnum(fromDocType);

			if (SwFileTypes::IsSupportedFileFormat(sourceExtn, destExtn, handlerType) 
				&& handlerType == SwFileHandlerMethod::USE_CREATE_HOOK) {
				LOG_INFO_FMT("Source Extension =  %s, Destination Extension =  %s", sourceExtn.c_str(), destExtn.c_str());
				CSwRMXMgr* swRMXMgrInstance = CSwRMXMgr::GetInstance();
				wstring activeDoc = theSwUtils->GetModelNameForModelDoc(activeModelDoc);
				LOG_INFO_FMT("Active Document[%p]:'%s'", activeModelDoc, activeDoc.c_str());
				LOG_INFO("New file being created : " << fileName.c_str());

				if (!_wcsicmp(destExtn.c_str(), L".sat")) {
					CSwAuthResult authResultObj(activeDoc);
					if (!CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj)) {
						CSldWorksRMXDialog rmxDialogObj;
						rmxDialogObj.ShowMessageDialog(&authResultObj);
						return INVALID_HANDLE_VALUE;
					}
				}
				//Get the source file for the referenced document in Assembly when exporting to JT/STL format.
				if (fromDocType == swDocASSEMBLY && (!_wcsicmp(destExtn.c_str(), L".jt") || !_wcsicmp(destExtn.c_str(), L".stl"))) {

					//If referenced document is protected then protect the corresponding jt/stl file with the same level of protection as of the referenced document.
					//If referenced document is unprotected then skip protection for the destination file.
					wstring sourceFileName = CSwUtils::GetInstance()->GetSrcReferencedModelDoc(activeDoc, fileName);
					if (sourceFileName != L"")
						activeDoc = sourceFileName;
				}


				std::wstring fileTags = L"";
				if (swRMXMgrInstance->IsTagMergeRequired(wstring(activeDoc), wstring(fileName))) {
					swRMXMgrInstance->DoTagMerge(fileTags);
				}
				else if (!activeDoc.empty()) {
					//Retrieve the tags assigned to the file
					bool isTagsFound = swRMXMgrInstance->GetTags(activeDoc, fileTags);
					if (!isTagsFound) {
						LOG_WARN("Error reading tags for the file : " << activeDoc.c_str());
					}
				}
				if (!fileTags.empty()) {
					if (PathIsRelativeW(lpFileName)) {
						//The lpFileName is the relative path of the file(ProE/Creo Part,JT etc)
						wstring absPath = FileUtils::GetAbsolutePath(lpFileName);
						fileName = absPath;
					}
					FileDetails fd(fileName, fileTags);
					fd.Add(fd);
				}
			}

		}
	}

	HANDLE hnd = CreateFileW_next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	return hnd;
}


void ParseProcessCommandLine(LPWSTR lpCommandLine, LPWSTR *fileName, LPWSTR fileExt) {
	LPWSTR *szArglist;
	int nArgs;
	int i;

	szArglist = CommandLineToArgvW(lpCommandLine, &nArgs);
	if (NULL == szArglist)
	{
		LOG_ERROR("CommandLineToArgvW failed");
		return ;
	}
	else for (i = 0; i < nArgs; i++) {
		LPWSTR arg = szArglist[i];
		if (wcsstr(lpCommandLine,arg) != NULL) {
			wstring tmpFileExt = FileUtils::GetFileExt(arg);
			//Check if the extension of the file matches to the passed argument fileExt
			if (!_wcsicmp(tmpFileExt.c_str(), fileExt)) {
				*fileName = arg;
				return;
			}
		}
	}

	// Free memory allocated for CommandLineToArgvW arguments.

	LocalFree(szArglist);

}


DWORD OnBeforeCreateProcess(LPWSTR lpCommandLine) {
	DWORD eProcessRunStat = static_cast<DWORD>(PROCESS_EXEC_REASONS::PROCESS_RUN_NOACTION);
	LPWSTR fileName = nullptr;
	if (wcsstr(lpCommandLine, L"sldimport.exe") != NULL) {
		wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
		//Parse the lpCommandLine and check if any of the arguments which is a file has extension .rfa
		ParseProcessCommandLine(lpCommandLine, &fileName, L".rfa");
		if (fileName != nullptr) {
			CSwAuthResult authResultObj(activeDoc);
			if (!CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj)) {
				CSldWorksRMXDialog rmxDialogObj;
				rmxDialogObj.ShowMessageDialog(&authResultObj);
				eProcessRunStat = static_cast<DWORD>(PROCESS_EXEC_REASONS::PROCESS_NO_RUN);
				return eProcessRunStat;
			}
			eProcessRunStat = static_cast<DWORD>(PROCESS_EXEC_REASONS::PROCESS_POST_FILEPROTECT) | static_cast<DWORD>(PROCESS_EXEC_REASONS::WAIT_FOR_PROCESS_COMPLETION);
		}
	}
	return eProcessRunStat;
}



bool OnAfterCreateProcess(LPWSTR lpCommandLine, DWORD action) {
	//This is function is to ensure if there are task needs to be performed post process execution

	if ((action & static_cast<DWORD>(PROCESS_EXEC_REASONS::PROCESS_POST_FILEPROTECT)) == static_cast<DWORD>(PROCESS_EXEC_REASONS::PROCESS_POST_FILEPROTECT)) {
		std::wstring fileTags;
		LPWSTR fileName = nullptr;
		CSwRMXMgr * swRMXMgrInstance = CSwRMXMgr::GetInstance();
		wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
		if (wcsstr(lpCommandLine, L"sldimport.exe") != NULL) {
			//Parse the lpCommandLine and check if any of the arguments which is a file has extension .rfa
			ParseProcessCommandLine(lpCommandLine, &fileName, L".rfa");
		}
		if (fileName != nullptr) {
			bool isTagMerged = swRMXMgrInstance->IsTagMergeRequired(wstring(activeDoc), wstring(fileName));
			if (isTagMerged) {
				swRMXMgrInstance->DoTagMerge(fileTags);
			}
			else {
				//Retrieve the tags assigned to the file
				bool isTagsFound = swRMXMgrInstance->GetTags(activeDoc, fileTags);
				if (!isTagsFound) {
					LOG_WARN("Error reading tags for the file : " << activeDoc.c_str());
				}
			}
			if (!fileTags.empty())
			{
				LOG_INFO("File Protection Begin :" << fileName);
				LOG_INFO_FMT("SourceFileName=%s , TargetFileName = %s , IsTagMerged = %d", activeDoc.c_str(), fileName, isTagMerged);
				bool status = swRMXMgrInstance->ProtectFile(fileName, fileTags);
				if (!status) {
					LOG_ERROR("Failed to protect the file = " << fileName);
				}
				LOG_INFO("File Protection End :" << fileName);
			}
		}
	}
	return true;
}



static pfCreateProcessW CreateProcessW_original;
static pfCreateProcessW CreateProcessW_next;
#define CreateProcessW_FULLNAME "CreateProcessW"
BOOL CreateProcessW_hooked(LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	LOG_INFO("Process Name : " << lpCommandLine);
	BOOL status = true;
	DWORD eProcessRunStat;
	eProcessRunStat = OnBeforeCreateProcess(lpCommandLine);
	if (eProcessRunStat != static_cast<DWORD>(PROCESS_EXEC_REASONS::PROCESS_NO_RUN)) {
		status = CreateProcessW_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
			lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		if ((eProcessRunStat & static_cast<DWORD>(PROCESS_EXEC_REASONS::WAIT_FOR_PROCESS_COMPLETION)) == static_cast<DWORD>(PROCESS_EXEC_REASONS::WAIT_FOR_PROCESS_COMPLETION)) {
			DWORD dw = WaitForSingleObject(lpProcessInformation->hProcess,INFINITE);
			LOG_INFO_FMT("Wait State = %d on lpCommandLine = %s",dw,lpCommandLine);
		}
		OnAfterCreateProcess(lpCommandLine, eProcessRunStat);
	}
	if (wcsstr(lpCommandLine, L"startswim.bat") != NULL) {
		LOG_INFO("Initalizing RMX support for swim");
		SwimUtils::InitSwim();
	}
	return status;
}
	


static pfCopyFileExW CopyFileExW_original;
static pfCopyFileExW CopyFileExW_next;
#define CopyFileExW_FULLNAME "CopyFileExW"
static BOOL CopyFileExW_hooked(
	LPCWSTR            lpExistingFileName,
	LPCWSTR            lpNewFileName,
	LPPROGRESS_ROUTINE lpProgressRoutine,
	LPVOID             lpData,
	LPBOOL             pbCancel,
	DWORD              dwCopyFlags)
{

	DWORD lasterror = 0;
	BOOL ret = 0;
	wstring sourceDoc = L"";
	bool found = false;
	LOG_DEBUG_FMT("lpExistingFileName='%s' lpNewFileName='%s'", lpExistingFileName, lpNewFileName);
	found = CSwUtils::GetInstance()->FindSourceFile(lpExistingFileName, lpNewFileName, sourceDoc);
	if (SwFileTypes::IsAutoRecoverFile(lpNewFileName)) {
		if (!sourceDoc.empty()) {
			//Check if sourceDoc is protected and doesn't contain EDIT permission
			if (!CSwUserAuthorization::GetInstance()->CheckUserAuthorization(sourceDoc, FileRight::RMX_RIGHT_EDIT)) {
				LOG_INFO_FMT("Auto Recovery File = %s will be created from its original source file = %s as source file doesn't contain EDIT permission", lpNewFileName, sourceDoc.c_str());
				ret = CopyFileExW_next(sourceDoc.c_str(), lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
				lasterror = GetLastError();
				if (!ret) {
					LOG_ERROR_FMT("CopyFileExW_next('%s','%s') returns %d(error=%lu)", sourceDoc.c_str(), lpNewFileName, ret, lasterror);
				}
				else
				{
					LOG_DEBUG_FMT("CopyFileExW_next('%s','%s') returns %d(error=%lu)", sourceDoc.c_str(), lpNewFileName, ret, lasterror);
				}
			}
		}
	}

	if (!ret) {
		ret = CopyFileExW_next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
		lasterror = GetLastError();
		if (!ret) {
			LOG_ERROR_FMT("CopyFileExW_next('%s','%s') returns %d(error=%lu)", lpExistingFileName, lpNewFileName, ret, lasterror);
		}
		else
		{
			LOG_DEBUG_FMT("CopyFileExW_next('%s','%s') returns %d(error=%lu)", lpExistingFileName, lpNewFileName, ret, lasterror);
		}
	}
	
	if (!ret) {
		//Handling case like saving to .AI files from .SLDPRT and .SLDDRW
		wstring lpExistingFileNameWithNXLExtn = wstring(lpExistingFileName) + NXL_FILE_EXT;
		if (!FileUtils::IsFileExists(lpExistingFileNameWithNXLExtn)) {
			LOG_ERROR_FMT("File %s doesn't exists", lpExistingFileNameWithNXLExtn.c_str());
		}
		else {
			LOG_DEBUG_FMT("source file-'%s' is already protected and in non-rpm folder", lpExistingFileName);
			/*if (_wrename(lpExistingFileNameWithNXLExtn.c_str(), lpExistingFileName) != 0) {
				LOG_ERROR_FMT("Renaming failed .Source File = %s, Destination File = %s ", lpExistingFileNameWithNXLExtn, lpExistingFileName);
				return ret;
			}*/
			wstring newNxlFile = wstring(lpNewFileName) + NXL_FILE_EXT;
			LOG_DEBUG_FMT("Copying '%s' to '%s'", lpExistingFileNameWithNXLExtn.c_str(), newNxlFile.c_str());
			ret = CopyFileExW_next(lpExistingFileNameWithNXLExtn.c_str(), newNxlFile.c_str(), lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
			lasterror = GetLastError();
		}
	}
	else if (ret 
		&& FileUtils::IsFile(lpNewFileName) 
		&& found) {

		//Check if file format supported
		wstring fromFileExt = FileUtils::GetFileExt(sourceDoc);
		wstring toFileExt = FileUtils::GetFileExt(lpNewFileName);
		if (sourceDoc.empty()) {
			//active doc is new document
			auto activeModelDoc = theSwUtils->GetCurrentlyActiveModelDoc();
			if (activeModelDoc) {
				long docType;
				auto activeFilePath = theSwUtils->GetModelNameForModelDoc(activeModelDoc);
				activeModelDoc->GetType(&docType);
				fromFileExt = theSwUtils->GetModelDocTypeByEnum(docType);
				LOG_DEBUG_FMT("ActiveModel[%p]:'%s' docType=%d ext=%s", activeModelDoc.p, activeFilePath.c_str(), docType, fromFileExt.c_str());
			}
		}

		SwFileHandlerMethod handlerType;
		if (SwFileTypes::IsSupportedFileFormat(fromFileExt, toFileExt, handlerType) && handlerType == SwFileHandlerMethod::USE_COPY_HOOK) {
			LOG_INFO_FMT("lpExistingFileName=%s , lpNewFileName = %s", lpExistingFileName, lpNewFileName);
	
			//Protect File
			std::wstring fileTags = L"";
			CSwRMXMgr * swRMXMgrInstance = CSwRMXMgr::GetInstance();
			bool isTagMerged = swRMXMgrInstance->IsTagMergeRequired(wstring(sourceDoc), wstring(lpNewFileName));
			if (isTagMerged) {
				CSwRMXMgr::GetInstance()->DoTagMerge(fileTags);
			}
			else if(!sourceDoc.empty()) {
				//Retrieve the tags assigned to the file
				bool isTagsFound = swRMXMgrInstance->GetTags(sourceDoc, fileTags);
				if (!isTagsFound) {
					LOG_WARN("Error reading tags for the file : " << sourceDoc);
				}
			}

			if (!fileTags.empty()) {
				LOG_INFO("File Protection Begin :" << lpNewFileName);
				LOG_INFO_FMT("SourceFileName='%s' , TargetFileName = %s , IsTagMerged = %d", sourceDoc.c_str(), lpNewFileName, isTagMerged);
				bool status = swRMXMgrInstance->ProtectFile(lpNewFileName, fileTags);
				if (!status) {
					LOG_ERROR("Failed to protect the file : " << lpNewFileName);
				}
				LOG_INFO("File Protection End :" << lpNewFileName);
				//Add the Save As folder as rpm directory if not already set.
				//This is required because for some newly saved file, SolidWorks immediately tries to open it in the same or new window.
				wstring fileDir = FileUtils::GetParentPath(lpNewFileName);
				if (SwFileTypes::isNativeFileFormat(toFileExt) && !swRMXMgrInstance->RMX_IsRPMFolder(fileDir)) {
					swRMXMgrInstance->RMX_AddRPMFolder(fileDir);
					HookInitializer::GetUserAddin()->RPMDirList.push_back(fileDir);
				}
			}
		}
	}
	SetLastError(lasterror);
	LOG_INFO_FMT("CopyFileExW('%s','%s') return %d(lasterror=%lu)", lpExistingFileName, lpNewFileName, ret, lasterror);
	return ret;
	
}


using JtkHierarchy_ptr = void* ;
using JtkCADExporter_ptr =  void* ;
#define JtkCADExporter_exportJt_FULLNAME "?exportJt@_JtkCADExporter@@UEAAHPEAVJtkHierarchy@@@Z"
#define JtkCADExporter_exportJt_ORDINAL 2485
using pfJtkCADExporter_exportJt = int(*)(JtkCADExporter_ptr, JtkHierarchy_ptr);
static pfJtkCADExporter_exportJt JtkCADExporter_exportJt_original;
static pfJtkCADExporter_exportJt JtkCADExporter_exportJt_next;
static int JtkCADExporter_exportJt_hooked(JtkCADExporter_ptr exporter, JtkHierarchy_ptr hier)
{
	if (CSwWndHook::IsJTQuickExportRunning())
	{
		LOG_DEBUG(L"Checking right before JT exported...");
		wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
		CSwAuthResult authResultObj(activeDoc);
		bool isAuthorized = CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj);
		if (!isAuthorized) {
			CSldWorksRMXDialog rmxDialogObj;
			rmxDialogObj.ShowMessageDialog(&authResultObj);
			LOG_ERROR("Unauthorized to perform this operation. Cancel JT export");
			return 0;
		}
	}
	int ret = JtkCADExporter_exportJt_next(exporter, hier);
	LOG_INFO("Calling ProcessRecords()");
	HookInitializer::GetUserAddin()->ProcessRecords();
	return ret;
}


static pfGdipSaveImageToFile GdipSaveImageToFile_original = NULL;
static pfGdipSaveImageToFile GdipSaveImageToFile_next = NULL;
#define GdipSaveImageToFile_FULLNAME "GdipSaveImageToFile"
static GpStatus WINAPI GdipSaveImageToFile_hooked(
	GpImage *image, GDIPCONST WCHAR* filename, GDIPCONST CLSID* clsidEncoder, GDIPCONST EncoderParameters* encoderParams)
{
	GpStatus status =  GdipSaveImageToFile_next(image,filename,clsidEncoder,encoderParams);
	wstring destFileExtn = FileUtils::GetFileExt(filename);
	if (!_wcsicmp(destFileExtn.c_str(), L".png")) {
		//Preview files are of the form : SamplePartp.PNG,SampleAssemblya.PNG,SampleDrawingd.PNG
		wstring destfileName = FileUtils::GetFileName(filename); //SamplePartp
		wchar_t typeIdentifier = destfileName.back();
		wstring modelType = L"";
		if (typeIdentifier == 'p')
			modelType = L".SLDPRT";
		else if (typeIdentifier == 'a')
			modelType = L".SLDASM";
		else if (typeIdentifier == 'd')
			modelType = L".SLDDRW";
		else
			return status;

		wstring modelName = destfileName.substr(0, destfileName.size() - 1); //SamplePart		
		wstring searchFileName = modelName + modelType; //SamplePart.SLDPRT

		wstring sourceDoc = L"";
		//check if the searchFileName matches any of the file opened in current solidworks session.
		CComPtr<IModelDoc2> modelDoc = CSwUtils::GetInstance()->IsFileOpened2(searchFileName);
		if (modelDoc != NULL) {
			LOG_INFO_FMT("File %s is opened in current SolidWorks session",searchFileName.c_str());
			CComBSTR modelDocName;
			modelDoc->GetPathName(&modelDocName);
			sourceDoc = FileUtils::bstr2ws(modelDocName);
		}
		else {
			//Check if the file with name searchFileName exists on the workspace folder;
			wstring workspaceDir = L"";
			if (CSwUtils::GetInstance()->GetSwUserPreferenceStringValue(swFileLocationsDocuments, workspaceDir)) {
				vector<wstring> refDocLoc;
				FileUtils::wsTokenizer(workspaceDir, L";", refDocLoc);
				for (const auto &loc : workspaceDir) {
					wstring pathToFile = loc + L"\\" + searchFileName;
					if (FileUtils::IsFileExists(pathToFile)) {
						LOG_INFO_FMT("File named %s exists in the workspace folder : ", pathToFile.c_str());
						sourceDoc = pathToFile;
						break;
					}
				}
			}
		}
		if (!sourceDoc.empty()) {
			bool isProtected = false;
			if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(sourceDoc, &isProtected) && isProtected) {
				wstring fileTags = L"";
				bool isTagsFound = CSwRMXMgr::GetInstance()->GetTags(sourceDoc, fileTags);
				if (!isTagsFound) {
					LOG_WARN("Error reading tags for the file : " << sourceDoc.c_str());
				}
				LOG_INFO("File Protection Begin :" << filename);
				LOG_INFO_FMT("SourceFileName=%s , TargetFileName = %s", sourceDoc.c_str(), filename);
				if (!CSwRMXMgr::GetInstance()->ProtectFile(filename, fileTags)) {
					LOG_ERROR("Error protecting file = " << filename);
					return status;
				}
				LOG_INFO("File Protection End :" << filename);
				//Set the %tmp% folder(or folder configured in swim.xml) as RPM. This is to ensure preview files can be viewed by trusted java.exe process in Save dialogs
				/*wstring fileParentPath = FileUtils::GetParentPath(filename);
				if (!CSwRMXMgr::GetInstance()->RMX_IsRPMFolder(fileParentPath)) {
					CSwRMXMgr::GetInstance()->RMX_AddRPMFolder(fileParentPath);
					HookInitializer::GetUserAddin()->RPMDirList.push_back(fileParentPath);
				}*/
			}
		}
	}
	
	return status;
}

bool CheckPrintRight()
{
	CSwAuthResult authResultObj;
	wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
	authResultObj.SetFileName(activeDoc);
	if (!CSwUserAuthorization::GetInstance()->OnFilePrintAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		LOG_ERROR("User is not authorized to perform Print Preview operation. FileName = " << authResultObj.FileName());

		return false;
	}

	return true;
}

#define OnPrintPreview_FULLNAME "?OnPrintPreview@uiModelView_c@@QEAAXXZ"
#define OnPrintPreview_ORDINAL 5117
using uiModelView_c_ptr = void *;
using pfOnPrintPreview = void(*)(uiModelView_c_ptr);
static pfOnPrintPreview OnPrintPreview_original;
static pfOnPrintPreview OnPrintPreview_next;
static void OnPrintPreview_hooked(uiModelView_c_ptr p)
{
	LOG_DEBUG(L"OnPrintPreview_hooked");
	if (CheckPrintRight())
	{
		OnPrintPreview_next(p);
	}
}


#define OnCtrlFilePrint_FULLNAME "?OnCtrlFilePrint@uiModelView_c@@MEAAXXZ"
#define OnCtrlFilePrint_ORDINAL 4588
using pfOnCtrlFilePrint = void(*)(uiModelView_c_ptr);
static pfOnCtrlFilePrint OnOnCtrlFilePrint_original;
static pfOnCtrlFilePrint OnCtrlFilePrint_next;
static void OnCtrlFilePrint_hooked(uiModelView_c_ptr p)
{
	LOG_DEBUG(L"OnCtrlFilePrint_hooked");
	if (CheckPrintRight()) {
		OnCtrlFilePrint_next(p);
	}
}

// void uiView_c::OnFilePrint(class CView *)
#define uiViewOnFilePrint_FULLNAME "?OnFilePrint@uiView_c@@SAXPEAVCView@@@Z"
#define uiViewOnFilePrint_ORDINAL 2142
using CView_ptr = void *;
using pfuiViewOnFilePrint = void(*)(CView_ptr);
static pfuiViewOnFilePrint uiViewOnFilePrint_original;
static pfuiViewOnFilePrint uiViewOnFilePrint_next;
static void uiViewOnFilePrint_hooked(CView_ptr pView)
{
	LOG_DEBUG(L"uiViewOnFilePrint_hooked");
	if (CheckPrintRight()) {
		uiViewOnFilePrint_next(pView);
	}
}

using pfPageSetupDlgW = BOOL(WINAPI *)(_Inout_ LPPAGESETUPDLG lppsd);
static pfPageSetupDlgW PageSetupDlgW_original = NULL;
static pfPageSetupDlgW PageSetupDlgW_next = NULL;
#define PageSetupDlgW_FULLNAME "PageSetupDlgW"
static BOOL PageSetupDlgW_hooked(_Inout_ LPPAGESETUPDLG lppsd) {
	LOG_DEBUG(L"PageSetupDlgW_hooked");

	if (!CheckPrintRight()) {
		return false;
	}
	return PageSetupDlgW_next(lppsd);
}

using uiModelDoc_c_ptr = void *;
#define OnSaveDocument_FULLNAME "?OnSaveDocument@uiModelDoc_c@@UEAAHPEB_W@Z"
#define OnSaveDocument_ORDINAL 5179
using pfOnSaveDocument = int(*)(uiModelDoc_c_ptr,wchar_t const *);
static pfOnSaveDocument OnSaveDocument_original;
static pfOnSaveDocument OnSaveDocument_next;
static int OnSaveDocument_hooked(uiModelDoc_c_ptr p, wchar_t const* fp)
{
	wstring destFileExtn = FileUtils::GetFileExt(fp);
	if (!_wcsicmp(destFileExtn.c_str(), L".sldprt") || !_wcsicmp(destFileExtn.c_str(), L".prt")) {
		//Do the final check to see if the file with no RIGHT_EDIT permission was modified.
		if (FileUtils::IsFileExists(fp) && CSwUtils::GetInstance()->IsFileOpened(fp)) {
			//Control will reach here only when the file already exists and also currently opened in the solidworks session.
			bool isProtected = false;
			if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(fp, &isProtected) && isProtected) {
				CSwAuthResult authResultObj(fp);
				bool status = CSwUserAuthorization::GetInstance()->PerformFileAccessCheck(authResultObj, FileRight::RMX_RIGHT_EDIT);
				if (!status) {
					//Control will only reach here if SolidWorks fails to trigger swPartFileSaveNotify and file has not edit permission but was modified.
					LOG_ERROR("OnSaveDocument_hooked: The file doesn't contain EDIT permission and was modified" << fp);
					authResultObj.SetErrorMsg(L"Operation Denied.\r\nYou do not have 'EDIT' permission to perform this action on the following file(s):\r\n");
					CSldWorksRMXDialog rmxDialogObj;
					rmxDialogObj.ShowMessageDialog(&authResultObj);
					return 0;
				}
			}
		}
	}
	int ret = 0;
	try
	{
		ret = OnSaveDocument_next(p, fp);
	}
	catch (...) {
		LOG_ERROR("OnSaveDocument_hooked: EXCEPTION CAUGHT during executing OnSaveDocument_next, user may cancelled save action");
	}
	return ret;
}

static pfShellExecuteExW ShellExecuteExW_original = NULL;
static pfShellExecuteExW ShellExecuteExW_next = NULL;
#define ShellExecuteExW_FULLNAME "ShellExecuteExW"
static BOOL ShellExecuteExW_hooked(SHELLEXECUTEINFOW *pExecInfo)
{
	LOG_DEBUG("ShellExecuteExW_hooked");
	if (pExecInfo->lpFile != nullptr && (pExecInfo->lpVerb != nullptr) && !_wcsicmp(pExecInfo->lpVerb,L"open")) {
		wstring destFileExtn = FileUtils::GetFileExt(pExecInfo->lpFile);
		if (!_wcsicmp(destFileExtn.c_str(), L".jpg") || !_wcsicmp(destFileExtn.c_str(), L".png") || !_wcsicmp(destFileExtn.c_str(), L".tif") || !_wcsicmp(destFileExtn.c_str(), L".pdf")) {
			LOG_INFO_FMT("FileName = %s , lpVerb = %s", pExecInfo->lpFile,pExecInfo->lpVerb);
			wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
			bool status = CSwRMXMgr::GetInstance()->FileSaveHandler(activeDoc, pExecInfo->lpFile);
			if (CSwUtils::GetInstance()->IsFileHasNXLDoc(activeDoc))
				return FALSE;
		}
	}
	return ShellExecuteExW_next(pExecInfo);
}


#define OnEdrawingPublishToolbarButton_FULLNAME "?OnEdrawingPublishToolbarButton@CAmApp@@QEAAXXZ"
#define OnEdrawingPublishToolbarButton_ORDINAL 2114
using pfOnEdrawingPublish = void(*)(void);
static pfOnEdrawingPublish OnEdrawingPublishToolbarButton_original;
static pfOnEdrawingPublish OnEdrawingPublishToolbarButton_next;
static void OnEdrawingPublishToolbarButton_hooked(void)
{
	LOG_DEBUG("OnEdrawingPublishToolbarButton_hooked");
	CSwAuthResult authResultObj;
	wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
	authResultObj.SetFileName(activeDoc);
	if (!CSwUserAuthorization::GetInstance()->OnPublishToEDrawingCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		LOG_ERROR("OnEdrawingPublishToolbarButton_hooked: A file cannot be published to eDrawings if a file or any of its references are protected. FileName = " << authResultObj.FileName());
	}
	else {
		OnEdrawingPublishToolbarButton_next();
	}
}
#define OnLayoutPresentationToolbarButton_FULLNAME "?OnLayoutPresentationToolbarButton@dtlDoc_c@@QEAAXXZ"
using pfOnLayoutPresentationToolbarButton = void(*)(void*);
static pfOnLayoutPresentationToolbarButton OnLayoutPresentationToolbarButton_original;
static pfOnLayoutPresentationToolbarButton OnLayoutPresentationToolbarButton_next;
static void OnLayoutPresentationToolbarButton_hooked(void* obj)
{
	LOG_DEBUG_FMT("OnLayoutPresentationToolbarButton(obj=%p)...", obj);
	CSwAuthResult authResultObj;
	authResultObj.SetFileName(CSwUtils::GetInstance()->GetCurrentActiveFile());
	if (!CSwUserAuthorization::GetInstance()->DenyIfAnyProtected(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		LOG_ERROR("Publish to 3D PDF: A file cannot be published to 3d PDF if a file or any of its references are protected. FileName = " << authResultObj.FileName());
	}
	else {
		try {
			OnLayoutPresentationToolbarButton_next(obj);
		}
		catch (...) {
			LOG_ERROR("OnLayoutPresentationToolbarButton():exception caught");
		}
	}
	LOG_DEBUG_FMT("OnLayoutPresentationToolbarButton(obj=%p) ends", obj);
}
#define OnPublishSTEP242FileToolbarButton_FULLNAME "?OnPublishSTEP242FileToolbarButton@dtlDoc_c@@QEAAXXZ"
using pfOnPublishSTEP242FileToolbarButton = void(*)(void*);
static pfOnPublishSTEP242FileToolbarButton OnPublishSTEP242FileToolbarButton_original;
static pfOnPublishSTEP242FileToolbarButton OnPublishSTEP242FileToolbarButton_next;
static void OnPublishSTEP242FileToolbarButton_hooked(void* obj)
{
	LOG_DEBUG_FMT("OnPublishSTEP242FileToolbarButton(obj=%p)...", obj);
	CSwAuthResult authResultObj;
	authResultObj.SetFileName(CSwUtils::GetInstance()->GetCurrentActiveFile());
	if (!CSwUserAuthorization::GetInstance()->DenyIfAnyProtected(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		LOG_ERROR("Publish to 3D PDF: A file cannot be published to 3d PDF if a file or any of its references are protected. FileName = " << authResultObj.FileName());
	}
	else {
		try {
			OnPublishSTEP242FileToolbarButton_next(obj);
		}
		catch (...) {
			LOG_ERROR("OnPublishSTEP242FileToolbarButton():exception caught");
		}
	}
	LOG_DEBUG_FMT("OnPublishSTEP242FileToolbarButton(obj=%p) ends", obj);
}

using pfLoadLibraryW = HMODULE(*)(LPCWSTR lpLibFileName);
static pfLoadLibraryW LoadLibraryW_original = NULL;
static pfLoadLibraryW LoadLibraryW_next = NULL;
#define LoadLibraryW_FULLNAME "LoadLibraryW"
static HMODULE LoadLibraryW_hooked(LPCWSTR lpLibFileName)
{
	HMODULE hnd = LoadLibraryW_next(lpLibFileName);
	if (wcsstr(lpLibFileName, L"TranscenData.SwimMgrCtrl.DLL") != NULL) {
		//Make java.exe corresponding to swim as trusted process
		if (!SwimUtils::SetSwimPIDAsTrusted()) {
			LOG_WARN("LoadLibraryW_hooked: Process Id of java.exe corresponding to SWIM couldn't be set as trusted");
		}
	}
	return hnd;
}

DEFINE_HOOK_CALLBACK(WriteFile)
BOOL WriteFile_hooked(
	HANDLE hFile,
	LPCVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
)
{
	auto ret = WriteFile_next(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	
	const wstring& filePath = RMXUtils::GetFilePathByHandle(hFile);
	if (wcsstr(filePath.c_str(), L"client_socket.log") != NULL)
	{
		string strRecv((LPCSTR)lpBuffer);
		if (strstr(strRecv.c_str(), "OpenInBackgroundCmd") != NULL ||
			strstr(strRecv.c_str(), "SetCurrentWorkspaceCmd") != NULL ||
			strstr(strRecv.c_str(), "OpenCB") != NULL ||
			strstr(strRecv.c_str(), "OpenFromPortalCB") != NULL)
		{
			//Set the Referenced Documents folder as RPM.This location is set and used by the Teamcenter Integration for SolidWorks.
			wstring refDocDir;
			CSwUtils::GetInstance()->GetSwUserPreferenceStringValue(swFileLocationsDocuments, refDocDir);
			if (!refDocDir.empty()) {
				refDocDir += L"\\swim.txr";
				LOG_INFO("Update swim.txr file in current workspace when client_socket.log updated: " << refDocDir);
				SwimUtils::RefreshSwimRegistry(refDocDir);
			}
		}
	}
	return ret;
}

void HookInitializer::Startup()
{
	InitializeMadCHook();
	LOG_INFO(L"MadCHook initialized");
	SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);

	HOOK_API("KernelBase.dll",CreateFileW);
	if (HookInitializer::GetUserAddin()->m_osCurrMajorVer == 10) {
		//Windows 10
		HOOK_API("KernelBase.dll", CopyFileExW);
	}
	else {
		//Windows 7
		HOOK_API("Kernel32.dll", CopyFileExW);
	}
	
	HOOK_API("kernel32.dll",CreateProcessW);
	HOOK_API("JtTk101.dll", JtkCADExporter_exportJt);
	HOOK_API("gdiplus.dll", GdipSaveImageToFile);
	HOOK_API("Comdlg32.dll", PageSetupDlgW);
	HOOK_API("sldappu.dll", uiViewOnFilePrint);
	HOOK_API("slduiu.dll", OnPrintPreview);
	HOOK_API("slduiu.dll", OnCtrlFilePrint);
	HOOK_API("slduiu.dll", OnSaveDocument);
	HOOK_API("Shell32.dll", ShellExecuteExW);
	HOOK_API("sldappu.dll", OnEdrawingPublishToolbarButton);
	HOOK_API("slddtlu.dll", OnLayoutPresentationToolbarButton);//publish to 3d pdf
	HOOK_API("slddtlu.dll", OnPublishSTEP242FileToolbarButton);//publish to step
	
	InstallSwimHook();
}

void HookInitializer::Shutdown()
{	
	userAddin = nullptr;
	UNHOOK(JtkCADExporter_exportJt);
	UNHOOK(CreateFileW);
	UNHOOK(CopyFileExW)
	UNHOOK(CreateProcessW);
	UNHOOK(GdipSaveImageToFile);
	UNHOOK(PageSetupDlgW);
	UNHOOK(uiViewOnFilePrint);
	UNHOOK(OnPrintPreview);
	UNHOOK(OnCtrlFilePrint);
	UNHOOK(OnSaveDocument);
	UNHOOK(ShellExecuteExW);
	UNHOOK(OnEdrawingPublishToolbarButton);
	UNHOOK(OnLayoutPresentationToolbarButton);
	UNHOOK(OnPublishSTEP242FileToolbarButton);
	
	UninstallSwimHook();

	FinalizeMadCHook();
	LOG_INFO(L"MadCHook Finalized");
}

void HookInitializer::InstallSwimHook()
{
	HOOK_API("Kernel32.dll", WriteFile);
}

void HookInitializer::UninstallSwimHook()
{
	UNHOOK(WriteFile);
}
