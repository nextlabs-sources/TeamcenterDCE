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
#include <unidefs.h>

#include <drmadaptortestprivate/DRMAdaptorTest_IAccessControlImpl.hxx>
#include <drmadaptortestprivate/DRMAdaptorTest_IDRMImpl.hxx>


using namespace UGS;

//----------------------------------------------------------------------------------------------------------------------------------
DISW::DRM::Adaptor::IAccessControl* DRMAdaptorTest::IDRMImpl::GetAccessControlInterface()
{
    static DRMAdaptorTest::IAccessControlImpl object;
    return &object;
}
