/*==================================================================================================

                           Copyright 2021 Siemens
                            All rights reserved

====================================================================================================
File description:
    Error codes for the DRM Adaptor test simulator.

====================================================================================================
Date         Name                    Description of Change
$HISTORY$
==================================================================================================*/
#include <unidefs.h>

#include <System_L10N_Localize.hxx>
#include <unordered_map.hxx>

#include <DRMAdaptorTest_Errors.hxx>

using namespace UGS;


extern const char* DRMAdaptorTest::DecodeError(int error_code, const char* locale)
{
    static const auto table = UGS::unordered_map<int, const char*>
    {
        { DRM_ADAPTOR_TEST_Error_not_authorized_to_save_part,
            LOCALIZE("DRM_ADAPTOR_TEST_Error_not_authorized_to_save_part", 
                        "User not authorized to save the part.",
                        "DRM ADAPTOR")},
        { DRM_ADAPTOR_TEST_Error_not_authorized_to_open_part,
                    LOCALIZE("DRM_ADAPTOR_TEST_Error_not_authorized_to_open_part", 
                                "User not authorized to open the part.",
                                "DRM ADAPTOR")},
        { DRM_ADAPTOR_TEST_Error_not_authorized_to_export_part,
                    LOCALIZE("DRM_ADAPTOR_TEST_Error_not_authorized_to_export_part", 
                                "User not authorized to export the part.",
                                "DRM ADAPTOR")},
        { DRM_ADAPTOR_TEST_Error_failed_to_apply_access_rights,
                    LOCALIZE("DRM_ADAPTOR_TEST_Error_failed_to_apply_access_rights",
                                "Failed to apply DRM access rights on the file.",
                                "DRM ADAPTOR")}

    };

    auto found = table.find(error_code);
    if (found != table.end())
    {
        return found->second;
    }
    return nullptr;
}

