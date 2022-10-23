#include "RACRMX.h"
#include "RMXInstance.h"
#include "view_watermark.h"
#include "RMXUtils.h"
#include <SDLInc.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <inttypes.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <sstream>
#include <unordered_set>
#include <filesystem>
#include "json.hpp"

ISDRmUser* pDRmUser = nullptr;
ISDRmcInstance* pDRmInstance = nullptr;
HWND handle;

static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_USER = L"$(user)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_HOST = L"$(host)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_DATE = L"$(date)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_TIME = L"$(time)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_BREAK = L"$(break)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_RIGHT = L"$(right)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_IP = L"$(ip)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_CLASSIFY = L"$(classification)";

static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_FILENAME = L"$(filename)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_FILEPATH = L"$(filepath)";

std::wstring dirPathOffice1;
std::wstring dirPathOffice2;
//std::wstring dirPathVisView;
std::wstring dirPathFCC;
std::wstring dirPathTemp;

static std::map<SDRmFileRight, std::wstring> RIGHTSMAP = {
	{ RIGHT_VIEW, L"View" },
	{ RIGHT_EDIT, L"Edit" },
	{ RIGHT_PRINT, L"Print" },
	{ RIGHT_CLIPBOARD, L"Clipboard" },
	{ RIGHT_SAVEAS, L"Save As" },
	{ RIGHT_DECRYPT, L"Extract" },
	{ RIGHT_SCREENCAPTURE, L"Screen Capture" },
	{ RIGHT_SEND, L"Send" },
	{ RIGHT_CLASSIFY, L"Classify" },
	{ RIGHT_SHARE, L"Share" },
	{ RIGHT_DOWNLOAD, L"Save As" },
	{ RIGHT_WATERMARK, L"Watermark" }
};

static std::unordered_set<std::wstring> SUPPORTED_FILE_EXT{
	L".doc", L".docx",
	L".pdf", L".jpg", L".jpeg"
};

static std::map<uint32_t, wchar_t*> s_rpmDirRelations = {
	{ RPM_SAFEDIRRELATION_SAFE_DIR, L"Safe dir" },
	{ RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR, L"Ancestor of safe dir" },
	{ RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR, L"Posterity of safe dir" },
	{ 0, L"Not safe dir" }
};

inline bool _IsRPMDir(DWORD sdrDirStatus)
{
	return (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR || sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
}

jbyteArray as_byte_array(JNIEnv *env, unsigned char* buf, int len) {
	jbyteArray array = env->NewByteArray(len);
	env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
	return array;
}

unsigned char* as_unsigned_char_array(JNIEnv *env, jbyteArray array) {
	int len = env->GetArrayLength(array);
	unsigned char* buf = new unsigned char[len];
	env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
	return buf;
}

bool isFileSupported(std::wstring filepath) {
	std::wstringstream debugStream;
	size_t i_FCC = filepath.find(L"\\FCCCache");
	debugStream << L"i_FCC: " << i_FCC << std::endl;
	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();
	if (i_FCC != std::wstring::npos)
		return true;

	size_t i_nxl = filepath.rfind('.', filepath.length());
	debugStream << L"i_nxl: " << i_nxl << std::endl;
	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();
	if (i_nxl == std::wstring::npos)
		return false;

	size_t i_ext = filepath.rfind('.', i_nxl - 1);
	debugStream << L"i_ext: " << i_ext << std::endl;
	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();
	if (i_ext == std::wstring::npos)
		return false;

	std::wstring ext = filepath.substr(i_ext, filepath.length() - i_nxl);

	debugStream << L"filext: " << ext.c_str() << L" " << std::endl;
	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();

	if (SUPPORTED_FILE_EXT.find(ext) != SUPPORTED_FILE_EXT.end()) {
		debugStream << L"filext: " << ext.c_str() << L" is supported" << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return true;
	}
	else {
		debugStream << L"filext: " << ext.c_str() << L" is not supported" << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return false;
	}
}

void setParentRPMFolder(std::wstring filepath) {
	std::wstringstream debugStream;

	size_t i_slash = filepath.rfind(L"\\");
	if (i_slash != std::wstring::npos) {
		dirPathTemp = filepath.substr(0, i_slash);
		debugStream << L"dirPathTemp: " << dirPathTemp.c_str() << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		uint32_t dirStatus;
		SDRmRPMFolderOption option;
		std::wstring tags;
		SDWLResult result = pDRmInstance->IsRPMFolder(dirPathTemp, &dirStatus, &option, tags);
		if (!result) {
			debugStream << L"failed to check isRPMFolder: " << dirPathTemp.c_str() << std::endl;
			OutputDebugStringW(debugStream.str().c_str());
			debugStream.str(L"");
			debugStream.flush();
		}
		else if (!_IsRPMDir(dirStatus)) {
			SDWLResult resTemp = pDRmInstance->AddRPMDir(dirPathTemp, SDRmRPMFolderOption::RPMFOLDER_OVERWRITE);

			if (resTemp.GetCode() == 0)
				debugStream << L"setRPMFolder[Temp] is done" << std::endl;
			else
				debugStream << L"setRPMFolder[Temp] is failed: " << resTemp.GetCode() << L"-" << resTemp.GetMsg().c_str() << std::endl;

			OutputDebugStringW(debugStream.str().c_str());
			debugStream.str(L"");
			debugStream.flush();
		}
	}
}

void NormalizeWatermarkText(ISDRmUser* pDRmUser, std::wstring& text, const std::wstring& nxlFile, const TEvalAttributeList& ctxAttrs, const std::vector<SDRmFileRight>& rights) {
	if (text.empty())
		return;

	std::wstring textLCase(text);
	RMXUtils::ToLower<wchar_t>(textLCase);

	// Translate new line keyword
	RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_BREAK, std::wstring(L"\n"));
	// $(user) - RMD user email
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_USER) != std::wstring::npos) {
		const std::wstring& rmsUser = pDRmUser->GetEmail();
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_USER, rmsUser);
	}
	// $(host) - host name
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_HOST) != std::wstring::npos) {
		static std::wstring hostName = RMXUtils::GetHostName();
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_HOST, hostName);
	}
	//$(ip) - IP address
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_IP) != std::wstring::npos)
	{
		static std::wstring hostName = RMXUtils::GetHostIP();
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_IP, hostName);
	}
	//$(date) - Local today's date
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_DATE) != std::wstring::npos) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		WCHAR	dateFormat[32];
		WCHAR	dateString[32];
		if (GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_SHORTDATE, &st, NULL, dateFormat, sizeof(dateFormat), NULL)) {
			swprintf_s(dateString, 32, dateFormat, st.wYear, st.wMonth, st.wDay);
		}
		else {
			swprintf_s(dateString, 32, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
		}

		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_DATE, dateString);
	}
	//$(time) - Current local time
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_TIME) != std::wstring::npos) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		WCHAR	timeFormat[32];
		WCHAR	timeString[32];
		if (GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, &st, NULL, timeFormat, sizeof(timeFormat))) {
			swprintf_s(timeString, 32, timeFormat, st.wHour, st.wMinute, st.wMinute);
		}
		else {
			swprintf_s(timeString, 32, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
		}

		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_TIME, timeString);
	}
	//$(right) - Permissions
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_RIGHT) != std::wstring::npos)
	{
		std::wstring rightNames;
		for (auto r : rights)
		{
			if (rightNames.empty())
				rightNames = RIGHTSMAP[(SDRmFileRight)r];
			else
				rightNames += L"+" + RIGHTSMAP[r];
		}
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_RIGHT, rightNames);
	}
	//$(classification) - Permissions
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_CLASSIFY) != std::wstring::npos)
	{
		std::wstring tags(L"");
		pDRmInstance->RPMReadFileTags(nxlFile, tags);

		const std::string& selfTags = RMXUtils::ws2s(tags);
		RMXLib::CTagCollection tagColl(selfTags);
		TEvalAttributeList attrs = tagColl.toEvalAttrs();
		std::wstring tagStrings;
		for (size_t i = 0; i < attrs.size(); i++)
		{
			if (i > 0)
				tagStrings += L";";

			tagStrings += attrs[i].first + L"=" + attrs[i].second;
		}
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_CLASSIFY, tagStrings);
	}

	//$(filename) - File name
	if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_FILENAME) != std::wstring::npos) {
		std::experimental::filesystem::path p(nxlFile);
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_FILENAME, p.stem());
	}
	//$(filepath) - File path

	std::wstringstream debugStream;

	// Support specific placeholders which can be customized by individual CAD RMX.
	// e.g.: Creo passes "modelname" with model name of file as context attribute to evaluate.
	// Can access modelname as watermark text. 
	size_t nextPos = 0;
	std::wstring textToParse(text);

	while (nextPos < textToParse.length()) {
		size_t startPos = textToParse.find_first_of(L'$', nextPos);
		if (startPos == std::wstring::npos)
			break;

		size_t endPos = textToParse.find_first_of(L')', startPos);
		if (endPos == std::wstring::npos)
			break;

		std::wstring placeHolder = textToParse.substr(startPos + 2, endPos - startPos - 2);
		bool findCtxAttr = false;

		for (const auto& ctxAttr : ctxAttrs) {
			if (_wcsicmp(ctxAttr.first.c_str(), placeHolder.c_str()) == 0) {
				placeHolder = L"$(" + placeHolder + L")";
				RMXUtils::IReplaceAllString<wchar_t>(text, placeHolder, ctxAttr.second);
				findCtxAttr = true;
				break;
			}
		}

		// If attribute not found for this placeholder, replace placeholder as empty.		
		if (!findCtxAttr) {
			placeHolder = L"$(" + placeHolder + L")";
			size_t pos = text.find(placeHolder);

			if (pos != std::string::npos) {
				// If found then erase it from string
				text.erase(pos, placeHolder.length());
			}
		}
		nextPos = endPos;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_racrmx_RACRMXInstance_loadDRmUser(JNIEnv *env, jobject jobj) {
	bool result = CRMXInstance::GetInstance().Initialize();

	if (result) {
		pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
		return JNI_TRUE;
	}
	else {
		return JNI_FALSE;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_racrmx_RACRMXInstance_loadDRmInstance(JNIEnv *env, jobject jobj) {
	bool result = CRMXInstance::GetInstance().Initialize();

	if (result) {
		pDRmInstance = CRMXInstance::GetInstance().GetDRmcInstance();
		return JNI_TRUE;
	}
	else {
		return JNI_FALSE;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_racrmx_RACRMXInstance_isFileProtected(JNIEnv *env, jobject jobj, jstring filepath) {
	std::wstringstream debugStream;

	const char *raw = env->GetStringUTFChars(filepath, NULL);
	std::string tmpPath(raw);
	std::wstring nxlPath = RMXUtils::s2ws(tmpPath);

	bool result = pDRmUser->IsFileProtected(nxlPath);

	if (result) {
		debugStream << L"nxlPath: " << nxlPath.c_str() << " is protected file" << std::endl;

		return JNI_TRUE;
	}
	else {
		debugStream << L"nxlPath: " << nxlPath.c_str() << " is not protected file" << std::endl;

		return JNI_FALSE;
	}

	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();
}

/*
* Class:     com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance
* Method:    setViewOverlay
* Signature: (J)J
*/
JNIEXPORT jlong JNICALL Java_com_nextlabs_racrmx_RACRMXInstance_setViewOverlay(JNIEnv *env, jobject jobj, jlong hwnd) {
	handle = (HWND)hwnd;
	std::wstringstream debugStream;
	debugStream << L"handle: " << handle << "set" << std::endl;
	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();

	// set RPM folders
	char usrHomeFolder[MAX_PATH];
	char tmpDirPathOffice[MAX_PATH];
	char tmpDirPathVisView[MAX_PATH];
	char tmpDirPathFCC[MAX_PATH];

	HRESULT result = SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, usrHomeFolder);
	if (SUCCEEDED(result)) {
		strncpy(tmpDirPathOffice, usrHomeFolder, sizeof(tmpDirPathOffice));
		strncpy(tmpDirPathVisView, usrHomeFolder, sizeof(tmpDirPathVisView));
		strncpy(tmpDirPathFCC, usrHomeFolder, sizeof(tmpDirPathFCC));

		dirPathOffice1 = RMXUtils::s2ws(tmpDirPathOffice) + L"\\AppData\\Local\\Microsoft\\Windows\\Temporary Internet Files\\Content.Word";
		dirPathOffice2 = RMXUtils::s2ws(tmpDirPathOffice) + L"\\AppData\\Local\\Microsoft\\Windows\\INetCache\\Content.Word";
		dirPathFCC = RMXUtils::s2ws(tmpDirPathFCC) + L"\\FCCCache";

		debugStream << L"setRPMFolder[Office1]: " << dirPathOffice1.c_str() << std::endl;
		debugStream << L"setRPMFolder[Office2]: " << dirPathOffice2.c_str() << std::endl;
		debugStream << L"setRPMFolder[FCC]: " << dirPathFCC.c_str() << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		SDWLResult resOffice1 = pDRmInstance->AddRPMDir(dirPathOffice1, SDRmRPMFolderOption::RPMFOLDER_EXT | SDRmRPMFolderOption::RPMFOLDER_OVERWRITE);
		SDWLResult resOffice2 = pDRmInstance->AddRPMDir(dirPathOffice2, SDRmRPMFolderOption::RPMFOLDER_EXT | SDRmRPMFolderOption::RPMFOLDER_OVERWRITE);
		SDWLResult resFCC = pDRmInstance->AddRPMDir(dirPathFCC, SDRmRPMFolderOption::RPMFOLDER_NORMAL);

		if (resOffice1.GetCode() == 0)
			debugStream << L"setRPMFolder[Office1] is done" << std::endl;
		else
			debugStream << L"setRPMFolder[Office1] is failed: " << resOffice1.GetCode() << L"-" << resOffice1.GetMsg().c_str() << std::endl;

		if (resOffice2.GetCode() == 0)
			debugStream << L"setRPMFolder[Office2] is done" << std::endl;
		else
			debugStream << L"setRPMFolder[Office2] is failed: " << resOffice2.GetCode() << L"-" << resOffice2.GetMsg().c_str() << std::endl;

		if (resFCC.GetCode() == 0)
			debugStream << L"setRPMFolder[FCC] is done" << std::endl;
		else
			debugStream << L"setRPMFolder[FCC] is failed: " << resFCC.GetCode() << L"-" << resFCC.GetMsg().c_str() << std::endl;

		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();
	}

	RPMInitViewOverlayInstance();

	return 0;
}

JNIEXPORT jlong JNICALL Java_com_nextlabs_racrmx_RACRMXInstance_renderOverlay(JNIEnv *env, jobject jobj, jstring filepath) {
	std::wstringstream debugStream;
	debugStream << L"handle: " << handle << L" is used" << std::endl;

	const char *raw = env->GetStringUTFChars(filepath, NULL);
	std::string tmpPath(raw);
	std::wstring nxlPath = RMXUtils::s2ws(tmpPath);

	debugStream << L"receivedJPath: " << raw << std::endl;
	debugStream << L"nxlFilePath: " << nxlPath.c_str() << std::endl;
	OutputDebugStringW(debugStream.str().c_str());
	debugStream.str(L"");
	debugStream.flush();

	if (!isFileSupported(nxlPath))
		return 0;

	setParentRPMFolder(nxlPath);

	std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> rightsAndWatermarks;
	SDWLResult resFileTags = pDRmUser->GetRights(nxlPath, rightsAndWatermarks, 0);

	// init watermark info with default value
	std::wstring rightName;
	std::wstring overlayText;
	std::wstring fontName = L"Arial";
	int fontSize = 22;
	int fontRotation = 45;
	int fontStyle = FontStyle_Regular;
	int textAlignment = TextAlign_Left;
	std::tuple<int, int, int, int> fontColor = {70, 0, 128, 21};

	for (auto r : rightsAndWatermarks) {
		rightName = RIGHTSMAP[(SDRmFileRight)r.first];

		if (rightName == L"View") {

			for (auto o : r.second) {
				//ifs << L"watermark info : fc" << o.fontColor.c_str() << L" fn" << o.fontName.c_str() << L" sz" << o.fontSize << L" rp" << o.repeat << L" ro" << o.rotation << L" t" << o.text.c_str() << std::endl;
				/*debugStream << L"watermark info : fc" << o.fontColor.c_str() << L" fn" << o.fontName.c_str() << L" sz" << o.fontSize << L" rp" << o.repeat << L" ro" << o.rotation << L" t" << o.text.c_str() << std::endl;
				OutputDebugStringW(debugStream.str().c_str());
				debugStream.str(L"");
				debugStream.flush();*/

				// setting watermark info, other use default
				overlayText = RMXUtils::s2ws(o.text);
				//if (!o.fontColor.empty())
					// do color mapping
				if (!o.fontName.empty())
					fontName = RMXUtils::s2ws(o.fontName);
				if (o.fontSize > 0)
					fontSize = o.fontSize;
				if (o.rotation > 0)
					fontRotation = o.rotation;
			}
			break;
		}
	}

	if (rightName == L"View") {
		std::wstring tags(L"");
		pDRmInstance->RPMReadFileTags(nxlPath, tags);
		debugStream << L"Tags: " << tags.c_str() << std::endl;

		const std::string& selfTags = RMXUtils::ws2s(tags);
		RMXLib::CTagCollection tagColl(selfTags);
		TEvalAttributeList evalAttrs = tagColl.toEvalAttrs();

		std::vector<SDRmFileRight> rights;
		for (auto rw2 : rightsAndWatermarks)
		{
			rights.push_back(rw2.first);
		}
		
		NormalizeWatermarkText(pDRmUser, overlayText, nxlPath, evalAttrs, rights);
		bool result = RPMSetViewOverlay_Background(handle, overlayText, fontColor, fontName, 
			fontSize, fontRotation, fontStyle, textAlignment, { 0, 0, 0, 0 });

		debugStream << L"SetViewOverlay_Background: " << result << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();
	}

	return 0;
}

JNIEXPORT jlong JNICALL Java_com_nextlabs_racrmx_RACRMXInstance_clearViewOverlay(JNIEnv *env, jobject jobj) {
	std::wstringstream debugStream;

	RPMClearViewOverlay(handle);

	if (!dirPathOffice1.empty()) {
		SDWLResult result = pDRmInstance->RemoveRPMDir(dirPathOffice1, true);

		if (result.GetCode() == 0)
			debugStream << L"removeRPMFolder[Office1] is done" << std::endl;
		else
			debugStream << L"removeRPMFolder[Office1] is failed: " << result.GetCode() << L"-" << result.GetMsg().c_str() << std::endl;

		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();
	}

	if (!dirPathOffice2.empty()) {
		SDWLResult result = pDRmInstance->RemoveRPMDir(dirPathOffice2, true);

		if (result.GetCode() == 0)
			debugStream << L"removeRPMFolder[Office2] is done" << std::endl;
		else
			debugStream << L"removeRPMFolder[Office2] is failed: " << result.GetCode() << L"-" << result.GetMsg().c_str() << std::endl;

		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();
	}

	if (!dirPathTemp.empty()) {
		SDWLResult result = pDRmInstance->RemoveRPMDir(dirPathTemp, true);

		if (result.GetCode() == 0)
			debugStream << L"removeRPMFolder[Temp] is done" << std::endl;
		else
			debugStream << L"removeRPMFolder[Temp] is failed: " << result.GetCode() << L"-" << result.GetMsg().c_str() << std::endl;

		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();
	}

	// don't remove FCC folder as NXRMX is also using and it does not remove it after use

	return 0;
}

