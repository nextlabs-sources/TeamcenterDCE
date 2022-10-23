// CSldWorksRMXDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SwRMXDialog.h"
#include "SwUtils.h"

CSldWorksRMXDialog::CSldWorksRMXDialog() {

}

CSldWorksRMXDialog::~CSldWorksRMXDialog() {

}

int CSldWorksRMXDialog::ShowMessageDialog(CSwResult * swResultObj,int options)
{
	HWND hwnd = (HWND)CSwUtils::GetInstance()->GetMainFrameWindowHandle();
	int uType = MB_TOPMOST | MB_OK;
	if (options != -1) {
		uType = uType | options;
	}
	MsgType msgType = swResultObj->GetMessageType();
	switch (msgType) {
	case MsgType::MSG_INFO:
		uType = uType | MB_ICONINFORMATION;
		break;
	case MsgType::MSG_WARN:
		uType = uType | MB_ICONWARNING;
		break;
	case MsgType::MSG_ERROR:
		uType = uType | MB_ICONERROR;
		break;
	}
	wstring err_msg = swResultObj->GetErrorMsg();
	CString aboutTitle;
	aboutTitle.LoadString(IDS_DIALOG_TITLE);
	int msgboxID = MessageBox(hwnd, err_msg.c_str(), aboutTitle, uType);
	return msgboxID;
	
}

int CSldWorksRMXDialog::ShowAboutDialog()
{
	HWND hwnd = (HWND)CSwUtils::GetInstance()->GetMainFrameWindowHandle();
	CString aboutText, aboutTitle;
	aboutText.LoadString(IDS_ABOUT_TEXT);
	aboutTitle.LoadString(IDS_DIALOG_TITLE);
	std::wstring msg = RMXUtils::FormatString((const wchar_t*)aboutText, _W(VER_FILEVERSION_STR), _W(VER_BUILD_STR), _W(VER_COPYRIGHT_Y));
	int msgboxID = MessageBox(hwnd, msg.c_str(), aboutTitle, MB_TOPMOST | MB_OK | MB_ICONINFORMATION);
	return msgboxID;
}

