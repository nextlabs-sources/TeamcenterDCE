#pragma once
#include <DISW_DRM.hxx>
#include <RPMUtils/RPMUtils.h>
#include "NXDRMErrors.h"
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NXDRMINTEGRATION_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NXDRMINTEGRATION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NXDRMINTEGRATION_EXPORTS
#define NXDRMINTEGRATION_API __declspec(dllexport)
#else
#define NXDRMINTEGRATION_API __declspec(dllimport)
#endif

namespace NXL
{
	namespace NX
	{
		namespace DRM
		{
			extern "C" NXDRMINTEGRATION_API DISW::DRM::Adaptor::IDRM* Initialize(std::shared_ptr<const DISW::DRM::Host::ISessionInformation> interf);

			extern std::shared_ptr<const DISW::DRM::Host::ISessionInformation> GetISessionInformation();

			extern RPMSession *GetRPMSession();
			
			class AdaptorImpl : public DISW::DRM::Adaptor::IDRM
			{
			public:

				//destructor
				~AdaptorImpl() override {}

				//constructor
				AdaptorImpl() {}

				DISW::DRM::Adaptor::IAccessControl* GetAccessControlInterface() override;
			};

			class AccessControlImpl :public DISW::DRM::Adaptor::IAccessControl
			{
			public:
				~AccessControlImpl() override {}

				AccessControlImpl() {}
				void PrepareFileForLoad(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo) override;

				int CheckFileAccess(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo, DISW::DRM::Adaptor::FileOperation OperationHintType) override;

				int GetFilePermissions
				(
					std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo,
					std::shared_ptr<DISW::DRM::Host::IFilePermissions> filePermissions
				) override;
				int GetErrorCodeForOperation
				(
					DISW::DRM::Adaptor::FileOperation operationType
				) override;
				int GetErrorMessage(int errorCode, const char* locale, std::wstring& message) override;

				bool IsEncrypted(std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo) override;

				int ApplyFileAccessControl
				(
					std::shared_ptr<const DISW::DRM::Host::IFileInformation> sourceFile,
					std::shared_ptr<const DISW::DRM::Host::IFileInformation> producedFile
				) override;
			};
		}
	}
}