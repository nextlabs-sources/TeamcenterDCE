
#include "stdafx.h"
#include "NXDRMIntegration.h"
#include "FileInfoCache.h"

#define RIGHT_VIEW "RIGHT_VIEW"
#define RIGHT_EDIT "RIGHT_EDIT" 
//TODO:WORKAROUND:in skydrm server, the SaveAS right is defined as RIGHT_DOWNLOAD
#define RIGHT_SAVEAS "RIGHT_DOWNLOAD"
#define RIGHT_PRINT "RIGHT_PRINT"
const char *FileOperationToRight(const DISW::DRM::Adaptor::FileOperation OperationType) {
	switch (OperationType) {
	case DISW::DRM::Adaptor::FileOperation::SAVE:// Is this file allowed to be written to and saved to under the same name
		return RIGHT_EDIT;
	case DISW::DRM::Adaptor::FileOperation::SAVEAS: // Is this file allowed to be SavedAs.
		return RIGHT_SAVEAS;
	case DISW::DRM::Adaptor::FileOperation::EXPORT: // Is this file allowed to be exported.
		return RIGHT_SAVEAS;
	case DISW::DRM::Adaptor::FileOperation::COPY:   // Is this file allowed to be copied from.
		return RIGHT_SAVEAS;
	case DISW::DRM::Adaptor::FileOperation::READ:   // Is this file allowed to be read from.
		return RIGHT_VIEW;
	case DISW::DRM::Adaptor::FileOperation::WRITE:   // Is this file allowed to be written to and saved to under the same name.
		return RIGHT_EDIT;
	default:
		return "";
	}
}

void NXL::NX::DRM::AccessControlImpl::PrepareFileForLoad(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo)
{
	DBGLOG("%s...", fileInfo->GetFileSpecification().c_str());
	RPMSession *rpmSession = NXL::NX::DRM::GetRPMSession();
	bool isprotected;
	bool isrpm;
	rpmSession->ValidateFileExtension(fileInfo->GetFileSpecification().c_str(), &isprotected, &isrpm);
	DBGLOG("%s:IsProtected=%d IsRPM=%d", fileInfo->GetFileSpecification().c_str(), isprotected, isrpm);
}
int ValidateFile(const NxlPath &filePath, bool &isProtected) {
	int retCode = DRM_ERROR_OK;
	if (!filePath.CheckFileExists()) {
		return DRM_ERROR_FILE_NOT_EXIST;
	}
	isProtected = false;
	RPMSession::RPMFolderStatus folderStatus;
	RPMSession *rpmSession = NXL::NX::DRM::GetRPMSession();
	auto rpmResult = rpmSession->CheckFileStatus(filePath.wchars(), &isProtected, &folderStatus);
	if (rpmResult != 0) {
		//convert RPM error code to DRM error code
		return RPMErrorToDRMError(rpmResult.Code());
	}
	if (!isProtected) {
		INFOLOG("'%s':error=%d(file is not protected)", filePath.tchars(), retCode);
		return retCode;
	}
	if (!RPMSession::FolderStatus_IsInRpm(folderStatus)) {
		retCode = DRM_ERROR_FILE_IS_NOT_IN_RPM;
		ERRLOG("'%s':error=%d(file not in rpm folder)", filePath.tchars(), retCode);
		return retCode;
	}
	else if (!filePath.HasNxlExtension())
	{
		auto fileWithNxlExt = filePath.EnsureNxlExtension();
		if (!fileWithNxlExt.CheckFileExists()) {
			//.nxl ext is missing from file, NX will not be able to recognize the file
			WARLOG("'%s':has no .nxl extension", filePath.tchars());
			MoveFile(filePath.tchars(), fileWithNxlExt.tchars());
			/*if (OperationType == DISW::DRM::Adaptor::FileOperation::READ) {
				return retCode = DRM_ERROR_FILE_HAS_NO_NXL_EXT;
			}*/
		}
	}
	return retCode;
}
#define ADD_PERMISSION(vec, p)	\
	if(vec->AddPermission(p)){	\
		DBGLOG("AddPermission(" #p "):success");	\
	} else {	\
		DBGLOG("AddPermission(" #p "):failed");	\
	}

int NXL::NX::DRM::AccessControlImpl::GetFilePermissions(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo, std::shared_ptr<DISW::DRM::Host::IFilePermissions> filePermissions)
{
	int retCode = DRM_ERROR_OK;
	bool isProtected;
	TRACELOG("fileInfo='%s'", fileInfo->GetFileSpecification().c_str());
	NxlPath filePath(fileInfo->GetFileSpecification().c_str());
	RPMSession *rpmSession = NXL::NX::DRM::GetRPMSession();
	string_list_p rightList = string_list_new();
	//protected file is placed in rpm dir and in good condition
	auto rpmResult = rpmSession->GetFileRights(fileInfo->GetFileSpecification().c_str(), isProtected, rightList);
	if (rpmResult == 0)
	{
		if (isProtected) {
			for (unsigned int i = 0; i < rightList->count; i++) {
				if (_stricmp(rightList->items[i], RIGHT_VIEW) == 0) {
					ADD_PERMISSION(filePermissions, DISW::DRM::Host::FilePermission::VIEW);
				}
				else if (_stricmp(rightList->items[i], RIGHT_EDIT) == 0) {
					ADD_PERMISSION(filePermissions, DISW::DRM::Host::FilePermission::EDIT);
				}
				else if (_stricmp(rightList->items[i], RIGHT_SAVEAS) == 0) {
					ADD_PERMISSION(filePermissions, DISW::DRM::Host::FilePermission::SAVEAS);
				}
				else if (_stricmp(rightList->items[i], RIGHT_PRINT) == 0) {
					ADD_PERMISSION(filePermissions, DISW::DRM::Host::FilePermission::PRINT);
				}
				else
				{
					//TODO: CLIPBOARD, EXTRACT, SCREEN_CAPTURE
				}
			}
		}
		else
		{
			ADD_PERMISSION(filePermissions, DISW::DRM::Host::FilePermission::NOT_PROTECTED);
		}		
	}
	else
	{
		//convert RPM error code to DRM error code
		retCode = RPMErrorToDRMError(rpmResult.Code());
	}
	FileInfoCache::Add(fileInfo->GetFileSpecification().c_str());
	INFOLOG("'%s': ret=%d", fileInfo->GetFileSpecification().c_str(), retCode);
	return retCode;

}
int NXL::NX::DRM::AccessControlImpl::CheckFileAccess(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo, DISW::DRM::Adaptor::FileOperation OperationType)
{
	int retCode = DRM_ERROR_OK;
	bool isProtected;
	TRACELOG("fileInfo='%s' OperationType=%d", fileInfo->GetFileSpecification().c_str(), OperationType);
	NxlPath filePath(fileInfo->GetFileSpecification().c_str());
	//check rights
	std::string rightName = FileOperationToRight(OperationType);
	if ((retCode = ValidateFile(filePath, isProtected)) == DRM_ERROR_OK
		&& isProtected)
	{
		bool allowed = false;
		RPMSession *rpmSession = NXL::NX::DRM::GetRPMSession();
		//protected file is placed in rpm dir and in good condition
		auto rpmResult = rpmSession->CheckFileRights(fileInfo->GetFileSpecification().c_str(), rightName.c_str(), allowed, true);
		if (rpmResult == 0)
		{
			if (!allowed) {
				retCode = GetErrorCodeForOperation(OperationType);
			}
			//TODO：what if exporting modified parts
		//	if (allowed
		//	&& UF_PART_is_modified(context->part_tag())
		//	&& (rightName == RIGHT_SAVEAS //TODO:remove hardcode
		//		|| rightName == RIGHT_PRINT)) {
		//	requiredRight = RIGHT_EDIT;
		//	//when exporting a modified part, the edit right is also required
		//	g_rpmSession->CheckFileRights(context->part_file().wstr(), RIGHT_EDIT, allowed, true);
		//}
		//if (!allowed) {
		//	auto dispName = name_to_display(context->part_name());
		//	DBGLOG("Current user doesn't have %s on modified Part-'%s'", requiredRight.c_str(), dispName.c_str());
		//	_denies.push_back(std::make_pair(context->part_name(), requiredRight));
		//}
		}
		else
		{
			//convert RPM error code to DRM error code
			retCode = RPMErrorToDRMError(rpmResult.Code());
		}
		FileInfoCache::Add(fileInfo->GetFileSpecification().c_str());
	}
	INFOLOG("'%s':right-'%S'(Operation=%d) ret=%d", fileInfo->GetFileSpecification().c_str(), rightName.c_str(), OperationType, retCode);
	return retCode;
}


bool NXL::NX::DRM::AccessControlImpl::IsEncrypted(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo) 
{
	bool isProtected = false;
	auto retCode = NXL::NX::DRM::GetRPMSession()->CheckFileProtection(fileInfo->GetFileSpecification().c_str(), &isProtected);
	//TODO:error handling
	DBGLOG("'%s':isProtected=%d(error=%#X)", fileInfo->GetFileSpecification().c_str(), isProtected, retCode.Code());
	if (retCode == 0) {
		return isProtected;
	}
	else
	{
		//TODO: when error
		return true;
	}
}

int NXL::NX::DRM::AccessControlImpl::ApplyFileAccessControl
(
	std::shared_ptr<const DISW::DRM::Host::IFileInformation> sourceFile,
	std::shared_ptr<const DISW::DRM::Host::IFileInformation> producedFile
) 
{
	NxlPath newFile(producedFile->GetFileSpecification().c_str());
	TRACELOG("sourceFile='%s' producedFile='%s'", sourceFile->GetFileSpecification().c_str(), producedFile->GetFileSpecification().c_str());
	RPMSession *rpmSession = NXL::NX::DRM::GetRPMSession();
	bool isSrcProtected;
	NxlMetadataCollection srcTags;
	const FileInfoCache *srcCache = FileInfoCache::Find(sourceFile->GetFileSpecification().c_str());
	if (srcCache == nullptr) {
		if (rpmSession->HelperReadFileTags(sourceFile->GetFileSpecification().c_str(), &isSrcProtected, srcTags) != 0) {
			isSrcProtected = false;
		}
	}
	else {
		isSrcProtected = srcCache->isprotected();
		srcTags = srcCache->tags();
	}
	if (isSrcProtected)
	{
		bool isTarProtected;
		if (rpmSession->ValidateFileExtension(newFile.wchars(), &isTarProtected) == 0)
		{
			if (!isTarProtected)
			{
				rpmSession->ProtectFile(newFile.wchars(), srcTags, true);
				INFOLOG("Protected:'%s'", newFile.tchars());
				if (!newFile.CheckFileExists()) {
					ERRLOG("FileNotInRPMFolder:'%s'", newFile);
					return DRM_ERROR_EXPORT_FOLDER_NOT_RPM;
				}
			}
			else
			{
				INFOLOG("AlreadyProtected:'%s'", producedFile->GetFileSpecification().c_str());
			}
		}
	}
	else
	{
		INFOLOG("SourceNotProtected:'%s'", sourceFile->GetFileSpecification().c_str());
	}
	return 0;

}