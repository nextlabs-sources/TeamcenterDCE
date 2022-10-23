#pragma once

#include <string>

#include "External\RMXLInstance.h"

class ISDRmcInstance;
class ISDRmUser;
class IRMXUser;

namespace RMXLib
{
	class CRMXLInstanceImpl :
		public IRMXInstance
	{
	public:
		CRMXLInstanceImpl();
		CRMXLInstanceImpl(ISDRmcInstance* pSDRmcInst, ISDRmUser* pSDRmUser);
		virtual ~CRMXLInstanceImpl();

	public:
		ISDRmcInstance* GetSDL() const { return m_pSDRmcInst; }
		RMXResult GetLoginUser(IRMXUser *& pUser);

	public:
		// Inherited via IRMXInstance
		virtual RMXResult IsInitFinished(bool & finished) override;
		
		virtual RMXResult AddRPMDir(const std::wstring & dirPath, uint32_t option = (RMXRPMFolderOption::RMX_RPMFOLDER_OVERWRITE | RMXRPMFolderOption::RMX_RPMFOLDER_API)) override;
		virtual RMXResult RemoveRPMDir(const std::wstring & dirPath, bool force = true) override;
		virtual RMXResult IsRPMFolder(const std::wstring & dirPath, bool & isRPMDir, RMX_RPMFolderRelation * relation = nullptr) override;
		virtual RMXResult RegisterApp(const std::wstring & appPath) override;
		virtual RMXResult UnregisterApp(const std::wstring & appPath) override;
		virtual RMXResult IsAppRegistered(const std::wstring & appPath, bool & registered) override;
		virtual RMXResult NotifyRMXStatus(bool running) override;
		virtual RMXResult AddTrustedApp(const std::wstring &appPath);
		virtual RMXResult RemoveTrustedApp(const std::wstring &appPath);
		virtual RMXResult EditCopyFile(const std::wstring & nxlFilePath, std::wstring & destPath) override;
		virtual RMXResult EditSaveFile(const std::wstring & filePath, const std::wstring & originalNXLfilePath = L"", bool deleteSource = false, uint32_t exitEdit = 0) override;
		virtual RMXResult RPMCopyFile(const std::wstring& existingNxlFilePath, const std::wstring& newNxlFilePath, bool deleteSource = false) override;
		virtual RMXResult RPMDeleteFile(const std::wstring & filePath) override;
		virtual RMXResult GetFileRights(const std::wstring & filePath, std::vector<RMXFileRight>& rights) override;
		virtual RMXResult CheckFileRight(const std::wstring& filePath, RMXFileRight right, bool& allowed, bool audit = false) override;
		virtual RMXResult GetFileStatus(const std::wstring & filePath, bool & isProtected, RMX_RPMFolderRelation * dirRelation = nullptr) override;
		virtual RMXResult ReadFileTags(const std::wstring & filePath, std::string & tags) override;
		virtual RMXResult RPMGetFileInfo(const std::wstring & filepath, std::string &duid, std::string &tags,
			std::string &tokengroup, std::string &creatorid, std::string &infoext, DWORD &attributes, RMX_RPMFolderRelation & dirRelation, BOOL &isNXLFile) override;
		virtual RMXResult RPMSetFileAttributes(const std::wstring & nxlFilePath, DWORD dwFileAttributes) override;
		virtual RMXResult SetViewOveraly(const RMX_VIEWOVERLAY_PARAMS & params) override;
		virtual RMXResult ClearViewOverlay(void * hTargetWnd) override;

	private:
		bool IsInit() const { return (m_pSDRmcInst != nullptr); };
		void NotifyRPM(const std::wstring& path, bool isFile);
		bool IsRPMDir(uint32_t sdrDirStatus);	
		RMX_RPMFolderRelation GetRPMDirRelation(uint32_t sdrDirStatus);
		std::wstring GetRightName(RMXFileRight right) const;

	private:
		ISDRmcInstance* m_pSDRmcInst;
		IRMXUser* m_pRMXUser;
	};
}

