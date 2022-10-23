#pragma once

#include "External\RMXLUser.h"

class ISDRmUser;

namespace RMXLib
{
	class CRMXLUserImpl :
		public IRMXUser
	{
	public:
		CRMXLUserImpl();
		explicit CRMXLUserImpl(ISDRmUser* pDRmUser);
		virtual ~CRMXLUserImpl();

	public:
		// Inherited via IRMXUser
		virtual const std::wstring GetName() override;
		virtual const std::wstring GetEmail() override;
		virtual uint32_t GetUserID() override;
		virtual RMXResult ProtectFile(const std::wstring & filePath, const std::wstring & targetDir, const std::string & tags, std::wstring & newNxlFile) override;
		virtual RMXResult GetResourceRightsFromCentralPolicies(const std::wstring & resourceName, const std::wstring & resourceType, const std::vector<RMX_EVAL_ATTRIBUATE>& attrs, std::vector<RMX_CENTRAL_RIGHT>& centralRights) override;
		virtual bool GetDefaultPolicyBundle(std::string & policyBundle) override;
		virtual const std::string GetSystemProjectTenantId() override;
		virtual const std::string GetDefaultTokenGroupName() override;
		virtual RMXResult MergeTags(const std::string& unionTags, std::string& mergedTags) override;
		virtual RMXResult AddActivityLog(const std::wstring & nxlFilePath, RMXActivityLogOperation op, RMXActivityLogResult result);
	private:
		ISDRmUser* m_pDRmUser;
		std::string m_sysProjTenantId;
		std::string m_sysProjMemberID;
	};

}; //namespace RMXLib
