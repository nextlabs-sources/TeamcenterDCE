#pragma once

#include <pfcGlobal.h>
#include <ProToolkitDll.h>


#define IPEMRMX_API PRO_TK_DLL_EXPORT

// API called by JavaIpemRMX
extern "C" IPEMRMX_API ProError RMX_ProCheckRight(ProArgument* inputs, ProArgument** output);
extern "C" IPEMRMX_API ProError RMX_ProFileCheckSaveRight(ProArgument* inputs, ProArgument** output);
extern "C" IPEMRMX_API ProError RMX_ProFileCheckSaveAsRight(ProArgument* inputs, ProArgument** output);
extern "C" IPEMRMX_API ProError RMX_ProFileSetReadOnly(ProArgument* inputs, ProArgument** output);
extern "C" IPEMRMX_API ProError RMX_ProIsEAIBatchMode(ProArgument * inputs, ProArgument * *output);
extern "C" IPEMRMX_API ProError RMX_ProIsRPMFolder(ProArgument * inputs, ProArgument * *output);
extern "C" IPEMRMX_API ProError RMX_ProFixNxlExtInWorkspace(ProArgument * inputs, ProArgument * *output);
extern "C" IPEMRMX_API ProError RMX_ProFixNxlExt(ProArgument * inputs, ProArgument * *output);
extern "C" IPEMRMX_API ProError RMX_ProSetCacheFolder(ProArgument * inputs, ProArgument * *output);

namespace IpemRMX
{
	struct RegistryFile
	{
		wstring path;
		__time64_t modifiedTime;

		void Clear() {
			path.clear();
			modifiedTime = 0;
		}
	};

	std::wstring GetRMXDllIDFilePath();
	void SaveRMXDllIdToFile(const std::wstring& appId);
	void DeleteRMXDllIdFile();

	void UpdateRegistryFile();
	void EnsureNxlExtInWorkspace(bool force=false);
	std::wstring GetCurrentRegistryFilePath();
	void RefreshCurrentRegistryFile();

	bool Register();
	void Unregister();

	void SetEAIBatchModel();
	bool IsEAIBatchModel();

	std::wstring GetTempFolder();

	void CleanupTrans();

	bool JTDispEnabled();

	bool JTDispRunning();

} // namespace IpemRMX
