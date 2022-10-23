#pragma once

#include <string>
#include <vector>

// Forward declarations
class ISDRmcInstance;
class ISDRmUser;

class CSDRmSession
{
public:
	CSDRmSession();
	virtual ~CSDRmSession();

	bool IsLoggedIn() const { return m_bInit; }

	bool RPMRegisterApp(const std::wstring& appPath);

	bool RPMUnregisterApp(const std::wstring& appPath);

	bool RPMIsAppRegistered(const std::wstring& appPath, bool& isRegistered) const;

	bool IsRPMDir(const std::wstring& dirPath, bool& isRPMDir) const;

	bool AddRPMDir(const std::wstring& dirPath);

	bool RemoveRPMDir(const std::wstring& dirPath);

	bool GetAllRPMDirs(const std::wstring & rootDir, std::vector<std::wstring>& retDirs) const;

	bool GetPolicyBundle(std::string& retPolicies) const;

	static std::wstring GetSDKVersion();

private:
	ISDRmcInstance* m_pDRmcInstance;
	ISDRmUser* m_pDRmUser;
	bool m_bInit;
};

