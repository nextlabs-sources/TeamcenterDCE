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
#ifndef DRMAdaptorTest_IAccessControlImpl_HXX_INCLUDED
#define DRMAdaptorTest_IAccessControlImpl_HXX_INCLUDED

#include <DISW_DRM.hxx>

namespace UGS
{
    namespace DRMAdaptorTest
    {
        class IAccessControlImpl : public DISW::DRM::Adaptor::IAccessControl
        {
        public:

            ~IAccessControlImpl() {}

            IAccessControlImpl() {}

            void PrepareFileForLoad(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo) override;

            int CheckFileAccess(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo, DISW::DRM::Adaptor::FileOperation OperationHintType) override;

            int GetErrorMessage(int errorCode, const char* locale, std::wstring& message) override;

            bool IsEncrypted(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo) override;

            int ApplyFileAccessControl
            (
                std::shared_ptr<const DISW::DRM::Host::IFileInformation> sourceFile,
                std::shared_ptr<const DISW::DRM::Host::IFileInformation> producedFile
            ) override;
        };
    }
}


#endif
