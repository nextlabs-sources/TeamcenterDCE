#ifndef RPMCOMMONDIALOGWRAPPER_H
#define RPMCOMMONDIALOGWRAPPER_H
#pragma once
/*
prepare environment to use the common dialog UI
on dev machine:
	1, install the RPM SDK which contains nxcommondialog.tlb, 
	or 
	1, copy the generated nxcommondialog.tlb to %SKYDRM_SDK_ROOT%\include
on test machine:
	1, install the RMD/RPM which contains nxcommondialog.dll, 
	or 
	1, copy the nxcommondialog.dll and WinFormControlLibrary.dll from build artifacts to %SKYDRM_INSTALL_DIR%\bin
	2, run following command to register dll manually
	"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\RegAsm.exe" /register /codebase nxcommondialog.dll
	"C:\Windows\Microsoft.NET\Framework\v4.0.30319\RegAsm.exe" /register /codebase nxcommondialog.dll
*/
#include "RMXLogger.h"
#import "mscorlib.tlb"
#import <nxcommondialog.tlb>
using namespace nxcommondialog;

class RPMCommonDialogWrapper
{
public:
	RPMCommonDialogWrapper()
	{
		try
		{
			LOG_TRACE_FMT("CoInitialize()...");
			::CoInitialize(NULL);
			LOG_TRACE_FMT("ICommonDialogPtr()...");
			_pDialog = ICommonDialogPtr(__uuidof(CommonDialog));
			LOG_TRACE_FMT("Initialized.");
		}
		catch (...) {
			LOG_ERROR_FMT("COM Initialization failed!");
		}
	}
	~RPMCommonDialogWrapper() {
		LOG_TRACE_FMT("ReleaseCOM()...");
		if (_pDialog != nullptr) {
			_pDialog->Release();
		}
		LOG_TRACE_FMT("CoUninitialize()...");
		CoUninitialize();
		LOG_TRACE_FMT("uninitialized.");
	}
	/*Note:
		filePath must be valid
		file must be protected, otherwise no dialog will be shown
		*/
	bool ShowFileInfoDialog(const wchar_t* filePath, const wchar_t* dispName, const wchar_t *dlgTitle=L"NextLabs RMX") {
		if (_pDialog == nullptr || filePath == nullptr)
			return false;
		if (dispName == nullptr)
			dispName = L"";
		LOG_TRACE_FMT("_pDialog->ShowFileInfoDlg('%s','%s', '%s')...", filePath, dispName, dlgTitle);
		DialogResult result = _pDialog->ShowFileInfoDlg(_bstr_t(filePath), _bstr_t(dispName), _bstr_t(dlgTitle));
		LOG_DEBUG_FMT("DialogResult='%d'", result);
		return result != DialogResult_Error;
	}
	bool ShowProtectDialog(const wchar_t *filePath, const wchar_t* actionName, std::wstring &oJsonTags, const wchar_t* dlgTitle = L"NextLabs RMX")
	{
		//when Tag Cateory is CompanyDefined, watermarkText and rights are not needed and are always empty
		std::wstring watermarkText;
		__int64 rights;
		return ShowProtectDialog(filePath, actionName, dlgTitle, NxlEnabledProtectMethod::NxlEnabledProtectMethod_Enabled_CompanyDefined, oJsonTags, rights, watermarkText);
	}
	bool ShowProtectDialog(const wchar_t* filePath, const wchar_t* actionName, const wchar_t* dlgTitle, NxlEnabledProtectMethod tagCategory,
		std::wstring &oJsonTags, __int64& oRights, std::wstring &oWatermarkText)
	{
		if (_pDialog == nullptr)
			return false;		
		const wchar_t* iconFile = L"";
		_bstr_t bstrTags(L"");
		__int64 rights = 0;
		_bstr_t bstrWmtext(L"");
		struct NxlExpiration nxlexp { NxlExpiryType::NxlExpiryType_NEVER_EXPIRE, 0, 0 };
		LOG_DEBUG_FMT("filePath='%s'", filePath);
		if (actionName == nullptr)
			actionName = L"";
		LOG_DEBUG_FMT("actionName='%s'", actionName);
		//if (iconFile == nullptr || wcslen(iconFile) == 0)
		//	iconFile = L"c:\\icon.png";
		LOG_DEBUG_FMT("iconFile='%s'", iconFile);
		LOG_DEBUG_FMT("title='%s'", dlgTitle);
		LOG_DEBUG_FMT("tagCategory='%d'", tagCategory);
		LOG_TRACE_FMT("_pDialog->ShowProtectDlg()...");
		DialogResult result = _pDialog->ShowProtectDlg(_bstr_t(filePath),
			_bstr_t(iconFile),
			_bstr_t(dlgTitle),
			_bstr_t(actionName),
			tagCategory,
			bstrTags.GetAddress(),
			&rights,
			bstrWmtext.GetAddress(),
			&nxlexp);
		LOG_DEBUG_FMT("DialogResult='%d'", result);
		if (result == DialogResult_Positive)
		{
			oJsonTags = (LPCTSTR)bstrTags;
			LOG_DEBUG_FMT("tags='%s'", oJsonTags.c_str());
			oRights = rights;
			LOG_DEBUG_FMT("rights='%lld'", oRights);
			oWatermarkText = (LPCTSTR)bstrWmtext;
			LOG_DEBUG_FMT("watermark='%s'", oWatermarkText.c_str());
			return true;
		}
		return false;
	}
private:
	ICommonDialogPtr _pDialog;
};

#endif // RPMCOMMONDIALOGWRAPPER_H