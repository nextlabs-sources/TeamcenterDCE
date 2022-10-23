/*==================================================================================================

                           Copyright 2021 Siemens
                            All rights reserved

====================================================================================================
File description:
    The test DRM adaptor registrations

====================================================================================================
Date         Name                    Description of Change
$HISTORY$
==================================================================================================*/
#include <unidefs.h>

#include <DRM_Adaptor_Utils.hxx>
#include <env.h>
#include <drmadaptortestprivate/DRMAdaptorTest_AdaptorRegistrant.hxx>


using namespace UGS;

static std::shared_ptr<const DISW::DRM::Host::ISessionInformation> sessionInfoInterface;

//----------------------------------------------------------------------------------------------------------------------------------
static DISW::DRM::Adaptor::IDRM* getIDRMInterface()
{
    static DRMAdaptorTest::IDRMImpl object;
    return &object;
}

//----------------------------------------------------------------------------------------------------------------------------------
extern "C" DISW::DRM::Adaptor::IDRM* UGS::DRMAdaptorTest::Initialize
(
    std::shared_ptr<const DISW::DRM::Host::ISessionInformation> interf
)
{
    sessionInfoInterface = interf;
    return getIDRMInterface();
}

//----------------------------------------------------------------------------------------------------------------------------------
extern std::shared_ptr<const DISW::DRM::Host::ISessionInformation> UGS::DRMAdaptorTest::GetISessionInformation()
{
    return sessionInfoInterface;
}
