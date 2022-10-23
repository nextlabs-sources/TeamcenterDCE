#include "RMXLUserImpl.h"

#include <CADRMXTypeDef.h>
#include "InternalTypeDef.h"
#include "RMXLLogger.h"
#include "RMXLResultEx.h"
#include <RMXTagHlp.h>
#include <RMXUtils.h>

#include <SDLUser.h>

namespace RMXLib
{
	CRMXLUserImpl::CRMXLUserImpl()
		: m_pDRmUser(nullptr)
	{
	}

	CRMXLUserImpl::CRMXLUserImpl(ISDRmUser* pDRmUser)
		: m_pDRmUser(pDRmUser)
	{
		if (m_pDRmUser != nullptr)
		{
			m_sysProjTenantId = m_pDRmUser->GetSystemProjectTenantId();;
			m_sysProjMemberID = m_pDRmUser->GetMembershipID(m_sysProjTenantId);
		}
	}

	CRMXLUserImpl::~CRMXLUserImpl()
	{
	}

	const std::wstring CRMXLUserImpl::GetName()
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
			return std::wstring();

		const std::wstring& name = m_pDRmUser->GetName();
		LOG_INFO(L"GetName succeeded: " << name.c_str());

		return name;
	}

	const std::wstring CRMXLUserImpl::GetEmail()
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
			return std::wstring();

		const std::wstring& email = m_pDRmUser->GetEmail();
		LOG_INFO(L"GetEmail succeeded: " << email.c_str());

		return email;
	}

	uint32_t CRMXLUserImpl::GetUserID()
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
			return -1;

		uint32_t id = m_pDRmUser->GetUserID();
		LOG_INFO_FMT(L"GetUserID succeeded: %d", id);

		return id;
	}

	/*const std::string CRMXLUserImpl::GetMembershipID(uint32_t projectId)
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
			return std::string();

		const std::string& id = m_pDRmUser->GetMembershipID(projectId);
		LOG_DEBUG_FMT(L"IRMXLUser::GetMembershipID succeeded: %d", id);

		return id;
	}

	const std::string CRMXLUserImpl::GetMembershipID(const std::string & tenantId)
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
			return std::string();

		const std::string& id = m_pDRmUser->GetMembershipID(tenantId);
		LOG_DEBUG_FMT(L"IRMXLUser::GetMembershipID succeeded: %d", id);

		return id;
	}*/

	RMXResult CRMXLUserImpl::ProtectFile(const std::wstring & filePath, const std::wstring & targetDir, const std::string & tags, std::wstring & newNxlFile)
	{
		RMXL_LOG_ENTER;
		RMXL_ERROR_RETURN(m_pDRmUser == nullptr, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid m_pDRmUser"));
		RMXL_ERROR_RETURN(filePath.empty(), RMX_RESULT2(ERROR_INVALID_DATA, L"filePath not specified"));

		std::wstring newCreatedFile(targetDir);
		std::vector<SDRmFileRight> rights;
		SDR_Expiration expire;
		SDR_WATERMARK_INFO watermarkinfo;
		expire.type = IExpiryType::NEVEREXPIRE;
		CRMXLResultEx result;
		result = m_pDRmUser->ProtectFile(filePath, newCreatedFile, rights, watermarkinfo, expire, tags, m_sysProjMemberID);
		RMXL_ERROR_RETURN_FMT(!result, result,  L"[SDL]ProtectFile failed: '%s'(targetDir=%s, tags=%s)", filePath.c_str(), targetDir.c_str(), RMXUtils::s2ws(tags).c_str());

		newNxlFile = newCreatedFile;
		LOG_INFO_FMT(L"ProtectFile succeeded(sourceFile=%s, targetDir=%s, tags=%s): %s", filePath.c_str(), targetDir.c_str(), RMXUtils::s2ws(tags).c_str(), newNxlFile.c_str());

		return result;
	}

	RMXResult CRMXLUserImpl::GetResourceRightsFromCentralPolicies(
		const std::wstring & resourceName,
		const std::wstring & resourceType,
		const std::vector<RMX_EVAL_ATTRIBUATE>& attrs,
		std::vector<RMX_CENTRAL_RIGHT>& centralRights)
	{
		RMXL_LOG_ENTER;
		RMXL_ERROR_RETURN(m_pDRmUser == nullptr, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid m_pDRmUser"));

		std::vector<std::pair<SDRmFileRight, std::vector<SDR_OBLIGATION_INFO>>> vecRO;
		TEvalAttributeList attrList;
		for (const auto& attr : attrs)
		{
			attrList.push_back(TEvalAttribute(std::wstring(attr.key), std::wstring(attr.value)));
		}
		CRMXLResultEx result;
		result = m_pDRmUser->GetResourceRightsFromCentralPolicies(resourceName, resourceType, attrList, vecRO);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]GetResourceRightsFromCentralPolicies failed: resourceName: %s, resourceType: %s", resourceName.c_str(), resourceType.c_str());

		const auto size = vecRO.size();
		for (const auto& ro : vecRO)
		{
			RMX_CENTRAL_RIGHT cRight;
			cRight.right = (RMXFileRight)ro.first;
			// Pack obligations
			for (const auto& ob : ro.second)
			{
				RMX_OBLIGATION_INFO obInfo;
				obInfo.name = ob.name;
				for (const auto& option : ob.options)
				{
					RMX_EVAL_ATTRIBUATE evalAttr;
					evalAttr.key = option.first;
					evalAttr.value = option.second;
					obInfo.options.push_back(evalAttr);
				}
				cRight.obligations.push_back(obInfo);
			}
			centralRights.push_back(cRight);
		}

		LOG_INFO_FMT(L"GetResourceRightsFromCentralPolicies succeeded: resourceName: %s, resourceType: %s", resourceName.c_str(), resourceType.c_str());

		return result;
	}

	bool CRMXLUserImpl::GetDefaultPolicyBundle(std::string & policyBundle)
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
			return false;

		const std::string& defaultTenant = m_pDRmUser->GetDefaultTokenGroupName();
		if (!defaultTenant.empty())
		{
			if (m_pDRmUser->GetPolicyBundle(RMXUtils::s2ws(defaultTenant), policyBundle))
			{
				LOG_INFO_FMT(L"GetDefaultPolicyBundle succeeded: %s", RMXUtils::s2ws(policyBundle).c_str());
				return true;
			}
		}
		
		return false;
	}

	const std::string CRMXLUserImpl::GetSystemProjectTenantId()
	{
		return m_sysProjMemberID;
	}

	const std::string CRMXLUserImpl::GetDefaultTokenGroupName()
	{
		RMXL_LOG_ENTER;
		if (m_pDRmUser == nullptr)
		{
			LOG_ERROR(L"Invalid m_pDRmUser");
			return false;
		}
	
		return m_pDRmUser->GetDefaultTokenGroupName();
	}

	RMXResult CRMXLUserImpl::MergeTags(const std::string & unionTags, std::string & mergedTags)
	{
		RMXL_LOG_ENTER;
		RMXL_ERROR_RETURN(m_pDRmUser == nullptr, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid m_pDRmUser"));
		
		bool succeed = RMXLib::MergeTags(m_pDRmUser, unionTags, mergedTags);
		RMXL_ERROR_RETURN(!succeed, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"IRMXLUser::MergeTags failed"));

		return RMXResult();
	}

	RMXResult CRMXLUserImpl::AddActivityLog(const std::wstring & nxlFilePath, RMXActivityLogOperation op, RMXActivityLogResult result)
	{
		RMXL_LOG_ENTER;
		RMXL_ERROR_RETURN(m_pDRmUser == nullptr, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid m_pDRmUser"));

		CRMXLResultEx ret;
		ret = m_pDRmUser->AddActivityLog(nxlFilePath, (RM_ActivityLogOperation)op, (RM_ActivityLogResult)result);
		RMXL_ERROR_RETURN_FMT(!ret, ret, L"[SDL]AddActivityLog failed: '%s '", nxlFilePath.c_str());

		LOG_INFO_FMT(L"AddActivityLog succeeded(%s): op=%d, result=%d", nxlFilePath.c_str(), op, result);

		return ret;
	}

} // namespace RMXLib