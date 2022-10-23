#pragma once

#if !defined(LIBRMX_STATIC)
#if defined(LIBRMX_EXPORTS)
	#define LIBRMXAPI __declspec(dllexport)
#else
	#define LIBRMXAPI __declspec(dllimport)
#endif
#else
	#define LIBRMXAPI
#endif 

#include "RMXTypes.h"

#if defined(__cplusplus)
extern "C" {
#endif
	LIBRMXAPI bool RMX_Initialize();
	LIBRMXAPI void RMX_Terminate();
	LIBRMXAPI bool RMX_ProtectFile(const wchar_t* filePath, const char* tags, const wchar_t* newFilePath);
	LIBRMXAPI bool RMX_IsProtectedFile(const wchar_t* nxlFilePath);
	LIBRMXAPI void RMX_GetTags(const wchar_t* nxlFilePath, char** ppTags);
	LIBRMXAPI bool RMX_IsRPMFolder(const wchar_t* dirPath);
	LIBRMXAPI bool RMX_GetSafeDirRelation(const wchar_t* dirPath, RMX_SAFEDIRRELATION& dirRelation);
	LIBRMXAPI bool RMX_AddRPMDir(const wchar_t* dirPath);
	LIBRMXAPI bool RMX_AddRPMDir2(const wchar_t* dirPath, unsigned int option);
	LIBRMXAPI bool RMX_RemoveRPMDir(const wchar_t* dirPath);
	LIBRMXAPI bool RMX_CopyNxlFile(const wchar_t* srcNxlFilePath, const wchar_t* destNxlFilePath, bool deleteSource);
	LIBRMXAPI bool RMX_RPMResisterApp(const wchar_t* appPath);
	LIBRMXAPI bool RMX_RPMUnresisterApp(const wchar_t* appPath);
	LIBRMXAPI bool RMX_RPMAddTrustedProcess(unsigned long processId);
	LIBRMXAPI bool RMX_CheckRights(const wchar_t* nxlFilePath, unsigned long rights, bool audit);
	LIBRMXAPI bool RMX_DeleteNxlFile(const wchar_t* nxlFilePath);
	LIBRMXAPI void RMX_GetRightName(unsigned long rights, wchar_t* name);
	LIBRMXAPI void RMX_ReleaseArray(void* pArr);
	LIBRMXAPI unsigned long RMX_GetLastError(wchar_t** ppErrMsg);
	LIBRMXAPI bool RMX_IsValidNxl(const wchar_t* nxlFilePath);
	LIBRMXAPI bool RMX_MergeTags(const char* tags, char** ppNewTags);
	LIBRMXAPI bool RMX_RPMSetViewOverlay(const RMX_VIEW_OVERLAY_PARAMS& params);
	LIBRMXAPI bool RMX_RPMClearViewOverlay(void* hTargetWnd);
	LIBRMXAPI bool RMX_GetRights(const wchar_t* nxlFilePath, unsigned long & rights);
	LIBRMXAPI bool RMX_RPMAddTrustedApp(const wchar_t* appPath);
	LIBRMXAPI bool RMX_RPMRemoveTrustedApp(const wchar_t* appPath);
	LIBRMXAPI bool RMX_ReProtectFile(const wchar_t* srcNxlFilePath);
	LIBRMXAPI bool RMX_RPMEditSaveFile(const wchar_t* srcFilePath, const wchar_t* origNxlFilePath, bool deleteSource, RMXEditSaveMode mode);
	LIBRMXAPI bool RMX_AddActivityLog(const wchar_t* nxlFilePath, RMXActivityLogOperation op, RMXActivityLogResult logResult);
	LIBRMXAPI bool RMX_EnsureNxlExtension(const wchar_t* filePath);
	LIBRMXAPI bool RMX_AddFileAttributes(const wchar_t* nxlFilePath, unsigned long attrs);
	LIBRMXAPI bool RMX_SetFileAttributes(const wchar_t* nxlFilePath, unsigned long attrs);
	LIBRMXAPI bool RMX_ShowFileInfoDialog(const wchar_t* nxlFilePath, const wchar_t* fileDisplyName, const wchar_t* dlgTitle);
	LIBRMXAPI bool RMX_ShowProtectDialog(const wchar_t* filePath, const wchar_t* buttonLabel, wchar_t** tags);
	LIBRMXAPI void RMX_NotifyMessage(const wchar_t* nxlFilePath, const wchar_t* msg);

#if defined(__cplusplus)
}
#endif