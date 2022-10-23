#include "stdafx.h"
#include "NXDRMIntegration.h"
#include "NXDRMErrors.h"
#include <unordered_map>

static std::unordered_map<int, std::wstring> errorMessages({
	//RPM API Errors
	{ DRM_ERROR_RPM_ERROR_BASE ,L"RPM API Error" },
	{ DRM_ERROR_RPM_USER_NOT_LOGGED_IN ,L"User not logged in RMD" },
	{ DRM_ERROR_RPM_INVALID_FILE ,L"Invalid file, you may not have view right" },

	//FILE errors
	{ DRM_ERROR_FILE_IS_NOT_IN_RPM, L"NextLabs protected file is not in safe directory" },
	{ DRM_ERROR_FILE_IS_NOT_PROTECTED ,L"File is not NextLabs protected" },
	{ DRM_ERROR_FILE_IS_PROTECTED ,L"File is NextLabs protected" },
	{ DRM_ERROR_FILE_NOT_EXIST,L"File doesn't exist" },
	{ DRM_ERROR_FILE_HAS_NO_NXL_EXT,L"Protected File has no .nxl extension. this error has been automatically fixed, please retry" },
	//rights errors
	{ DRM_ERROR_ACCESS_DENIED , L"Access denied" },
	//export errors
	{ DRM_ERROR_EXPORT_FOLDER_NOT_RPM, L"file is exported into a non-NextLabs RPM folder, .nxl is appended"}
});

int NXL::NX::DRM::AccessControlImpl::GetErrorMessage(int errorCode, const char * locale, std::wstring & message)
{
	auto kvp = errorMessages.find(errorCode);
	if (kvp != errorMessages.end()) {
		message = kvp->second;
	}
	else
	{
		wchar_t buff[100];
		wsprintf(buff, L"unknown error-%d", errorCode);
		message = buff;
	}
	message += std::wstring(L"[NextLabsDRM]");
	return 0;
}
int NXL::NX::DRM::AccessControlImpl::GetErrorCodeForOperation(DISW::DRM::Adaptor::FileOperation operationType)
{
	int errorCode = DRM_ERROR_ACCESS_DENIED + 1 + (int)operationType;//operation type is zero-based enum
	if (errorMessages.find(errorCode) == errorMessages.end()) {
		std::wstringstream wss;
		wss << L"You don't have permission to";
		switch (operationType) {
		case DISW::DRM::Adaptor::FileOperation::SAVE:// Is this file allowed to be written to and saved to under the same name
			wss << L" Save";
			break;
		case DISW::DRM::Adaptor::FileOperation::SAVEAS: // Is this file allowed to be SavedAs.
			wss << L" SaveAs";
			break;
		case DISW::DRM::Adaptor::FileOperation::EXPORT: // Is this file allowed to be exported.
			wss << L" Export";
			break;
		case DISW::DRM::Adaptor::FileOperation::COPY:   // Is this file allowed to be copied from.
			wss << L" Copy";
			break;
		case DISW::DRM::Adaptor::FileOperation::READ:   // Is this file allowed to be read from.
			wss << L" Read";
			break;
		case DISW::DRM::Adaptor::FileOperation::WRITE:   // Is this file allowed to be written to and saved to under the same name.
			wss << L" Write";
			break;
		default:
			wss << L" access";
			break;
		}
		wss << " this NextLabs protected file.";
		//initialize error message for access deny
		errorMessages.insert(std::make_pair(errorCode, wss.str()));
	}

	return errorCode;
}

static std::unordered_map<DWORD, int> rpmErrors({ {1245, DRM_ERROR_RPM_USER_NOT_LOGGED_IN }, {13, DRM_ERROR_RPM_INVALID_FILE } });
int RPMErrorToDRMError(DWORD e)
{
	auto kvp = rpmErrors.find(e);
	if (kvp != rpmErrors.end()) {
		return kvp->second;
	}
	return DRM_ERROR_RPM_ERROR_BASE;
}
