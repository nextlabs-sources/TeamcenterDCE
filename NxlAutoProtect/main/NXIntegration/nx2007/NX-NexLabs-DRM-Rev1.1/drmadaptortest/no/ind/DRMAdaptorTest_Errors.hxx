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
#ifndef DRMAdaptorTest_Errors_HXX_INCLUDED
#define DRMAdaptorTest_Errors_HXX_INCLUDED

namespace UGS
{
    namespace DRMAdaptorTest
    {
        // DRM test adaptor implementaton errors
        //
        constexpr int DRM_ADAPTOR_TEST_Error_base = 0;
        constexpr int DRM_ADAPTOR_TEST_Error_max = 4999;
        constexpr int DRM_ADAPTOR_TEST_Error_ok = DRM_ADAPTOR_TEST_Error_base;
        constexpr int DRM_ADAPTOR_TEST_Error_not_authorized_to_open_part = DRM_ADAPTOR_TEST_Error_base + 1;
        constexpr int DRM_ADAPTOR_TEST_Error_not_authorized_to_save_part = DRM_ADAPTOR_TEST_Error_base + 2;
        constexpr int DRM_ADAPTOR_TEST_Error_not_authorized_to_export_part = DRM_ADAPTOR_TEST_Error_base + 3;
        constexpr int DRM_ADAPTOR_TEST_Error_failed_to_apply_access_rights = DRM_ADAPTOR_TEST_Error_base + 4;

        extern const char* DecodeError(int error_code, const char* locale);
    }
}
#endif
