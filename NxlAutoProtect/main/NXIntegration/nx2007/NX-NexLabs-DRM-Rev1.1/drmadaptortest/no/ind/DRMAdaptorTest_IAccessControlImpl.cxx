/*==================================================================================================

                           Copyright 2021 Siemens
                            All rights reserved

====================================================================================================
File description:
    Test DRM Adaptor implementation of DISW::DRM::Adaptor::IAccessControl

====================================================================================================
Date         Name                    Description of Change

$HISTORY$
==================================================================================================*/
#include <unidefs.h>

#include <cfi.h>
#include <DRMAdaptorTest_Errors.hxx>
#include <DRMAdaptorTest_Simulator.hxx>
#include <DRMAdaptorTest_Simulator_Types.hxx>
#include <error.h>
#include <UString.hxx>
#include <UStringBuilder.hxx>

#include <drmadaptortestprivate/DRMAdaptorTest_IAccessControlImpl.hxx>


using namespace UGS;

//----------------------------------------------------------------------------------------------------------------------------------
void DRMAdaptorTest::IAccessControlImpl::PrepareFileForLoad(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo)
{
    // NextLabs can implement file preprocessing here.
}

//----------------------------------------------------------------------------------------------------------------------------------
int DRMAdaptorTest::IAccessControlImpl::CheckFileAccess(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo, DISW::DRM::Adaptor::FileOperation OperationType)
{
    std::wstring wFileSpec = fileInfo->GetFileSpecification();
    UString usFileSpec(wFileSpec);
    
    int retErrorCode = ERROR_OK;
    switch (OperationType)
    {
    case DISW::DRM::Adaptor::FileOperation::READ:
        if (!DRMAdaptorTest::Simulator::HasAccess(usFileSpec, DRM_FILE_ACCESS_READ))
        {
            retErrorCode = DRM_ADAPTOR_TEST_Error_not_authorized_to_open_part;
        }
        break;
    case DISW::DRM::Adaptor::FileOperation::SAVE:
    case DISW::DRM::Adaptor::FileOperation::WRITE:
        if (!DRMAdaptorTest::Simulator::HasAccess(usFileSpec, DRM_FILE_ACCESS_WRITE))
        {
            retErrorCode = DRM_ADAPTOR_TEST_Error_not_authorized_to_save_part;
        }
        break;
    case DISW::DRM::Adaptor::FileOperation::EXPORT:
        if (!DRMAdaptorTest::Simulator::HasAccess(usFileSpec, DRM_FILE_ACCESS_EXPORT))
        {
            retErrorCode = DRM_ADAPTOR_TEST_Error_not_authorized_to_export_part;
        }
        break;
    case DISW::DRM::Adaptor::FileOperation::COPY:

        break;

    default:
        //??
        break;
    }

    return retErrorCode;
}

//----------------------------------------------------------------------------------------------------------------------------------
int DRMAdaptorTest::IAccessControlImpl::ApplyFileAccessControl
(
    std::shared_ptr<const DISW::DRM::Host::IFileInformation> sourceFile,
    std::shared_ptr<const DISW::DRM::Host::IFileInformation> producedFile
)
{
    int retErrorCode = ERROR_OK;
    UString uSourceFile(sourceFile->GetFileSpecification());
    UString uProducedFile(producedFile->GetFileSpecification());
    if (!DRMAdaptorTest::Simulator::CopyAccess(uSourceFile, uProducedFile))
        retErrorCode = DRM_ADAPTOR_TEST_Error_failed_to_apply_access_rights;
    
    return retErrorCode;
}

//----------------------------------------------------------------------------------------------------------------------------------
int DRMAdaptorTest::IAccessControlImpl::GetErrorMessage(int errorCode, const char* locale, std::wstring& message)
{
    UString error_message(DRMAdaptorTest::DecodeError(errorCode, locale));
    message = std::wstring(error_message.begin(), error_message.end());
    return ERROR_OK;
}

//----------------------------------------------------------------------------------------------------------------------------------
bool DRMAdaptorTest::IAccessControlImpl::IsEncrypted(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo)
{
    UString usFileSpec(fileInfo->GetFileSpecification());
    return DRMAdaptorTest::Simulator::IsEncrypted(usFileSpec);
}
