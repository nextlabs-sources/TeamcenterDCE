/*==================================================================================================

                           Copyright 2021 Siemens
                            All rights reserved

====================================================================================================
File description:                                                                                                              
    Header for the test DRM adaptor registrations
    
====================================================================================================
Date         Name                    Description of Change
$HISTORY$
==================================================================================================*/
#ifndef DRMAdaptorTest_AdaptorRegistrant_HXX_INCLUDED
#define DRMAdaptorTest_AdaptorRegistrant_HXX_INCLUDED

#include <drmadaptortestprivate/DRMAdaptorTest_IDRMImpl.hxx>

#include <libdrmadaptortest_exports.h>

namespace UGS
{
    namespace DRMAdaptorTest
    {
        extern "C" DRMADAPTORTESTEXPORT DISW::DRM::Adaptor::IDRM* Initialize
        (
            std::shared_ptr<const DISW::DRM::Host::ISessionInformation> interf
        );

        extern std::shared_ptr<const DISW::DRM::Host::ISessionInformation> GetISessionInformation();
    }
}


#endif
