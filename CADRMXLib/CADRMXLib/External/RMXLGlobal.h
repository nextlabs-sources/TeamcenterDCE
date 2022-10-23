//
// This is a part of the NextLabs CADRMX Client SDK.
// Copyright (C) 2020 NextLabs, Inc. All Rights Reserved.
//


/*!
 * \file RMXLGlobal.h
 * 		 
 * This head file contains all gloal functions
 * 
 */
#pragma once

#include <wtypes.h>

#include "RMXLResult.h"
#include "RMXLTypeDef.h"

class IRMXInstance;
class IRMXUser;

/**
* \defgroup globalFunc Global Functions
*/

/**
* Returns the version number of the CADRMX Client SDK
* 
* \ingroup globalFunc
* \returns The format of version is AABBCCCC major.minor.build.
*/
CADRMX_API DWORD RMX_GetLibVersion(void);

/**
* Returns the version number of the SkyDRM Client SDK
*
* \ingroup globalFunc
* \returns The format of version is AABBCCCC major.minor.build.
*/
CADRMX_API DWORD RMX_GetSDWLibVersion(void);

/**
* Gets current login user
*
* \ingroup globalFunc
* \param [out] pInstance IRMXInstance pointer.
* \param [out] pUser	IRMXUser pointer
*
* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
*
* \note Call \ref RMX_DeleteRMXInstance to delete instance.
*/
CADRMX_API RMXResult RMX_GetCurrentLoggedInUser(IRMXInstance*& pInstance, IRMXUser*& pUser);

/**
* Deletes an IRMXInstance instance
*
* \ingroup globalFunc
* \pre
* 	  \ref RMX_GetCurrentLoggedInUser is called to get the IRMXInstance instance.
*
* \param [in] pInstance IRMXInstance pointer.
*
* \returns void
*
*/
CADRMX_API void RMX_DeleteRMXInstance(IRMXInstance* pInstance);