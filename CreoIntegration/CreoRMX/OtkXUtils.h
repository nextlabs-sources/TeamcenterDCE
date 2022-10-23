#pragma once

#include <set>
#include <string> 
#include <Windows.h>

#include <pfcExceptions.h>
#include <pfcModel.h>
#include <pfcUI.h>
#include <xstring.h>

namespace OtkXUtils
{
	inline cWStringT CS2XS(const wchar_t* sz);
	inline cStringT CS2XS(const char* sz);
	
	inline const wchar_t* XS2CS(cWStringT xs);
	inline const char* XS2CS(cStringT xs);
	
#define CW2XS(szStr) XOPTCVT_CS2XS((cWStringT)szStr)
#define CXS2W(str) XOPTCVT_XS2CS((cWStringT)str)

#define CA2XS(szStr) XOPTCVT_CS2XS((cStringT)szStr)
#define CXS2A(str) XOPTCVT_XS2CS((cStringT)str)

#define OTKX_MSGFILE "rmx_msg.txt"

// Log macros
// Otk exception macros
#define OTKX_EXCEPTION_PRINT(pfcex) \
{ \
	xstring msg = pfcex->GetMessage(); \
	xstring method = pfcex->GetMethodName(); \
	xint err = pfcex->GetErrorCode(); \
	LOG_ERROR_FMT(L"pfcXToolkitError in method %s with error(%d) %s", CXS2W(method), err, CXS2W(msg)); \
}

/* Print uncaught/default exception (second parameter in macro) */
#define OTKX_DEFAULT_EXCEPTION_PRINT(defaultex) \
{ \
	xstring msg = defaultex->getCipTypeName(); \
	LOG_ERROR_FMT(L"Uncaught %s exception", CXS2W(msg)); \
}

/* Prints invalid file info */
#define OTKX_FILENAME_EXCEPTION_PRINT(fname) \
{ \
	xstring msg = fname->GetMessage(); \
	LOG_ERROR_FMT(L"Invalid File name: %s", CXS2W(msg)); \
}

/* Writes invalid mdl extension info (second parameter in macro) */
#define OTKX_MDLEXT_EXCEPTION_PRINT(mdlext) \
{ \
	xstring msg = mdlext->GetMessage(); \
	LOG_ERROR_FMT(L"Invalid mdl extension: %s", CXS2W(msg)); \
}

#define OTKX_EXCEPTION_HANDLER() \
xcatchbegin \
xcatch (pfcXToolkitError, pfcex) { \
	OTKX_EXCEPTION_PRINT(pfcex) } \
xcatch (pfcXInvalidFileName, pfcex) { \
	OTKX_FILENAME_EXCEPTION_PRINT(pfcex) } \
xcatch (pfcXUnknownModelExtension, pfcex) { \
	OTKX_MDLEXT_EXCEPTION_PRINT(pfcex) } \
xcatchcip(pfcex) { \
	OTKX_DEFAULT_EXCEPTION_PRINT(pfcex) } \
xcatchend

#define OTKX_MODELDESCR_TOSTR(pModelDesc, info) \
	{ \
		try { \
			pfcModelType type = pModelDesc->GetType(); \
			info = info + L"\n\t - Type: " + std::to_wstring((int)type).c_str(); \
			info = info + L"\n\t - FullName: " + pModelDesc->GetFullName(); \
			info = info + L"\n\t - Path: " + pModelDesc->GetPath(); \
			info = info + L"\n\t - FileName: " + pModelDesc->GetFileName(); \
			info = info + L"\n\t - Extension: " + pModelDesc->GetExtension(); \
			info = info + L"\n\t - InstanceName: " + pModelDesc->GetInstanceName(); \
			xstring genericName = pModelDesc->GetGenericName(); \
			if (genericName != xstringnil) \
				info = info + L"\n\t - GenericName: " + genericName; \
			xint ver = pModelDesc->GetFileVersion(); \
			info = info + L"\n\t - FileVersion: " + std::to_wstring(ver).c_str(); \
		} \
		OTKX_EXCEPTION_HANDLER(); \
	}

	#define OTKX_MODELDESCRIPTOR_DUMP(pModelDesc) \
	{ \
		if (pModelDesc != NULL ) \
		{ \
			xstring info; \
			OTKX_MODELDESCR_TOSTR(pModelDesc, info); \
			LOG_DEBUG(L"Dump pModelDesc(" << __FUNCTIONW__ << L"): " << info); \
		} \
		else \
			LOG_DEBUG(L"NULL pModelDesc(" << __FUNCTIONW__ << L")"); \
	}

	#define OTKX_MODELDESCCOPY_DUMP(pFrom, pTo) \
	{ \
		if (pFrom != NULL && pTo != NULL) \
		{ \
			xstring info; \
			OTKX_MODELDESCR_TOSTR(pFrom, info); \
			info = xstring(L"Dump model copy(") + xstring(__FUNCTIONW__) + xstring(L"):\npFromDescr: ") + info; \
			xstring infoTo; \
			OTKX_MODELDESCR_TOSTR(pTo, infoTo); \
			infoTo = L"\npToDescr: " + infoTo; \
			LOG_DEBUG(info << infoTo); \
		}\
	}
	#define OTKX_MODEL_TOSTR(pModel, info) \
	{ \
		try { \
			pfcModelType type = pModel->GetType(); \
			xstring mod = L"true"; \
			if (!pModel->GetIsModified()) mod = L"false"; \
			info = info + L"\n\t - GetIsModified: " + mod; \
			info = info + L"\n\t - Type: " + std::to_wstring((int)type).c_str(); \
			info = info + L"\n\t - FullName: " + pModel->GetFullName(); \
			info = info + L"\n\t - FileName: " + pModel->GetFileName(); \
			info = info + L"\n\t - CommonName: " + pModel->GetCommonName(); \
			info = info + L"\n\t - InstanceName: " + pModel->GetInstanceName(); \
			info = info + L"\n\t - VersionStamp: " + pModel->GetVersionStamp(); \
			info = info + L"\n\t - Origin: " + pModel->GetOrigin(); \
			xstring genericName = pModel->GetGenericName(); \
			if (genericName != xstringnil) \
				info = info + L"\n\t - GenericName: " + genericName; \
			xstring rev = pModel->GetRevision(); \
			if (rev != xstringnil) \
				info = info + L"\n\t - Revision: " + rev; \
			xstring ver = pModel->GetVersion(); \
			if (ver != xstringnil) \
				info = info + L"\n\t - Version: " + ver; \
			xstring level = pModel->GetReleaseLevel(); \
			if (level != xstringnil) \
				info = info + L"\n\t - ReleaseLevel: " + level; \
		} \
		OTKX_EXCEPTION_HANDLER(); \
	}

	#define OTKX_MODEL_DUMP(pModel) \
	{ \
		if (pModel != NULL ) \
		{ \
			xstring info; \
			OTKX_MODEL_TOSTR(pModel, info); \
			LOG_DEBUG(L"Model information: " << info ); \
		}\
	}

	#define OTKX_MODELCOPY_DUMP(pFrom, pTo) \
	{ \
		if (pFrom != NULL && pTo != NULL) \
		{ \
			xstring info; \
			OTKX_MODEL_TOSTR(pFrom, info); \
			info = xstring(L"Model information:") + xstring(L"\npFromModel: ") + info; \
			xstring infoTo; \
			OTKX_MODEL_TOSTR(pTo, infoTo); \
			infoTo = L"\npToModel: " + infoTo; \
			LOG_DEBUG(info << infoTo); \
		}\
	}

#define OTKX_MAKE_ID(name, ext, id) \
{ \
	std::wstring lCaseName(name); \
	std::wstring lCaseExt(ext); \
	RMXUtils::ToLower<wchar_t>(lCaseName); \
	RMXUtils::ToLower<wchar_t>(lCaseExt); \
	id = lCaseName + L"." + lCaseExt; \
}
	/*!
	* Get the model from the given descriptor
	*
	* \param pMdlDescr
	* \return
	*/
	pfcModel_ptr OtkX_GetModelFromDescr(pfcModelDescriptor_ptr pMdlDescr);

	/*!
	* Get the current model
	*
	* \return
	*/
	pfcModel_ptr OtkX_GetCurrentModel();

	/*!
	* Get the parameter value
	*
	* \return
	*/
	xstring OtkX_GetStringParamValue(pfcModel_ptr pMdl, xrstring paramName);

	/*!
	* Get the parameter value
	*
	* \return
	*/
	bool OtkX_GetBoolParamValue(pfcModel_ptr pMdl, xrstring paramName);

	/*!
	* Set the parameter value
	* if it does not exist, create new parameter
	* \return
	*/
	void OtkX_SetStringParamValue(pfcModel_ptr pMdl, const xstring& paramName, const xstring& paramValue);

	/*!
	* Set the parameter value
	* if it does not exist, create new parameter
	* \return
	*/
	void OtkX_SetBoolParamValue(pfcModel_ptr pMdl, const xstring& paramName, bool paramValue);

	/*!
	* Set the parameter value
	* if it does not exist, create new parameter
	* \return
	*/
	void OtkX_RemoveParam(pfcModel_ptr pMdl, const xstring& paramName);

	/*!
	* Get the model type by the given file extension
	*
	* \return
	*/
	pfcModelType OtkX_GetModelType(const wstring& fileExt);
	
	/*!
	* Determine if it's a Creo native file type by the given file extension
	*
	* \return
	*/
	bool OtkX_IsNativeModelType(const wstring& fileExt);

	/*!
	* Get file extension by the given image type. e.g.: JPGE -> jpg
	*
	* \return
	*/
	std::wstring OtkX_GetImageExtention(const xstring& imageType);

	/*!
	* Open the file into Creo session and display in the window
	*
	*/
	bool OtkX_OpenFile(xrstring fileFullPath);

	/*!
	* Display a message dialog with the given message and dialog type like error, info, warning.
	*
	*/
	void OtkX_ShowMessageDialog(xrstring msg, pfcMessageDialogType dlgType);

	/*!
	* Display a message dialog with ok, cancel buttons, and the given message and dialog type like error, info, warning.
	*
	*/
	pfcMessageButton OtkX_ShowMessageDialog2(xrstring msg, pfcMessageDialogType dlgType);

	/*!
	* Display a message dialog with yes, no, cancel buttons, and the given message and dialog type like error, info, warning.
	*
	*/
	pfcMessageButton OtkX_ShowMessageDialog3(xrstring msg, pfcMessageDialogType dlgType);

	/*!
	* Determine if the given file is a export file type, excluding nxl file 
	* 
	* \return
	*/
	bool OtkX_IsExportFileType(const wchar_t* szFilePath);

	/*!
	* Determine if the given file is supported file type, excluding nxl file
	*
	* \return
	*/
	bool OtkX_IsSupportedFileType(const wchar_t* szFilePath);

	/*!
	* Recursively look for the dependency model from the given parent assembly, whose file name is matched  
	*
	* \return
	*/
	pfcModel_ptr OtkX_FindDepModel(pfcModel_ptr pParentMdl, const wchar_t* searchFileName);

	/*!
	* Get the file extension for the given path
	* \return File extension without dot. In case of part.prt.2, ext is prt, not 2.
	*/
	std::wstring OtkX_GetFileExtension(const wchar_t* szFilePath);

	/*!
	* Get the current working directory in session
	* \return
	*/
	std::wstring OtkX_GetCurrentDirectory();

	/*!
	* Determine if Creo session is running in UI mode
	*
	* \return
	*/
	bool OtkX_RunInGraphicMode();

	/*!
	* Display message about the protected file
	*
	* \return
	*/
	void OtkX_DisplayMessageForNxlFileInfo(pfcModel_ptr pMdl);

	/*!
	* Get the model for the given file name(part.1.prt)
	*
	* \return
	*/
	pfcModel_ptr OtkX_GetModelFromFileName(const wstring& fileName);

	void OtkX_CloseModel(pfcModel_ptr pMdl);
	bool OtkX_EraseModel(pfcModel_ptr pMdl, bool includeDep = true);
	void OtkX_ReopenFile(const wstring& fileName);

	//*****************************************************************************
	class OtkXFileNameData
	{
	public:
		OtkXFileNameData();
		OtkXFileNameData(const std::wstring& fileName);
		OtkXFileNameData(const std::wstring& dir, const std::wstring& name, const std::wstring& ext, int version);

		std::wstring GetDirectoryPath() const { return m_directoryPath; }
		std::wstring GetBaseName() const { return m_name; } // File name without extension
		std::wstring GetExtension() const { return m_extension; } // File extension without leading period (.)
		int GetVersion() const { return m_version; }

		std::wstring GetFullPath() const { return m_fullPath; }
		std::wstring GetFileName() const { return m_name + L"." + m_extension; } // File name with extension

		bool IsSameVersionFile(const OtkXFileNameData& otherFNameData) const;

	private:
		std::wstring m_directoryPath;
		std::wstring m_name;
		std::wstring m_extension;
		int m_version;
		std::wstring m_fullPath;
	};
}; // namespace OtkXUtils