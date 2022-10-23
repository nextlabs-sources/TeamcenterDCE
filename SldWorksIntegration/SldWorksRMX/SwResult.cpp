#include "SwResult.h"

wstring CSwResult::GetErrorMsg() {
	return m_Msg;
}


wstring CSwAuthResult::GetErrorMsg() {
	wstring delim = L"";
	wstring err_msg = L"";
	err_msg = m_Msg;
	vector<wstring> denyFileNames = m_fileDenyAccessList;
	for (auto fileName : denyFileNames) {
		err_msg = err_msg + delim + fileName;
		delim = L"\r\n";
	}

	return err_msg;

}