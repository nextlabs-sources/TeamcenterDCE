#pragma once
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RPMUTILS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RPMUTILS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#include <Windows.h>
#include <key_value_list.h>
#include <utils_string_list.h>

#ifdef RPMUTILS_EXPORTS
#define RPMUTILS_API __declspec(dllexport)
#else
#define RPMUTILS_API __declspec(dllimport)
#endif

//TODO:
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif
//#define ERROR_FILE_NOT_FOUND 0xAA0010
#define ERROR_FILE_IS_PROTECTED 0xAA0011
#define ERROR_FILE_NOT_PROTECTED 0xAA0012


class RPMUTILS_API ResultInfo
{
public:
	ResultInfo(DWORD code = 0, const char *msg = "", const char *func = "", const char *file = "", int line = 0);
	ResultInfo(const ResultInfo &inner, DWORD code, const char *msg, const char *func = "", const char *file = "", int line = 0);
	ResultInfo(const ResultInfo &err);
	ResultInfo & operator=(const ResultInfo &err);
	virtual ~ResultInfo();
	inline DWORD Code() const { return _code; }
	inline const char *Message() const { return _msg == nullptr ? "" : _msg; }
	inline const char *file() const { return _file == nullptr ? "" : _file; }
	inline const char *func() const { return _func == nullptr ? "" : _func; }
	inline int line() const { return _line; }
	inline const ResultInfo *InnerError() const { return _innerError; }
	inline bool operator == (int code) const { return (code == _code); }
	inline bool operator != (int code) const { return (code != _code); }
	void clear();
private:
	DWORD _code;
	char *_msg;
	char *_func;
	char *_file;
	int _line;
	ResultInfo *_innerError;
	void copy_from(const ResultInfo &err);
};



class NxlMetadataCollectionImpl;
class RPMUTILS_API NxlMetadataCollection
{
public:
	NxlMetadataCollection();
	NxlMetadataCollection(const NxlMetadataCollection &tags);
	NxlMetadataCollection(const char *str);
	NxlMetadataCollection &operator=(const NxlMetadataCollection &tags);
	inline bool operator==(const NxlMetadataCollection& tags) const {
		return strcmp(tags.str(), str()) == 0;
	}
	inline bool operator!=(const NxlMetadataCollection& tags) const {
		return strcmp(tags.str(), str()) != 0;
	}
	virtual ~NxlMetadataCollection();
	size_t len() const;
	const char *str() const;
	const char *join(const NxlMetadataCollection &tobemerged);
private:
	NxlMetadataCollectionImpl *_impl;
};

RPMUTILS_API bool ShowFileInfoDialog(const wchar_t* filePath, const wchar_t* displayName = L"");
RPMUTILS_API bool ShowProtectDialog(const wchar_t* filePath, NxlMetadataCollection &oTags, const wchar_t* actionName = L"");

class RPMSessionImpl;
// This class is exported from the RPMUtils.dll
class RPMUTILS_API RPMSession {
public:
	enum RPMFolderStatus { IsRPMDir, AncestorOfRPMDir, PosterityOfRPMDir, NonRPMDir, UnknownStatus	};
	static inline bool FolderStatus_IsRpm(RPMFolderStatus status) {
		return status == RPMSession::IsRPMDir;
	}
	static inline bool FolderStatus_IsInRpm(RPMFolderStatus status) {
		return status == RPMSession::IsRPMDir || status == RPMSession::PosterityOfRPMDir;
	}
	RPMSession(const char *passcode);
	~RPMSession();
	const ResultInfo &GetLastError();

	bool IsRPMLoggedIn();
	/*
		Prerequisite:		caller is trusted
	*/
	const ResultInfo &SetTrustedProcess(DWORD pid, bool isAdd);
	/*
		Prerequisite:		Process is signed
		current application will be automatically registered
	*/
	const ResultInfo &SetRMXStatus(bool status);

	const ResultInfo &RegisterApp(bool isRegister, const wchar_t *app = nullptr);
	const ResultInfo &IsAppRegistered(const wchar_t *app, bool *oRegistered);

	const ResultInfo &AddRPMFolder(const wchar_t *folder, bool *newFolder = nullptr, bool refresh=false);
	const ResultInfo &RemoveRPMFolder(const wchar_t *folder, bool bForce = true);
	const ResultInfo &CheckFolderStatus(const wchar_t *folder, RPMFolderStatus *status);
	const ResultInfo &IsFileInRPMDir(const wchar_t *file, bool *isinrpm);
	const ResultInfo &RPMEditCopy(const wchar_t *file, wchar_t rpmDir[MAX_PATH]);
	enum SaveMode {FinishWithoutSave, SaveOnly, SaveAndFinish};
	const ResultInfo &RPMEditSave(SaveMode mode, const wchar_t *rpmfile, const wchar_t *nxlfile = L"");
	const ResultInfo &RPMDeleteFile(const wchar_t *nxlfile);

	///when forceNxlExt=true, the .nxl extension will always be appended to the protected file
	//when forceNxlExt=false, the .nxl extension will be appened to the protected file when it's in PPM folder
	const ResultInfo &ProtectFile(const wchar_t *file, const NxlMetadataCollection &tags, bool forceNxlExt = false);
	const ResultInfo &CheckFileProtection(const wchar_t *file, bool *isProtected);
	const ResultInfo &CheckFileStatus(const wchar_t *file, bool *isProtected, RPMFolderStatus *folderStatus);

	/* get file metadata
	if file is in RPM folder, use Nxlhelper.exe to read the tags
	otherwise use RPM api to read the tags*/
	const ResultInfo &ReadFileTags(const wchar_t *file, bool *isProtected, NxlMetadataCollection &oTags);
	const ResultInfo &RPMReadFileTags(const wchar_t *file, bool *isProtected, NxlMetadataCollection &oTags);
	//TODO:WORKAROND for RPM bug
	const ResultInfo &HelperReadFileTags(const wchar_t *file, bool *isProtected, NxlMetadataCollection &oTags);
	//TODO:WORKAROND for RPM bug
	const ResultInfo &HelperCheckFileProtection(const wchar_t *file, bool *isProtected);

	const ResultInfo &ValidateFileExtension(const wchar_t *file, bool *oIsProtected=nullptr, bool *osInRpmDir=nullptr);

	bool IsNxlProtectionEnabled();

	/*
		SecureOverlay
	*/
	const ResultInfo &UpdateSecureOverlay(
		kvl_ro rmxAttrs,
		//additional information about the caller rmx, e.g. rmx_name=nxrmx;app_name=ugraf.exe 
		//these values will be passed as resource attributes, which can be used when defining a resource object in control center
		string_list_ro contextFiles,
		//the files of the assembly files which are displayed in current window
		HWND targertWnd
		//the window handle which will be passed to RPM SDK
	);

	/*
		Screen Capture
	*/
	const ResultInfo& SetScreenCapture(HWND targetWnd, bool enable);
	const ResultInfo& UpdateScreenCapture(HWND targetWnd, string_list_ro contextFiles);

	//for debug purpose or test only
	const ResultInfo &Logout();
	const ResultInfo &ListPolicies();
	/* 
		Prerequisite: caller is trust	
	*/
	const ResultInfo &CheckFileRights(const wchar_t *file, const char *right, bool &oIsAllowed, bool addLog=false);
	const ResultInfo &GetFileRights(const wchar_t *file, bool &oIsProtected, string_list_p oRights);
	const ResultInfo &AddActivityLog(const wchar_t *file, const char *right, bool isAllowed);
	void NotifyUser(const wchar_t *message, const wchar_t *context, const wchar_t *operation);
	void ListRPMInfo();
private:
	RPMSessionImpl *_impl;
};
