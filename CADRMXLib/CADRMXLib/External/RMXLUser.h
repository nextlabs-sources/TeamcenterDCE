//
// This is a part of the NextLabs CADRMX Client SDK.
// Copyright (C) 2020 NextLabs, Inc. All Rights Reserved.
//
#pragma once

#include "RMXLResult.h"
#include "RMXLTypeDef.h"

/*!
* \file RMXLUser.h
* \class IRMXUser
*
* This interface provides information about a RMX user.
*
*/
class CADRMX_API IRMXUser
{
public:
	IRMXUser() {};
	virtual ~IRMXUser() {};

public:
	/*!
	 * Gets current login user name
	 *
	 * \return User name string.
	 */
	virtual const std::wstring GetName() = 0;
	
	/*!
	 * Gets current login user email address
	 * 
	 * \return User email string.
	 */
	virtual const std::wstring GetEmail() = 0;
	
	/*!
	 * Get current login user id
	 * 
	 * \return User id.
	 */
	virtual uint32_t GetUserID() = 0;
	
	/*!
	 * Protects a local file
	 * 
	 * \param [in] filePath File path to be protected.
	 * \param [in] targetDir Target directory path to place new protected file.
	 * \param [in] tags Document classification metadata.
	 * \param [out] newNxlFile File path of NXL file to be created.
	 * 
	 * \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	 */
	virtual RMXResult ProtectFile(const std::wstring& filePath, const std::wstring& targetDir, const std::string& tags, std::wstring& newNxlFile) = 0;

	/*!
	 * Gets the resource rights with their associated obligations from central policies
	 * 
	 * \param [in] resourceName Name of resource
	 * \param [in] resourceType Type of resource (e.g. "fso", "portal")
	 * \param [in] attrs Resource attributes to be evaluated
	 * \param [out] centralRights Rights assigned to the resource and their associated obligations to be returned.
	 * 
	 * \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	 */
	virtual RMXResult GetResourceRightsFromCentralPolicies(const std::wstring& resourceName, const std::wstring& resourceType, const std::vector<RMX_EVAL_ATTRIBUATE> &attrs, std::vector<RMX_CENTRAL_RIGHT>& centralRights) = 0;

	/*!
	 * Gets policy boudle for default tenant
	 * 
	 * \param [out] policyBundle Policy bundle to be returned.
	 * 
	 * \return True if success, else return false.
	 */
	virtual bool GetDefaultPolicyBundle(std::string& policyBundle) = 0;

	/*!
	 * Gets system project tenantId
	 * 
	 * \return System project tenantId
	 */
	virtual const std::string GetSystemProjectTenantId() = 0;
	
	/*!
	 * Gets token group name of default tenant
	 * 
	 * \return Token group name of default tenant
	 */
	virtual const std::string GetDefaultTokenGroupName() = 0;

	/*!
	 * Merges tags in case tags come from multiple source NXL files, according to the priority obligation defined in central policy
	 * 
	 * \param [in] unionTags Multiple tags strings grouped by "{}", such as {"ip_classification":["secret"]}{"ip_classification":["supper-securet"]}.
	 * \param [out] mergedTags Single group of tags to be returned, such as {"ip_classification":["supper-securet"]}.
	 * \note The algorithm to merge tags groups are:
	 *	- Duplicated value should be removed for same key.  
	 *	- Different values are combined as multiples values for same key.  
	 *	- Lower priorit values will be removed, and leave higher one as single value for same key, if priority obligation is found in central policy.  
	 * 
	 * \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	 */
	virtual RMXResult MergeTags(const std::string& unionTags, std::string& mergedTags) = 0;

	/*!
	* Adds file activity log
	*
	* \param [in] nxlFilePath File path of the NXL file.
	* \param [in] op Activity operation type.
	* \param [in] result Operation result such as denied or allowed
	*
	* \returns \ref RMXResult object. Either call RMXResult::GetCode() to check error code, or call bool() to check if error code is 0.
	*/
	virtual RMXResult AddActivityLog(const std::wstring & nxlFilePath, RMXActivityLogOperation op, RMXActivityLogResult result) = 0;
};