/*==================================================================================================

                           Copyright 2021 Siemens
                            All rights reserved

====================================================================================================
File description:                                                                                                              
    Test DRM Adaptor implementation of DISW::DRM::Adaptor::IDRM
    
====================================================================================================
Date         Name                    Description of Change
$HISTORY$
==================================================================================================*/
#ifndef DRMAdaptorTest_IDRMImpl_HXX_INCLUDED
#define DRMAdaptorTest_IDRMImpl_HXX_INCLUDED

#include <DISW_DRM.hxx>

namespace UGS
{
    namespace DRMAdaptorTest
    {
        class IDRMImpl : public DISW::DRM::Adaptor::IDRM
        {
        public:

            ~IDRMImpl() {}

            IDRMImpl() {}

            DISW::DRM::Adaptor::IAccessControl* GetAccessControlInterface() override;
        };
    }
}


#endif
