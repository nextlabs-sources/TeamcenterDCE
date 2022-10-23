#include "OtkXUtils.h"

#include <pfcGlobal.h>
#include <wfcSession.h>

#include "..\common\CommonTypes.h"
#include "..\common\CreoRMXLogger.h"
#include "OtkXTypes.h"
#include <PathEx.h>
#include <RMXUtils.h>
#include "OtkXUIStrings.h"
#include <LibRMX.h>

namespace OtkXUtils
{
	cStringT CS2XS(const char* sz)
	{
		return XOPTCVT_CS2XS((cStringT)sz);
	}

	cWStringT CS2XS(const wchar_t* sz)
	{
		return XOPTCVT_CS2XS((cWStringT)sz);
	}

	const char* XS2CS(cStringT xs)
	{
		return XOPTCVT_XS2CS((cStringT)xs);
	}

	const wchar_t* XS2CS(cWStringT xs)
	{
		return XOPTCVT_XS2CS((cWStringT)xs);
	}

	pfcModel_ptr OtkX_GetModelFromDescr(pfcModelDescriptor_ptr pMdlDescr)
	{
		if (pMdlDescr)
		{
			try
			{
				pfcSession_ptr pSession = pfcGetProESession();
				if (pSession != nullptr)
				{
					return pSession->GetModelFromDescr(pMdlDescr);
				}
			}
			OTKX_EXCEPTION_HANDLER();
		}
		return nullptr;
	}

	pfcModel_ptr OtkX_GetCurrentModel()
	{
		try
		{
			pfcSession_ptr pSession = pfcGetProESession();
			if (pSession)
			{
				return pSession->GetCurrentModel();
			}
		}
		OTKX_EXCEPTION_HANDLER();
		return nullptr;
	}

	xstring OtkX_GetStringParamValue(pfcModel_ptr pMdl, xrstring paramName)
	{
		try
		{
			auto pParamOwner = pfcParameterOwner::cast(pMdl);
			if (pParamOwner == nullptr)
				return xwstringnil;

			pfcParameter_ptr pParam = pParamOwner->GetParam(paramName);
			if (pParam == nullptr)
				return xwstringnil;

			auto pParamValue = pParam->GetValue();
			if (pParamValue == nullptr)
				return xwstringnil;

			return pParamValue->GetStringValue();
		}
		OTKX_EXCEPTION_HANDLER();
		return xwstringnil;
	}

	bool OtkX_GetBoolParamValue(pfcModel_ptr pMdl, xrstring paramName)
	{
		try
		{
			auto pParamOwner = pfcParameterOwner::cast(pMdl);
			if (pParamOwner == nullptr)
				return xwstringnil;

			pfcParameter_ptr pParam = pParamOwner->GetParam(paramName);
			if (pParam == nullptr)
				return xwstringnil;

			auto pParamValue = pParam->GetValue();
			if (pParamValue == nullptr)
				return xwstringnil;

			return (pParamValue->GetBoolValue() == xtrue);
		}
		OTKX_EXCEPTION_HANDLER();
		return false;
	}

	void OtkX_SetStringParamValue(pfcModel_ptr pMdl, const xstring& paramName, const xstring& paramValue)
	{
		try
		{
			auto pParamOwner = pfcParameterOwner::cast(pMdl);
			if (pParamOwner == nullptr)
				return;

			auto pNewParamValue = pfcCreateStringParamValue(cStringT(paramValue));
			pfcParameter_ptr pParam = pParamOwner->GetParam(cStringT(paramName));
			if (pParam != nullptr)
			{
				// Parameter has already existed, update value
				auto pParamValue = pParam->GetValue();
				if (pParamValue != nullptr && pParamValue->GetStringValue() != paramValue)
				{
					pParam->SetValue(pNewParamValue);
					LOG_INFO_FMT(L"'%s' parameter updated(Model:'%s'). New value: '%s'", CXS2W(paramName), CXS2W(pMdl->GetFileName()), CXS2W(paramValue));
				}
			}
			else
			{
				// Create the parameter with the given value
				pParam = pParamOwner->CreateParam(cStringT(paramName), pNewParamValue);
				pParam->SetDescription("A parameter indicates if file origin is protected. Used by RMX.");
				LOG_INFO_FMT(L"'%s' parameter created(Model:'%s'). Value: '%s'", CXS2W(paramName), CXS2W(pMdl->GetFileName()), CXS2W(paramValue));
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}

	void OtkX_SetBoolParamValue(pfcModel_ptr pMdl, const xstring& paramName, bool paramValue)
	{
		try
		{
			auto pParamOwner = pfcParameterOwner::cast(pMdl);
			if (pParamOwner == nullptr)
				return;
			xbool value = paramValue ? xtrue : xfalse;
			auto pParamValue = pfcCreateBoolParamValue(value);
			pfcParameter_ptr pParam = pParamOwner->GetParam(cStringT(paramName));
			if (pParam != nullptr)
			{
				auto pParamValue = pParam->GetValue();
				if (pParamValue != nullptr && pParamValue->GetBoolValue() != value)
				{
					pParam->SetValue(pParamValue);
					LOG_INFO_FMT(L"Parameter '%s' updated(Model:'%s'). New value: '%s'", CXS2W(paramName), CXS2W(pMdl->GetFileName()), paramValue ? L"yes" : L"no");
				}
			}
			else
			{
				pParam = pParamOwner->CreateParam(cStringT(paramName), pParamValue);
				pParam->SetDescription("A parameter to store file origin if model is saved. Used by RMX.");
				LOG_INFO_FMT(L"Parameter '%s' created(Model:'%s'). Value: '%s'", CXS2W(paramName), CXS2W(pMdl->GetFileName()), paramValue ? L"yes" : L"no");
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}

	void OtkX_RemoveParam(pfcModel_ptr pMdl, const xstring& paramName)
	{
		try
		{
			auto pParamOwner = pfcParameterOwner::cast(pMdl);
			if (pParamOwner == nullptr)
				return;

			pfcParameter_ptr pParam = pParamOwner->GetParam(cStringT(paramName));
			if (pParam != nullptr)
			{
				pParam->Delete();
				LOG_INFO_FMT(L"Parameter '%s' deleted(Model: '%s')", CXS2W(paramName), CXS2W(pMdl->GetFileName()));
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}
	
	pfcModelType OtkX_GetModelType(const wstring& fileExt)
	{
		if (fileExt.empty())
			return pfcModelType_nil;

		xstring ext = CW2XS(fileExt.c_str());
		ext = ext.ToLower();
		if (ext.Substring(0, 1) == ".")
			ext = ext.Substring(1);

		// mappings taken from pfcModel.ModelDescriptor.GetExtension() doc
		if (ext == "asm") return pfcMDL_ASSEMBLY;
		else if (ext == "prt") return pfcMDL_PART;
		else if (ext == "drw") return pfcMDL_DRAWING;
		else if (ext == "sec") return pfcMDL_2D_SECTION; // ignore this because it's used by 2 model types
		else if (ext == "lay") return pfcMDL_LAYOUT;
		else if (ext == "frm") return pfcMDL_DWG_FORMAT;
		else if (ext == "mfg") return pfcMDL_MFG;
		else if (ext == "rep") return pfcMDL_REPORT;
		else if (ext == "mrk") return pfcMDL_MARKUP;
		else if (ext == "dgm") return pfcMDL_DIAGRAM;
		else if (ext == "cem") return pfcMDL_CE_SOLID;
		else return pfcModelType_nil;
	}

	bool OtkX_IsNativeModelType(const wstring& fileExt)
	{
		return (OtkX_GetModelType(fileExt) != pfcModelType_nil);
	}

	wstring OtkX_GetImageExtention(const xstring& imageType)
	{
		wstring imageExt;
		xstring imageTypeLCase = imageType.ToLower();
		if (imageTypeLCase == "jpeg")
			imageExt = L"jpg";
		else if (imageTypeLCase == "bmp")
			imageExt = L"bmp";
		else if (imageTypeLCase == "tiff")
			imageExt = L"tif";
		else if (imageTypeLCase == "gif")
			imageExt = L"gif";
		else if (imageTypeLCase == "cgm")
			imageExt = L"cgm";
		return imageExt;
	}

	bool OtkX_OpenFile(xrstring fileFullPath)
	{
		const wstring& wFileFullPath = RMXUtils::s2ws(fileFullPath);
		LOG_INFO_FMT(L"Starting to retrieve model: '%s'...", wFileFullPath.c_str());
		try
		{
			wfcWSession_ptr pSess = wfcWSession::cast(pfcGetProESession());
			if (pSess)
			{
				wfcParsedFileNameData_ptr pFileData = pSess->ParseFileName(fileFullPath);
				if (pFileData == nullptr)
					return false;
				
				wstring ext = CXS2W(pFileData->GetExtension());
				pfcModelType mdlType = OtkX_GetModelType(ext);
				if (mdlType == pfcModelType_nil)
				{
					LOG_INFO(L"Invalid model type: " << ext.c_str());
					return false;
				}			

				xstring saveDir = pSess->GetCurrentDirectory();
				bool currDirChanged = false;
				if (saveDir != pFileData->GetDirectoryPath())
				{
					LOG_INFO_FMT(L"Change current directory before retrieving model: '%s' to '%s'", CXS2W(saveDir), CXS2W(pFileData->GetDirectoryPath()));
					pSess->ChangeDirectory(pFileData->GetDirectoryPath());
					currDirChanged = true;
					RMX_AddRPMDir(CXS2W(saveDir));
				}
				CPathEx filePathEx(wFileFullPath.c_str());
				char buffer[FILENAME_MAX];
				sprintf(buffer, "%s%s", RMXUtils::ws2s(filePathEx.GetFileName()).c_str(), RMXUtils::ws2s(filePathEx.GetExtension()).c_str());
				xstring fileName(buffer);
				pfcModelDescriptor_ptr pDesc = pfcModelDescriptor::Create(mdlType, fileName, "");
				if (pDesc)
				{
					pfcModel_ptr pMdl = pSess->GetModelFromDescr(pDesc);
					if (pMdl == nullptr)
					{
						OTKX_MODELDESCRIPTOR_DUMP(pDesc);
						pMdl = pSess->RetrieveModel(pDesc);
						
						pfcWindow_ptr pWindow = pSess->CreateModelWindow(pMdl);
						if (pWindow != nullptr)
							pWindow->Activate();
						LOG_INFO_FMT(L"Model retrieved: '%s'", CXS2W(fileName));
					}
					else
					{
						LOG_INFO_FMT(L"Ignore. Model already in memory: '%s'", CXS2W(fileName));
					}
				}

				if (currDirChanged)
				{
					pSess->ChangeDirectory(saveDir);
					RMX_AddRPMDir(pFileData->GetDirectoryPath());
					LOG_INFO_FMT(L"Revert current directory after retrieving model");
				}	

				return true;
			}
		}
		OTKX_EXCEPTION_HANDLER();

		LOG_INFO_FMT(L"Failed to retrieve model");

		return false;
	}

	void OtkX_ShowMessageDialog(xrstring msg, pfcMessageDialogType dlgType)
	{
		if (!OtkX_RunInGraphicMode())
		{
			LOG_WARN(L"No graphics mode. Do not show message dialog");
			return;
		}
		try
		{
			wfcWSession_ptr pSess = wfcWSession::cast(pfcGetProESession());
			if (pSess)
			{
				pfcMessageDialogOptions_ptr pMsgOpts = pfcMessageDialogOptions::Create();
				pfcMessageButtons_ptr pMsgBtns = pfcMessageButtons::create();
				pMsgBtns->append(pfcMESSAGE_BUTTON_OK);
				pMsgOpts->SetButtons(pMsgBtns);
				pMsgOpts->SetDialogLabel(OTKX_MESSAGEDLG_LABEL);
				pMsgOpts->SetMessageDialogType(dlgType);
				pSess->UIShowMessageDialog(msg, pMsgOpts);
			}
			
		}
		OTKX_EXCEPTION_HANDLER();
	}
	
	pfcMessageButton OtkX_ShowMessageDialog2(xrstring msg, pfcMessageDialogType dlgType)
	{
		if (!OtkX_RunInGraphicMode())
		{
			LOG_WARN(L"No graphics mode. Do not show message dialog");
			return pfcMESSAGE_BUTTON_YES; // Default to Yes.
		}
		try
		{
			wfcWSession_ptr pSess = wfcWSession::cast(pfcGetProESession());
			if (pSess)
			{
				pfcMessageDialogOptions_ptr pMsgOpts = pfcMessageDialogOptions::Create();
				pfcMessageButtons_ptr pMsgBtns = pfcMessageButtons::create();
				pMsgBtns->append(pfcMESSAGE_BUTTON_OK);
				pMsgBtns->append(pfcMESSAGE_BUTTON_CANCEL);
				pMsgOpts->SetButtons(pMsgBtns);
				pMsgOpts->SetDialogLabel(OTKX_MESSAGEDLG_LABEL);
				pMsgOpts->SetMessageDialogType(dlgType);
				return pSess->UIShowMessageDialog(msg, pMsgOpts);
			}

		}
		OTKX_EXCEPTION_HANDLER();

		return pfcMESSAGE_BUTTON_CANCEL; // Cancel in case of any exception.
	}

	pfcMessageButton OtkX_ShowMessageDialog3(xrstring msg, pfcMessageDialogType dlgType)
	{
		if (!OtkX_RunInGraphicMode())
		{
			LOG_WARN(L"No graphics mode. Do not show message dialog");
			return pfcMESSAGE_BUTTON_YES; // Default to Yes.
		}
		try
		{
			wfcWSession_ptr pSess = wfcWSession::cast(pfcGetProESession());
			if (pSess)
			{
				pfcMessageDialogOptions_ptr pMsgOpts = pfcMessageDialogOptions::Create();
				pfcMessageButtons_ptr pMsgBtns = pfcMessageButtons::create();
				pMsgBtns->append(pfcMESSAGE_BUTTON_YES);
				pMsgBtns->append(pfcMESSAGE_BUTTON_NO);
				pMsgBtns->append(pfcMESSAGE_BUTTON_CANCEL);
				pMsgOpts->SetButtons(pMsgBtns);
				pMsgOpts->SetDialogLabel(OTKX_MESSAGEDLG_LABEL);
				pMsgOpts->SetMessageDialogType(dlgType);
				return pSess->UIShowMessageDialog(msg, pMsgOpts);
			}

		}
		OTKX_EXCEPTION_HANDLER();

		return pfcMESSAGE_BUTTON_CANCEL; // Cancel in case of any exception.
	}

	bool OtkX_IsExportFileType(const wchar_t* szFilePath)
	{
		const wstring& fileExt = OtkX_GetFileExtension(szFilePath);
		if (fileExt.empty() || (wcsicmp(fileExt.c_str(), NXL_FILE_EXT_WITHOUT_DOT) == 0))
			return false;
		
		if (wcsicmp(fileExt.c_str(), L"dxf") == 0)
		{
			// Filter away some files generated temporarily during export.
			CPathEx pathEx(szFilePath);
			return (DXFFILE_BLACKLIST.find(pathEx.GetFileName()) == DXFFILE_BLACKLIST.end());
		}

		return (EXPORT_FILETYPE_TABLE.find(fileExt) != EXPORT_FILETYPE_TABLE.end());
	}

	bool OtkX_IsSupportedFileType(const wchar_t * szFilePath)
	{
		const wstring& ext = OtkX_GetFileExtension(szFilePath);
		if (ext.empty() || (wcsicmp(ext.c_str(), NXL_FILE_EXT_WITHOUT_DOT) == 0))
			return false;

		if (OtkX_IsNativeModelType(ext))
		{
			return true;
		}

		return OtkX_IsExportFileType(szFilePath);
	}

	pfcModel_ptr OtkX_FindDepModel(pfcModel_ptr pParentMdl, const wchar_t * searchFileName)
	{
		LOG_DEBUG_FMT(L"Search dependent model with file name: '%s'", searchFileName);
		try
		{
			if (pParentMdl->GetType() == pfcMDL_ASSEMBLY)
			{
				pfcSolid_ptr pCurrSolid = nullptr;
				pfcComponentFeat_ptr pCurrFeat = nullptr;
				pfcModel_ptr pCurrMdl = nullptr;

				pCurrSolid = pfcSolid::cast(pParentMdl);
				if (pCurrSolid == nullptr)
					return nullptr;

				pfcFeatures_ptr pFeats = pCurrSolid->ListFeaturesByType(xfalse, pfcFEATTYPE_COMPONENT);
				if (pFeats == nullptr)
					return nullptr;

				for (xint i = 0; i < pFeats->getarraysize(); ++i)
				{
					pCurrFeat = pfcComponentFeat::cast(pFeats->get(i)); 
					if (pCurrFeat == nullptr)
						continue;

					pCurrMdl = OtkX_GetModelFromDescr(pCurrFeat->GetModelDescr());
					if (pCurrMdl == nullptr)
						continue;

					wstring mdlName = CXS2W(pCurrMdl->GetInstanceName());
					LOG_DEBUG_FMT(L"\t -Check dependent model(status: %d): %s", pCurrFeat->GetStatus(), mdlName.c_str());
					if (wcsicmp(mdlName.c_str(), searchFileName) == 0)
					{		
						LOG_INFO_FMT(L"\t - Dependent model found: %s", mdlName.c_str());
						return pCurrMdl;
					}
					
					if ((pCurrMdl = OtkX_FindDepModel(pCurrMdl, searchFileName)) != nullptr)
						return pCurrMdl;
				}
			}
		}
		OTKX_EXCEPTION_HANDLER();

		return nullptr;
	}
	
	wstring OtkX_GetFileExtension(const wchar_t * szFilePath)
	{
		CPathEx filePath(szFilePath);
		wstring ext = filePath.GetExtension();
		if (ext.empty())
			return ext;

		// Remove the "." header
		if (ext.front() == L'.')
			ext.erase(0, 1);

		xstring xExt(CW2XS(ext.c_str()));
		int nbSuffix;
		if (xExt.ToInt(&nbSuffix))
		{
			// In case the file name contains the version number like test.prt.1,
			// try to remove the version number
			CPathEx fileName(filePath.GetFileName().c_str());
			ext = fileName.GetExtension();
			if (ext.empty())
				return ext;

			// Remove the "." header
			if (ext.front() == L'.')
				ext.erase(0, 1);
		}

		return ext;
	}

	wstring OtkX_GetCurrentDirectory()
	{
		wstring currDir;
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			if (pSess != nullptr)
				currDir = CXS2W(pSess->GetCurrentDirectory());
		}
		OTKX_EXCEPTION_HANDLER();

		return currDir;
	}

	bool OtkX_RunInGraphicMode()
	{
		// Check if the session is running in no graphics mode
		auto cmdLines = RMXUtils::GetCommandLines();
		for (auto& cmdLine : cmdLines)
		{
			if (wcsstr(cmdLine.c_str(), L"-g:no_graphics") != NULL)
			{
				return false;
			}
		}

		return true;
	}

	void OtkX_DisplayMessageForNxlFileInfo(pfcModel_ptr pMdl)
	{
		if (!OtkX_RunInGraphicMode())
		{
			LOG_WARN(L"No graphics mode. Ignore to display info for protected model");
			return;
		}
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			if (pSess)
			{
				unsigned long fileRights = 0;
				RMX_GetRights(CXS2W(pMdl->GetOrigin()), fileRights);
				xstringsequence_ptr msgs = xstringsequence::create();
				xstring line(pMdl->GetFileName(), " (Permissions: ");

				if (HAS_BIT(fileRights, RMX_RIGHT_VIEW))
				{
					line += "View";
				}
				if (HAS_BIT(fileRights, RMX_RIGHT_EDIT))
				{
					line += ", Edit";
				}
				if (HAS_BIT(fileRights, RMX_RIGHT_SAVEAS) || HAS_BIT(fileRights, RMX_RIGHT_DOWNLOAD))
				{
					line += ", Save As";
				}
				if (HAS_BIT(fileRights, RMX_RIGHT_PRINT))
				{
					line += ", Print";
				}
				if (HAS_BIT(fileRights, RMX_RIGHT_SHARE))
				{
					line += ", ReShare";
				}
				if (HAS_BIT(fileRights, RMX_RIGHT_DECRYPT))
				{
					line += ", Extract";
				}
				line += ")";
				msgs->append(line);
				pSess->UIDisplayMessage(OTKX_MSGFILE, "RMXFileLoaded", msgs);
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}

	pfcModel_ptr OtkX_GetModelFromFileName(const wstring& fileName)
	{
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			if (pSess != nullptr)
				return pSess->GetModelFromFileName(RMXUtils::ws2s(fileName).c_str());
		}
		OTKX_EXCEPTION_HANDLER();
	
		return nullptr;
	}

	void OtkX_CloseModel(pfcModel_ptr pMdl)
	{
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			if (pSess)
			{
				LOG_DEBUG(L"Closing model : " << CXS2W(pMdl->GetInstanceName()));
				auto pWnd = pSess->GetModelWindow(pMdl);
				if (pWnd)
					pWnd->Close();
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}

	bool OtkX_EraseModel(pfcModel_ptr pMdl, bool includeDep /*= true*/)
	{
		try
		{
			LOG_DEBUG(L"Erasing model : " << CXS2W(pMdl->GetInstanceName()));
			pfcSession_ptr pSess = pfcGetProESession();
			if (pSess)
			{
				pSess->EraseUndisplayedModels();
				LOG_DEBUG("Undisplay models erased");
			}
			
			if (includeDep)
			{
				pMdl->EraseWithDependencies();
				LOG_DEBUG("Model erased with dependencies");
			}
			else
			{
				pMdl->Erase();
				LOG_DEBUG("Model erased");
			}

			return true;
		}
		OTKX_EXCEPTION_HANDLER();

		return false;
	}

	void OtkX_ReopenFile(const wstring& fileName)
	{
		LOG_DEBUG(L"Reopenning file : " << fileName.c_str());
		string filePath = RMXUtils::ws2s(fileName);
		RMXUtils::IReplaceAllString<char>(filePath, "\\", "\\\\");
		std::string macros = "~ Command `ProCmdModelOpen` ;~ Trail `UI Desktop` `UI Desktop` `DLG_PREVIEW_POST` `file_open`;~ Update `file_open` `Inputname` `" + filePath + "`;~ Trail `UI Desktop` `UI Desktop` `PREVIEW_POPUP_TIMER` `file_open:Ph_list.Filelist:<NULL>`;~ Command `ProFileSelPushOpen@context_dlg_open_cmd`;";
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			pSess->RunMacro(CA2XS(macros.c_str()));
		}
		OTKX_EXCEPTION_HANDLER();
	}

	OtkXFileNameData::OtkXFileNameData()
		: m_version(-1)
	{
	}

	OtkXFileNameData::OtkXFileNameData(const std::wstring & fileName)
	{
		m_fullPath = fileName;

		CPathEx pathParser(fileName.c_str());
		m_directoryPath = pathParser.GetParentPath();
		if (m_directoryPath.back() == L'\\' || m_directoryPath.back() == L'/')
			m_directoryPath.pop_back();

		m_extension = pathParser.GetExtension();
		if (m_extension.front() == L'.')
			m_extension.erase(0, 1);
		m_name = pathParser.GetFileName();
		if (!m_extension.empty())
		{
			xstring xExt(CW2XS(m_extension.c_str()));
			int nbSuffix;
			if (xExt.ToInt(&nbSuffix))
			{
				// In case the file name contains the version number like test.prt.1,
				// try to remove the version number
				CPathEx innerPath(pathParser.GetFileName().c_str());
				m_extension = innerPath.GetExtension();
				if (m_extension.front() == L'.')
					m_extension.erase(0, 1);
				m_name = innerPath.GetFileName();
				m_version = nbSuffix;
			}
		}
	}

	OtkXFileNameData::OtkXFileNameData(const std::wstring & dir, const std::wstring & name, const std::wstring & ext, int version)
		: m_directoryPath(dir)
		, m_name(name)
		, m_extension(ext)
		, m_version(version)
	{
		if (m_directoryPath.back() == L'\\' || m_directoryPath.back() == L'/')
			m_directoryPath.pop_back();

		if (m_extension.front() == L'.')
			m_extension.erase(0, 1);

		m_fullPath = m_directoryPath + L'\\' + GetFileName();
		if (m_version > -1)
		{
			m_fullPath += std::to_wstring(m_version);
		}
	}

	bool OtkXFileNameData::IsSameVersionFile(const OtkXFileNameData & other) const
	{
		if (wcsicmp(m_directoryPath.c_str(), other.GetDirectoryPath().c_str()) == 0 &&
			wcsicmp(m_name.c_str(), other.GetBaseName().c_str()) == 0 &&
			wcsicmp(m_extension.c_str(), other.GetExtension().c_str()) == 0)
			return true;
		return false;
	}
}
