//
// This is a part of the NextLabs CADRMX Client SDK.
//
// Copyright (C) 2020 NextLabs, Inc. All Rights Reserved.
//
#pragma once

#include <wtypes.h>

#include "RMXLResult.h"
#include "RMXLTypeDef.h"

class IRMXUser;

/*!
* \file RMXLInstance.h
* \class IRMXInstance
*
* This interface defines the RMX instance
*
*/
class CADRMX_API IRMXInstance
{
public:
	IRMXInstance() {};
	virtual ~IRMXInstance() {};

public:
	/**
	* Checks if the initialization of RMX instance has finished
	*
	* \param [out] finished true if intialization has finished.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	*/
	virtual RMXResult IsInitFinished(bool& finished) = 0;

	/**
	* Adds RPM secure folder
	*
	* \param [in] dirPath Directory full path.
	* \param [in] option \ref RMXRPMFolderOption. Defaults to RMX_RPMFOLDER_OVERWRITE|RMX_RPMFOLDER_API
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \see RMXRPMFolderOption
	*/
	virtual RMXResult AddRPMDir(const std::wstring& dirPath, uint32_t option = (RMXRPMFolderOption::RMX_RPMFOLDER_OVERWRITE | RMXRPMFolderOption::RMX_RPMFOLDER_API)) = 0;

	/**
	* Removes RPM secure folder
	*
	* \param [in] dirPath Directory path.
	* \param [in] force True to force to remove even though the trusted process is still running. Otherwise, remove will fail. Defaults to true.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	*/
	virtual RMXResult RemoveRPMDir(const std::wstring& dirPath, bool force = true) = 0;

	/**
	* Checks if the specified directory is a RPM secure folder
	*
	* \param [in] dirPath Directory path.
	* \param [out] isRPMDir True if it is RPM folder.
	* \param [out] relation \ref RMX_RPMFolderRelation type.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \see RMX_RPMFolderRelation
	*/
	virtual RMXResult IsRPMFolder(const std::wstring& dirPath, bool& isRPMDir, RMX_RPMFolderRelation* relation = nullptr) = 0;

	/**
	* Registers an application as rpm trusted app
	*
	* \param [in] appPath Full path of an executable image.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	*/
	virtual RMXResult RegisterApp(const std::wstring& appPath) = 0;

	/**
	* Unregisters an application as rpm trusted app
	*
	* \pre
	* 	  RegisterApp is called to register caller application as RPM trusted application.
	* 	  
	* \param [in] appPath Full path of an executable image.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	*/
	virtual RMXResult UnregisterApp(const std::wstring& appPath) = 0;

	/**
	* Checks if an application is registered as rpm trusted application
	*
	* \param [in] appPath Full path of an executable image.
	* \param [out] registered True if the executable image is registered as rpm trusted application.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	*/
	virtual RMXResult IsAppRegistered(const std::wstring& appPath, bool& registered) = 0;

	/**
	* Notifies RPM driver to update the trusted status of caller process
	*
	* \pre
	* 	  RegisterApp is called to register caller application as RPM trusted application.
	*
	* \param [in] running True to notify RMX starts to run. False to notify RMX gets stopped.
	*
	* @returns RMXResult::GetCode() returns 0 if succeeded. Otherwise, returns error code.
	*
	*/
	virtual RMXResult NotifyRMXStatus(bool running) = 0;

	/**
	 * Adds an application to the non-persistent trusted application list
	 * 
	 * \pre
	 * 	  The caller process is a trusted process.
	 * 	  
	 * \param [in] appPath Full path of an executable image.
	 * 
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	 */
	virtual RMXResult AddTrustedApp(const std::wstring &appPath) = 0;

	/**
	* Removes an application from the non-persistent trusted application list
	*
	* \pre
	* 	  The caller process is a trusted process.
	*
	* \param [in] appPath Full path of an executable image.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*/
	virtual RMXResult RemoveTrustedApp(const std::wstring &appPath) = 0;

	/**
	* Copies NXL file to an RPM folder for editing with native file name
	*
	* \param [in] nxlFilePath File path of the NXL file to be edited.
	* \param [in, out] destPath As input, specify an RPM folder path. If empty, use RPM secret directory. As output, return the generated NXL file path without .nxl extension.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \note \ref EditCopyFile is used to copy NXL file into RPM folder, so that native application can edit protected file transparently.
	*/
	virtual RMXResult EditCopyFile(const std::wstring& nxlFilePath, std::wstring& destPath) = 0;

	/**
	* Re-protects the protected file in RPM folder back to original NXL file
	*
	* \param [in] filePath Native file path to be updated.
	* \param [in] originalNXLfilePath File path of original NXL file. If empty, search local mapping or check current folder.
	* \param [in] deleteSource True to delete original NXL file. Defaults to false.
	* \param [in] exitEdit Defaults to 0.\n
	* 			  0: Not exit and save \n
	* 			  1: Not exit but save \n
	* 			  2: Exit but not save \n
	* 			  3: Exit and save
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \note \ref EditSaveFile is used to re-protect the protected file to have recent modification. 
	*/
	virtual RMXResult EditSaveFile(const std::wstring& filePath, const std::wstring& originalNXLfilePath = L"", bool deleteSource = false, uint32_t exitEdit = 0) = 0;

	/**
	* Copies Nxl file in RPM folder
	*
	* \param [in] existingNxlFilePath The name of an existing NXL file to be copied.
    * \param [in] newNxlFilePath The name of the new NXL file. If the specified new file already exists, the function overwrites the existing file and succeeds.
	* \param [in] deleteSource True to delete the source NXL file. Defaults to false.
	* 			  
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \note \ref RPMCopyFile is used to copy nxl file by trusted process in RPM folder. By default, using Windows API to copy file by trusted process 
	* 		will copy the corresponding plain file.
	*/
	virtual RMXResult RPMCopyFile(const std::wstring& existingNxlFilePath, const std::wstring& newNxlFilePath, bool deleteSource = false) = 0;

	/**
	* Deletes NXL file in RPM folder
	*
	* \param [in] filePath File path to be deleted.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* @\note \ref RPMDeleteFile must be called to let RPM driver delete NXL file in RPM folder clearly, including cache.
	*/
	virtual RMXResult RPMDeleteFile(const std::wstring& filePath) = 0;

	/**
	* Gets rights for the specified file
	*
	* \param [in] filePath File path to be evaluated.
	* \param [out] rights Vector of \ref RMXFileRight.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \see RMXFileRight
	*/
	virtual RMXResult GetFileRights(const std::wstring& filePath, std::vector<RMXFileRight>& rights) = 0;

	/**
	* Checks if the specified rights are granted to the file
	*
	* \param [in] filePath File path to be evaluated.
	* \param [in] right RMXFileRight to be checked.
	* \param [out] allowed True if the rights are granted.
	* \param [in] audit True to add activity log into SkyDRM. Defaults to false.
	* For auditing, either call \ref IRMXUser::AddActivityLog on demand or set this parameter to true when checking the rights. 
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \since 2020.08
	* 
	* \see RMXFileRight
	* \see IRMXUser::AddActivityLog
	*/
	virtual RMXResult CheckFileRight(const std::wstring& filePath, RMXFileRight right, bool& allowed, bool audit = false) = 0;

	/**
	* Gets status for the specified file, such as if it is in RPM folder, and if it is protected
	*
	* \param [in] filePath File path to be evaluated.
	* \param [out] isProtected True if file is protected.
	* \param [out] dirRelation \ref RMX_RPMFolderRelation type.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \see RMX_RPMFolderRelation
	*/
	virtual RMXResult GetFileStatus(const std::wstring& filePath, bool& isProtected, RMX_RPMFolderRelation* dirRelation = nullptr) = 0;

	/**
	* Gets tags for the specified file
	*
	* \param [in] filePath File path to be evaluated.
	* \param [out] tags File tags(json-format string), such as {"ip_classification":["secret"],"gov_classification:["test"]}
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*/
	virtual RMXResult ReadFileTags(const std::wstring& filePath, std::string& tags) = 0;

	/**
	* Gets information for the specified file
	*
	* \param [in] filePath File path to be evaluated.
	* \param [out] duid DUID of NXL file.
	* \param [out] tags File tags(json-format string), such as {"ip_classification":["secret"],"gov_classification:["test"]}
	* \param [out] tokengroup Token group name.
	* \param [out] creatorid owner Id.
	* \param [out] infoext File information.
	* \param [out] attributes File system attributes. Refer to Win32 API GetFileAttributes for more information.
	* \param [out] relation \ref RMX_RPMFolderRelation type.
	* \param [out] isNXLFile True if file is protected.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \since 2020.08
	* 
	* \see RMX_RPMFolderRelation
	*/
	virtual RMXResult RPMGetFileInfo(const std::wstring & filepath, std::string &duid, std::string &tags,
		std::string &tokengroup, std::string &creatorid, std::string &infoext, DWORD &attributes, RMX_RPMFolderRelation & dirRelation, BOOL &isNXLFile) = 0;

	/**
	* Sets the attributes for a NXL file
	*
	* \param [in] nxlFilePath The name of the file whose attributes are to be set.
	* \param [in] dwFileAttributes The file attributes to set for the file. This parameter can be one or more values, the values same like Win32 API SetFileAttributes: \n
	*		FILE_ATTRIBUTE_ARCHIVE 32 (0x20) \n
	*		FILE_ATTRIBUTE_HIDDEN 2 (0x2) \n
	*		FILE_ATTRIBUTE_NORMAL 128 (0x80) \n
	*		FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 8192 (0x2000) \n
	*		FILE_ATTRIBUTE_OFFLINE 4096 (0x1000) \n
	*		FILE_ATTRIBUTE_READONLY 1 (0x1) \n
	*		FILE_ATTRIBUTE_SYSTEM 4 (0x4) \n
	*		FILE_ATTRIBUTE_TEMPORARY 256 (0x100) \n
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \since 2020.08
	* 
	* \see RPMGetFileInfo
	*/
	virtual RMXResult RPMSetFileAttributes(const std::wstring &nxlFilePath, DWORD dwFileAttributes) = 0;

	/**
	* Creates a view overlay if dynamic watermark can be found for specified tags parameters.
	*
	* \param [in] params \ref RMX_VIEWOVERLAY_PARAMS object, containing all below parameters:
	* - hTargetWnd Top window handler to show view overlay.  
	* - vecTags Vector of tags to be evaluated for dynamic watermark.  
	* - vecCtxAttrs Vector of context attributes to be evaluated for dynamic watermark, like appPath, fileName, username, etc.  
	* - displayOffsets Integer array to specify the offset effect in overlay display area, with {left,upper,right,bottom} sequence.
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	* \note Call \ref ClearViewOverlay to remove view overlay
	*
	* \see RMX_VIEWOVERLAY_PARAMS
	* \see ClearViewOverlay
	*/
	virtual RMXResult SetViewOveraly(const RMX_VIEWOVERLAY_PARAMS& params) = 0;

	/**
	* Removes view overlay
	*
	* \param [in] hTargetWnd Top window handler to show view overlay
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*
	*/
	virtual RMXResult ClearViewOverlay(void* hTargetWnd) = 0;

};
