#include "RMXViewOverlay.h"

#include <algorithm>
#include <windows.h>
#include <tuple>

#include "RMXLogger.h"
#include "RMXTagHlp.h"
#include "RMXUtils.h"

#include <SDLInstance.h>
#include <SDLUser.h>

//static const wchar_t* OB_NAME_WATERMARK = L"OB_OVERLAY";
//static const wchar_t* OB_OPTION_WATERMARK_TEXT = L"Text";
//static const wchar_t* OB_OPTION_WATERMARK_FONT_NAME = L"FontName";
//static const wchar_t* OB_OPTION_WATERMARK_FONT_COLOR = L"TextColor";
//static const wchar_t* OB_OPTION_WATERMARK_FONT_SIZE = L"FontSize";
//static const wchar_t* OB_OPTION_WATERMARK_TRANSPARENCY = L"Transparency";
//static const wchar_t* OB_OPTION_WATERMARK_ROTATION = L"Rotation";
//static const wchar_t* OB_OPTION_WATERMARK_FONT_STYLE = L"FontStyle";
//static const wchar_t* OB_OPTION_WATERMARK_TEXT_ALIGNMENT = L"TextAlignment";


static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_USER = L"$(user)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_HOST = L"$(host)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_DATE = L"$(date)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_TIME = L"$(time)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_BREAK = L"$(break)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_RIGHT = L"$(right)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_IP = L"$(ip)";
static const wchar_t* WATERMARK_TEXT_PLACEHOLDER_CLASSIFY = L"$(classification)";

static const std::string LOGIN_PASSCODE = "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}";

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

//
// CRMXViewOverlay class to provide all internal facilities
//
class CRMXViewOverlay
{
public:
	CRMXViewOverlay(ISDRmcInstance* pDRmcInst, void* hTargetWnd)
		: m_hTargetWnd(hTargetWnd), m_pDRmcInst(pDRmcInst){};

	bool SetViewOverlay(const std::vector<std::string>& tags, const TEvalAttributeList& ctxAttrs,
		const std::tuple<int, int, int, int>& displayOffset = { 0, 0, 0, 0 });

	bool SetViewOverlay(const std::vector<std::wstring>& nxlFiles, const TEvalAttributeList& ctxAttrs,
		const std::tuple<int, int, int, int>& displayOffset = { 0, 0, 0, 0 });

	bool ClearViewOverlay();

	bool EvalWatermarkInfoFromPolicies(
		const std::wstring& nxlFile,
		const TEvalAttributeList& ctxAttrs);

	const RMXLib::RMX_WATERMARK_INFO& GetWatermarkInfo() const {
		return m_wmInfo;
	};

// Helper functions
private:
	void CovertHexColorToRGB(const std::wstring& hex, RMXLib::RMX_WATERMARK_COLOR& color)
	{
		// Drop a hash if the value has one
		std::wstring hexColor(hex);
		if (hexColor[0] == '#') {
			hexColor.erase(0, 1);
		}

		unsigned long value = std::stoul(hexColor, nullptr, 16);
		color.colorR = (value >> 16) & 0xff;
		color.colorG = (value >> 8) & 0xff;
		color.colorB = (value >> 0) & 0xff;
	}

	void InitWatermarkInfo(RMXLib::RMX_WATERMARK_INFO& wmInfo)
	{
		// Apply default values in case it's not configured in policy obligation
		// TBD: Need to provide default text? If yes, will cause watermark feature cannot be disabled.
		wmInfo.text.clear();
		wmInfo.fontName = L"Arial";
		wmInfo.fontSize = 22;
		wmInfo.fontRotation = 45;
		wmInfo.fontStyle = RMXLib::FontStyle_Regular;
		wmInfo.textAlignment = RMXLib::TextAlign_Left;
		wmInfo.fontColor = { 70, 0, 128, 21 };
	}

	void NormalizeWatermarkText(ISDRmUser* pDRmUser, std::wstring& text, const std::wstring& nxlFile, const TEvalAttributeList& ctxAttrs, 
		const std::vector<SDRmFileRight>& rights)
	{
		if (text.empty())
			return;

		std::wstring textLCase(text);
		RMXUtils::ToLower<wchar_t>(textLCase);

		// Since 13.2 release, call RPMGetRights API to retrieve watermark, RPM has parsed these pre-defined macros
		// Comment out them here.
		// Translate new line keyword
		RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_BREAK, std::wstring(L"\n"));
		// $(user) - RMD user email
		if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_USER) != std::wstring::npos)
		{
			const std::wstring& rmsUser = pDRmUser->GetEmail();
			RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_USER, rmsUser);
		}
		// $(host) - host name
		if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_HOST) != std::wstring::npos)
		{
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
		if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_DATE) != std::wstring::npos)
		{
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
		if (textLCase.find(WATERMARK_TEXT_PLACEHOLDER_TIME) != std::wstring::npos)
		{
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
			m_pDRmcInst->RPMReadFileTags(nxlFile, tags);

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
			RMXUtils::IReplaceAllString<wchar_t>(text, WATERMARK_TEXT_PLACEHOLDER_RIGHT, tagStrings);
		}
		// 1. Support specific placeholders which can be customized by individual CAD RMX.
		//	e.g.: Creo passes "modelname" with model name of file as context attribute to evaluate.
		//		  Then the watermakr definition can contain $(modelname)
		// 2. Support tag name as placeholder. watermark text is tag value 
		// e.g.: Watermark definition can contain $(ip_classification)
		size_t nextPos = 0;
		std::wstring textToParse(text);
		TEvalAttributeList evalAttrs;
		while (nextPos < textToParse.length())
		{
			size_t startPos = textToParse.find_first_of(L'$', nextPos);
			if (startPos == std::wstring::npos)
				break;

			size_t endPos = textToParse.find_first_of(L')', startPos);
			if (endPos == std::wstring::npos)
				break;
		
			if (evalAttrs.size() < ctxAttrs.size())
			{
				// Initialize the atribute list in the first time
				std::wstring tags(L"");
				m_pDRmcInst->RPMReadFileTags(nxlFile, tags);

				const std::string& selfTags = RMXUtils::ws2s(tags);
				RMXLib::CTagCollection tagColl(selfTags);
				evalAttrs = tagColl.toEvalAttrs();
				if (!ctxAttrs.empty())
					evalAttrs.insert(evalAttrs.end(), ctxAttrs.begin(), ctxAttrs.end());
				
			}

			std::wstring placeHolder = textToParse.substr(startPos + 2, endPos - startPos - 2);
			bool findCtxAttr = false;
			for (const auto& ctxAttr : evalAttrs)
			{
				if (_wcsicmp(ctxAttr.first.c_str(), placeHolder.c_str()) == 0)
				{
					placeHolder = L"$(" + placeHolder + L")";
					RMXUtils::IReplaceAllString<wchar_t>(text, placeHolder, ctxAttr.second);
					findCtxAttr = true;
					break;
				}
			}

			// If attribute not found for this placeholder, replace placeholder as empty.		
			if (!findCtxAttr)
			{
				placeHolder = L"$(" + placeHolder + L")";
				size_t pos = text.find(placeHolder);
				if (pos != std::string::npos)
				{
					// If found then erase it from string
					text.erase(pos, placeHolder.length());
				}
			}
			nextPos = endPos;
		}
	}

// Data members
private:
	RMXLib::RMX_WATERMARK_INFO m_wmInfo;
	ISDRmcInstance* m_pDRmcInst;
	void* m_hTargetWnd;
};

bool CRMXViewOverlay::SetViewOverlay(const std::vector<std::wstring>& nxlFiles, const TEvalAttributeList& ctxAttrs,
	const std::tuple<int, int, int, int>& displayOffset)
{
	CHK_ERROR_RETURN(m_pDRmcInst == nullptr, false, L"Cannot set view overlay(error: %s)", L"pDRmcInstance not specified");
	CHK_ERROR_RETURN(m_hTargetWnd == nullptr, false, L"Cannot set view overlay(error: %s)", L"Target window not specified");

	SDWLResult result;
	for (const auto& nxlFile : nxlFiles)
	{
		EvalWatermarkInfoFromPolicies(nxlFile, ctxAttrs);

		if (!m_wmInfo.text.empty())
		{
			ClearViewOverlay();

			result = m_pDRmcInst->RPMSetViewOverlay(
				m_hTargetWnd,
				m_wmInfo.text,
				std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>(
					m_wmInfo.fontColor.colorA,
					m_wmInfo.fontColor.colorR,
					m_wmInfo.fontColor.colorG,
					m_wmInfo.fontColor.colorB),
				m_wmInfo.fontName,
				m_wmInfo.fontSize,
				m_wmInfo.fontRotation,
				m_wmInfo.fontStyle,
				m_wmInfo.textAlignment,
				displayOffset);

			CHK_ERROR_RETURN(!result, false, L"Failed to set view overlay @%p (error: %s)", m_hTargetWnd, result.ToString().c_str());

			LOG_INFO_FMT(L"Set view overlay @%p", m_hTargetWnd);
			return true;
		}
		else
		{
			LOG_WARN(L"View watermark not defined, do not show view overlay");
		}
	}

	return false;
}

bool CRMXViewOverlay::ClearViewOverlay()
{
	CHK_ERROR_RETURN(m_pDRmcInst == nullptr, false, L"Cannot clear view overlay(error: %s)", L"pDRmcInstance not specified");
	CHK_ERROR_RETURN(m_hTargetWnd == nullptr, false, L"Cannot clear view overlay(error: %s)", L"Target window not specified");

	SDWLResult result = m_pDRmcInst->RPMClearViewOverlay(m_hTargetWnd);
	CHK_ERROR_RETURN(!result, false, L"Failed to clear view overlay @%p(error: %s)", m_hTargetWnd, result.ToString().c_str());

	LOG_INFO_FMT(L"Removed view overlay @%p", m_hTargetWnd);
	return true;
}

bool CRMXViewOverlay::EvalWatermarkInfoFromPolicies(
	const std::wstring& nxlFile,
	const TEvalAttributeList& ctxAttrs)
{
	ISDRmUser* pDRmUser = nullptr;
	SDWLResult result = m_pDRmcInst->RPMGetLoginUser(LOGIN_PASSCODE, &pDRmUser);
	CHK_ERROR_RETURN(!result || pDRmUser == nullptr, false, L"Failed to get logged in user (error: %s)", result.ToString().c_str());

	std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> rightsAndWatermarks;
	LOG_INFO(L"Evaluate watermark for the file..." << nxlFile.c_str());
	// Note: 3rd arguments for RPMGetFileRights must be 1 as 0 not working for NX
	// However, 1 is not to check owner, but watermark text will not be parsed by RMD. So we have
	// to parse manualy again here.
	result = m_pDRmcInst->RPMGetFileRights(nxlFile, rightsAndWatermarks, 0);

	InitWatermarkInfo(m_wmInfo);
	CHK_ERROR_RETURN(!result, false, L"Failed to get central policies (error: %s)", result.ToString().c_str());

	for (const auto& rw : rightsAndWatermarks)
	{
		if (rw.first != RIGHT_VIEW)
			continue;

		if (rw.second.size() < 1)
			break;

		m_wmInfo.text = RMXUtils::s2ws(rw.second.at(0).text);

		if (!m_wmInfo.text.empty())
		{
			RMXUtils::IReplaceAllString<wchar_t>(m_wmInfo.text, std::wstring(L"\\n"), std::wstring(L"\n"));

			std::wstring valTmp;
			std::wstring origText = m_wmInfo.text;
			std::for_each(origText.begin(), origText.end(), [&valTmp](wchar_t ch) {
				if (ch == L'\n') valTmp += L"\\n";  else valTmp += ch; });
			std::wstring msgWMInfo = L"\n\t-Text : " + valTmp;

			std::vector<SDRmFileRight> rights;
			for (auto rw2 : rightsAndWatermarks)
			{
				rights.push_back(rw2.first);
			}
			NormalizeWatermarkText(pDRmUser, m_wmInfo.text, nxlFile, ctxAttrs, rights);
			msgWMInfo += L"\n\t-EvalText : " + m_wmInfo.text;

			LOG_INFO_FMT(L" view watermark defined: %s", msgWMInfo.c_str());
		}

		break;
	}

	return m_wmInfo.text.empty() ? false : true;
}

namespace RMXLib
{
	
	bool ClearViewOverlay(ISDRmcInstance* pDRmcInstance, void* hTargetWnd)
	{
		CRMXViewOverlay overlay(pDRmcInstance, hTargetWnd);
		return overlay.ClearViewOverlay();
	}

	bool SetViewOverlay(
		ISDRmcInstance* pDRmcInstance,
		void* hTargetWnd,
		const std::vector<std::wstring>& nxlFiles,
		const TEvalAttributeList& ctxAttrs,
		const std::tuple<int, int, int, int>& displayOffset /*= { 0,0,0,0 }*/)
	{
		CRMXViewOverlay overlay(pDRmcInstance, hTargetWnd);
		return overlay.SetViewOverlay(nxlFiles, ctxAttrs, displayOffset);
	}

	bool EvalWatermarkInfo(ISDRmcInstance* pDRmcInstance, const std::wstring& nxlFile, const TEvalAttributeList& ctxAttrs, RMX_WATERMARK_INFO& wmInfo)
	{
		CRMXViewOverlay overlay(pDRmcInstance, NULL);
		bool ret = overlay.EvalWatermarkInfoFromPolicies(nxlFile, ctxAttrs);
		wmInfo = overlay.GetWatermarkInfo();

		return ret;
	}

} // namespace RMXLib

