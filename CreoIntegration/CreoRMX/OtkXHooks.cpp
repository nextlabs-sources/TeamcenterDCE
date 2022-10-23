#include "OtkXHooks.h"

#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <winternl.h>
#include <mapi.h>
#include <VersionHelpers.h>

#include <madCHook.h>
#include "..\common\CommonTypes.h"
#include "..\common\CreoRMXLogger.h"
#include "OtkXUtils.h"
#include <madCHook.h>
#include <PathEx.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>

#include <LibRMX.h>
#include "FileClosedHooks.h"
#include "HookDefs.h"
#include "OtkXSessionCache.h"
#include "OtkXUIStrings.h"
#include "ProtectController.h"
#include "UsageController.h"

using namespace OtkXUtils;

#pragma region WINAPI_DEFS
/*
CreateFileA and CreateFileW
*/
typedef HANDLE(WINAPI *pfCreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

typedef HANDLE(WINAPI *pfCreateFileW)(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

//
// CreateProcess
//
typedef BOOL(WINAPI *pfCreateProcessA)(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);

typedef BOOL(WINAPI *pfCreateProcessW)(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);

//
// MoveFileExA and MoveFileExW
//
typedef
BOOL(WINAPI *pfMoveFileExA)(
	__in     LPCSTR lpExistingFileName,
	__in_opt LPCSTR lpNewFileName,
	__in     DWORD    dwFlags
	);
typedef
BOOL(WINAPI *pfMoveFileExW)(
	__in     LPCWSTR lpExistingFileName,
	__in_opt LPCWSTR lpNewFileName,
	__in     DWORD    dwFlags
	);

/*
DeleteFileA and DeleteFileW
*/
typedef BOOL(WINAPI *pfDeleteFileA)(
	LPCSTR lpFileName
	);
typedef BOOL(WINAPI *pfDeleteFileW)(
	LPCWSTR lpFileName
	);

/*
CopyFileA and CopyFileW
*/
typedef BOOL(WINAPI *pfCopyFileExA)(
	_In_        LPCSTR lpExistingFileName,
	_In_        LPCSTR lpNewFileName,
	_In_opt_    LPPROGRESS_ROUTINE lpProgressRoutine,
	_In_opt_    LPVOID lpData,
	_When_(pbCancel != NULL, _Pre_satisfies_(*pbCancel == FALSE))
	_Inout_opt_ LPBOOL pbCancel,
	_In_        DWORD dwCopyFlags
	);

typedef BOOL(WINAPI *pfCopyFileExW)(
	_In_        LPCWSTR lpExistingFileName,
	_In_        LPCWSTR lpNewFileName,
	_In_opt_    LPPROGRESS_ROUTINE lpProgressRoutine,
	_In_opt_    LPVOID lpData,
	_When_(pbCancel != NULL, _Pre_satisfies_(*pbCancel == FALSE))
	_Inout_opt_ LPBOOL pbCancel,
	_In_        DWORD dwCopyFlags
	);

/*
mapi32!MAPISendEmail
*/
typedef ULONG(WINAPI* pfMAPISendMail)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	_In_ lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);

typedef ULONG(WINAPI* pfMAPISendMailW)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	_In_ lpMapiMessageW lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);

typedef BOOL(WINAPI *pfShellExecuteExW)(
	SHELLEXECUTEINFOW *pExecInfo
	);

/*
GetFileAttributesW
*/
typedef DWORD(WINAPI* pfGetFileAttributesW)(_In_ LPCWSTR lpFileName);

#pragma endregion WINAPI_DEFS

DEFINE_HOOK_CALLBACK(CreateFileW)
static HANDLE CreateFileW_hooked(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	auto callNextFunc = [&]() -> HANDLE {
		return CreateFileW_next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	};

#ifndef NDEBUG
	//wstring currDir = OtkX_GetCurrentDirectory();
	//LOG_DEBUG_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d, currDir='%s'): '%s'", dwDesiredAccess, dwCreationDisposition, currDir.c_str(), lpFileName);
#endif // !NDEBUG

	if (CProtectController::GetInstance().IsInProcess())
	{
		/*LOG_DEBUG_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d, InHook='%d'): Nested call. Ignored. '%s'", 
			dwDesiredAccess, dwCreationDisposition, CreateFileWInHook, lpFileName);*/
		return callNextFunc();
	}

	// TO be investigated. 
	// When save as asm, the previous createfile is not completed, but next call is coming.
	// It causes some calls for parts not caught properly by RMX.
	//RMX_SCOPE_GUARD_ENTER(&CreateFileWInHook);

	if (dwCreationDisposition == OPEN_EXISTING)
	{
		if (dwDesiredAccess == GENERIC_READ)
		{
			//LOG_DEBUG_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d, ): '%s'", dwDesiredAccess, dwCreationDisposition, lpFileName);
			if (wcsstr(lpFileName, L"txd_Open.png") != NULL)
			{
				HANDLE hFile = callNextFunc();
				CUsageController::GetInstance().ApplyToIPEM();
				return hFile;
			}
			else if (wcsstr(lpFileName, L"tessPRO.config") != NULL)
			{
				HANDLE hFile = callNextFunc();
				CUsageController::GetInstance().ApplyToPLMJT();
				return hFile;
			}
			if (CProtectController::GetInstance().IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTBATCH))
			{
				HANDLE hFile = callNextFunc();	
				const wstring& fileExt = OtkX_GetFileExtension(lpFileName);
				if (wcsicmp(fileExt.c_str(), L"jt") == 0)
				{
					wstring filePath = RMXUtils::GetFilePathByHandle(hFile);
					if (!filePath.empty())
					{
						LOG_INFO_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d): Apply protection control to exported file '%s'", dwDesiredAccess, dwCreationDisposition, filePath.c_str());
						CProtectController::GetInstance().WatchFile(filePath);
					}			
				}
				//else if (wcsicmp(fileExt.c_str(), L"txt") == 0)
				//{
				//	// Cache command file of JT translator and grab source file path later
				//	wstring filePath = RMXUtils::GetFilePathByHandle(hFile);
				//	CProtectController::GetInstance().AddJTBatchCommandFile(filePath);
				//}
				return hFile;
			}

			// Below special handling to automatically append .nxl extension for managed mode only.
			// The case will happen when downloading file from TC but a local file exists, .nxl extension is lost.
			// e.g. Update in cache manager, or choose "Fetch Model From Teamcenter" option in open dialog.
			if (!CUsageController::GetInstance().IsIPEMCmdApplied()||
				!CPathEx::IsFile(lpFileName) || !OtkX_IsSupportedFileType(lpFileName))
				return callNextFunc();

			if (RMX_EnsureNxlExtension(lpFileName))
			{
				LOG_INFO_FMT(L"CreateFileW_hooked: Fixed missing .nxl extension: '%s'", lpFileName);
				CPathEx filePath(lpFileName);
				filePath += NXL_FILE_EXT;
				return CreateFileW_next(filePath.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
			}
		}
	}
	else if (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == OPEN_ALWAYS)
	{
		//LOG_DEBUG_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d): '%s'", dwDesiredAccess, dwCreationDisposition, lpFileName);
		if (HAS_BIT(dwDesiredAccess, GENERIC_WRITE) || HAS_BIT(dwDesiredAccess, GENERIC_WRITE | GENERIC_READ))
		{
			// During export, notify the protection control to track the exported file
			// Currently only monitor file creation for export and copy action
			if (CProtectController::GetInstance().IsWatcherRunning())
			{
				HANDLE hFile = callNextFunc();
				wstring filePath = RMXUtils::GetFilePathByHandle(hFile);
				if (!filePath.empty() && OtkX_IsSupportedFileType(filePath.c_str()))
				{
					LOG_INFO_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d): Apply protection control to exported file '%s'", dwDesiredAccess, dwCreationDisposition, filePath.c_str());
					CProtectController::GetInstance().WatchFile(filePath);
				}

				return hFile;
			}
			// Since RMX 5.2: below workaround to save nxl file is no longer needed.
			/*if (CPathEx::IsFile(lpFileName) && OtkX_IsSupportedFileType(lpFileName))
			{
				CPathEx filePath(lpFileName);
				filePath += NXL_FILE_EXT;
				if (CPathEx::IsFile(filePath.c_str()) &&
					RMX_IsRPMFolder(filePath.GetParentPath().c_str()) && RMX_IsProtectedFile(filePath.c_str()))
				{
					LOG_INFO_FMT(L"CreateFileW_hooked(desiredAccess=0x%08x, disposition=%d): Redirect writing to nxl file '%s'", dwDesiredAccess, dwCreationDisposition, filePath.c_str());
					return CreateFileW_next(filePath.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
				}
			}*/
		}	
	}
	
	return callNextFunc();
}

std::multimap<std::wstring, std::wstring> g_denyZipFiles;
std::multimap<std::wstring, std::wstring> g_denyEditFilesInZip;
void ZipOnBeforeCreateProcess(LPWSTR lpCommandLine)
{
	LOG_INFO_FMT(L"CreateProcessW_hooked('%s'): Trigger protection control before zipping files", lpCommandLine);
	g_denyZipFiles.clear();
	g_denyEditFilesInZip.clear();

	bool isExporting = CProtectController::GetInstance().IsWatchingExport();
	if (isExporting)
	{
		CProtectController::GetInstance().ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
	}
	if (!isExporting || CProtectController::GetInstance().IsWatcherRunning(PROTECT_WATCHER_SENDMAIL))
	{
		LPWSTR *szArglist = nullptr;
		int nArgs = 0;
		szArglist = CommandLineToArgvW(lpCommandLine, &nArgs);
		if (szArglist != nullptr)
		{
			for (int i = 0; i < nArgs; i++)
			{
				LPWSTR szArg = szArglist[i];
				if (wcsstr(szArg, L".zip") == NULL)
					continue;

				auto callProcessFile = [&](const wstring& dir, pfcSession_ptr pSess) -> void
				{
					WIN32_FIND_DATA fData;
					HANDLE hFind;
					const wstring searchDir(dir + L"\\*");
					if ((hFind = FindFirstFile(searchDir.c_str(), &fData)) != INVALID_HANDLE_VALUE) {
						do {
							if (wcscmp(fData.cFileName, L".") == 0 || wcscmp(fData.cFileName, L"..") == 0)
								continue;
 
							CPathEx filePath(dir.c_str());
							filePath /= fData.cFileName;

							if ((fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
							{
								// Usage control is triggered here.
								// Send to mail as attachment from model tree or folder content browser ,  it's not able to 
								// determine the source before command is triggered and do usage control as normal.
								// So postpone to check if file has copy right before zip and record deny result. 
								// SendMail will be denied via MAPISendMail_hooked.
								CPathEx nxlFilePath(filePath.c_str());
								wstring filePathWithoutNxl = filePath.ToString();
								if (wcsicmp(filePath.GetExtension().c_str(), NXL_FILE_EXT) != 0)
								{
									nxlFilePath += NXL_FILE_EXT;
								}
								else
								{
									filePathWithoutNxl = filePathWithoutNxl.substr(0, wcslen(filePathWithoutNxl.c_str()) - 4);
								}
								if (RMX_IsProtectedFile(nxlFilePath.c_str()))
								{
									LOG_DEBUG_FMT(L"CreateProcessW_hooked(%s): USAGECONTROL triggerd", nxlFilePath.c_str());
									OtkXFileNameData nameData(filePathWithoutNxl);
									if (OtkX_IsNativeModelType(nameData.GetExtension())) {
										LOG_DEBUG(L"CreateProcessW_hooked: Searching model in session containing file name..." << filePath.c_str());
										if (pSess) {
											auto pMdl = pSess->GetModelFromFileName(RMXUtils::ws2s(nameData.GetFileName()).c_str());
											bool checkEdit = false;
											if (pMdl && pMdl->GetIsModified())
												checkEdit = true;
											COtkXModel xMdl(pMdl);
											auto usageRight = CUsageController::CheckRight(xMdl, RMX_RIGHT_EXPORT, checkEdit);
											if (usageRight == Usage_Right_Deny)
												g_denyZipFiles.insert(std::make_pair(szArg, nameData.GetFileName()));
											else if (usageRight == Usage_Right_DenyEdit)
												g_denyEditFilesInZip.insert(std::make_pair(szArg, nameData.GetFileName()));
										}
									}
								}
								else
								{
									try
									{
										if (pSess)
										{
											// Try to protect zipping file. 
											// In Session folder browser, send mail as zipped attachment will not trigger any other hooks. 
											// Here is entry point to walk thru all zipping file and try to check its source file in session and protect accordingly
											OtkXFileNameData nameData(filePath.ToString());
											if (OtkX_IsNativeModelType(nameData.GetExtension())) {
												LOG_DEBUG(L"CreateProcessW_hooked: Searching model in session containing file name..." << filePath.c_str());
												auto pMdl = pSess->GetModelFromFileName(RMXUtils::ws2s(nameData.GetFileName()).c_str());
												COtkXModel xMdl(pMdl);
												if (xMdl.IsValid())
												{
													LOG_DEBUG_FMT(L"CreateProcessW_hooked(sourceFile: %s, zipping file: %s): ProtectControl triggered to process zipping file...",
														xMdl.GetOrigin().c_str(), filePath.c_str());
													if (CProtectController::GetInstance().ProcessCopyFile(xMdl.GetOrigin(), filePath.ToString()))
													{
														LOG_DEBUG_FMT(L"CreateProcessW_hooked(%s): USAGECONTROL triggerd to check permission", filePath.c_str());
														bool checkEdit = false;
														if (pMdl && pMdl->GetIsModified())
															checkEdit = true;
														auto usageRight = CUsageController::CheckRight(xMdl, RMX_RIGHT_EXPORT, checkEdit);
														if (usageRight == Usage_Right_Deny)
															g_denyZipFiles.insert(std::make_pair(szArg, nameData.GetFileName()));
														else if (usageRight == Usage_Right_DenyEdit)
															g_denyEditFilesInZip.insert(std::make_pair(szArg, nameData.GetFileName()));
													}
												}
											}
										}
									}

									OTKX_EXCEPTION_HANDLER();
								}
							}
						} while (FindNextFile(hFind, &fData) != 0);

						FindClose(hFind);
					}
				};

				CPathEx filePath(szArg);
				if (!filePath.GetParentPath().empty())
				{
					auto pSess = pfcGetProESession();
					callProcessFile(filePath.GetParentPath(), pSess);
				}
				break;

			}
			LocalFree(szArglist);
		}
	}
}

bool OnBeforeCreateProcess(LPWSTR lpCommandLine)
{
	//if (!CProtectController::GetInstance().IsWatchingExport())
	//	return false;

	// Case1: The exported file will be deleted after zip file is generated. It's too late if triggering protection after
	// the "export" command finishes.
	// So RMX has to trigger protection job before the exported files are compressed as zip file. 
	//
	bool isZip = false;
	bool handled = false;
	if ((isZip = (wcsstr(lpCommandLine, L"zip.exe") != NULL)))
	{
		ZipOnBeforeCreateProcess(lpCommandLine);
		handled = true;
	}
	//
	// Case2: Protect the pdf before opening it in Acrobat Reader. Result: Reader cannot open pdf.nxl w/o PDF RMX.
	// TODO: Provide better message to prompt the protected pdf cannot open in Reader.
	//
	// Since 5.4: Handled by ShellExecuteExW hook
	//else if (wcsstr(lpCommandLine, L".pdf") != NULL)
	//{
	//	LPWSTR *szArglist = nullptr;
	//	int nArgs = 0;
	//	szArglist = CommandLineToArgvW(lpCommandLine, &nArgs);
	//	if (szArglist != nullptr)
	//	{
	//		for (int i = 0; i < nArgs; i++)
	//		{
	//			LPWSTR szArg = szArglist[i];
	//			if (wcsstr(szArg, L".pdf") != NULL)
	//			{
	//				CPathEx filePath(szArg);
	//				LOG_INFO_FMT(L"CreateProcessW_hooked('%s'): Apply protection control to '%s'", lpCommandLine, filePath.c_str());
	//				if (filePath.GetParentPath().empty())
	//				{
	//					// In case the command line does not contain full path of file, 
	//					// assume the file locates in current directory.
	//					wstring CurrDir = OtkX_GetCurrentDirectory();
	//					filePath = CurrDir.c_str();
	//					filePath /= szArg;
	//				}
	//
	//				CProtectController::GetInstance().WatchFile(filePath.ToString());
	//				
	//				LOG_INFO_FMT(L"CreateProcessW_hooked('%s'): Trigger protection control before opennning pdf in the viewer", lpCommandLine);
	//				CProtectController::GetInstance().ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
	//				break;
	//			}			
	//		}
	//		LocalFree(szArglist);
	//	}

	//	handled = true;
	//}
	return handled;
}

bool OnAfterCreateProcess(LPWSTR lpCommandLine)
{
	// In batch mode, register ipemrmx failed due to the commands loading sequence.
	// This is 3rd place to try to register ipemrmx
	if (!OtkXUtils::OtkX_RunInGraphicMode() && (
		wcsstr(lpCommandLine, L"ipemrunner_sync.bat") != NULL))
	{
		CUsageController::GetInstance().ApplyToIPEM();
	}
	if (!CProtectController::GetInstance().IsWatchingExport())
		return false;

	// Some export types are generated by external process, CreateFile API cannot hook and watch the 
	// newly exported file. 
	// Explicitly track the process and protect the exported file.
	static std::pair<wstring, wstring> s_specialWatchExportTypes[] = {
		std::make_pair(wstring(L"pro_medconv.exe"), wstring(L".she")),
		std::make_pair(wstring(L"pro_dwgconv.exe"), wstring(L".dwg")),
		std::make_pair(wstring(L"pro_sthenoconv.exe"), wstring(L".tsh"))
	};

	for (const auto& exportType : s_specialWatchExportTypes)
	{
		if (wcsstr(lpCommandLine, exportType.first.c_str()) == NULL ||
			wcsstr(lpCommandLine, exportType.second.c_str()) == NULL)
		{
			continue;
		}
		LPWSTR *szArglist = nullptr;
		int nArgs = 0;
		szArglist = CommandLineToArgvW(lpCommandLine, &nArgs);
		if (szArglist != nullptr)
		{
			for (int i = 0; i < nArgs; i++)
			{
				LPWSTR szArg = szArglist[i];
				if (wcsstr(szArg, exportType.second.c_str()) == NULL)
					continue;

				CPathEx filePath(szArg);
				if (filePath.GetParentPath().empty())
				{
					// In case the command line does not contain full path of file, 
					// assume the file locates in current directory.
					wstring CurrDir = OtkX_GetCurrentDirectory();
					filePath = CurrDir.c_str();
					filePath /= szArg;
					CProtectController::GetInstance().WatchFile(filePath.c_str());
				}
				else
				{
					CProtectController::GetInstance().WatchFile(filePath.ToString());
				}
				LOG_INFO_FMT(L"CreateProcessW_hooked('%s'): Apply protection control to '%s'", lpCommandLine, filePath.c_str());
				// Some export action(drawing->dwg) can export multiple-times 
				// in single command request. Immediately protect the file to avoid encrypt content gets exposed in between.
				//CProtectController::GetInstance().ProcessWatchedFiles(PROTECT_FILEWATCHER_EXPORT);			
				break;
			}
			LocalFree(szArglist);
		}
		
		return true;
	}

	return false;
}

DEFINE_HOOK_CALLBACK(CreateProcessW)
BOOL CreateProcessW_hooked(LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
#ifndef NDEBUG
	TCHAR szCurrDir[MAX_PATH];
	DWORD dwRet = GetCurrentDirectory(MAX_PATH, szCurrDir);
	LOG_DEBUG_FMT(L"Before CreateProcessW_hooked(currDir:'%s'): %s", szCurrDir, lpCommandLine);
#endif // !NDEBUG

	if (CreateProcessWInHook || CProtectController::GetInstance().IsInProcess())
	{
		return  CreateProcessW_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
			lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	RMX_SCOPE_GUARD_ENTER(&CreateProcessWInHook);
	bool handled = OnBeforeCreateProcess(lpCommandLine);
	
	bool succeed = CreateProcessW_next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
								lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		
	if (!handled)
	{
		OnAfterCreateProcess(lpCommandLine);
	}
#ifndef NDEBUG
	LOG_DEBUG_FMT(L"After CreateProcessW_hooked: %s", lpCommandLine);
#endif // !NDEBUG
	return succeed;
}

DEFINE_HOOK_CALLBACK(MoveFileExW)
BOOL MoveFileExW_hooked(
	__in     LPCWSTR lpExistingFileName,
	__in_opt LPCWSTR lpNewFileName,
	__in     DWORD    dwFlags)
{
	if (MoveFileExWInHook ||
		CProtectController::GetInstance().IsInProcess() || !CProtectController::GetInstance().IsWatchingExport())
		return MoveFileExW_next(lpExistingFileName, lpNewFileName, dwFlags);

	RMX_SCOPE_GUARD_ENTER(&MoveFileExWInHook);

	BOOL ret = MoveFileExW_next(lpExistingFileName, lpNewFileName, dwFlags);

	static std::set<std::wstring, ICaseKeyLess> s_fileTypesToProcess =
	{
		L".stl", L".iv", L".slp", L".obj", L".gbf", L".asc", L".facet", L".amf"
	};

	CPathEx newPathEx(lpNewFileName);
	if (s_fileTypesToProcess.find(newPathEx.GetExtension()) != s_fileTypesToProcess.end())
	{
		LOG_DEBUG_FMT(L"MoveFileExW_hooked(existingFileName: %s, newFileName: %s)", lpExistingFileName, lpNewFileName);
		const wstring& currDir = RMXUtils::GetCurrentDir();
		if (newPathEx.GetParentPath().empty())
		{
			newPathEx = currDir.c_str();
			newPathEx /= lpNewFileName;
		}
		ProtectControllerHolder().ProcessFile(newPathEx.ToString(), PROTECT_WATCHER_EXPORT);
		if (wcsicmp(newPathEx.GetExtension().c_str(), L".obj") == 0)
		{
			// Protect mtl file at same time
			wstring mtlFilePath = newPathEx.ToString();
			mtlFilePath.replace(mtlFilePath.length() - 3, 3, L"mtl");
			ProtectControllerHolder().ProcessFile(mtlFilePath, PROTECT_WATCHER_EXPORT);
		}
		newPathEx = lpExistingFileName;
		if (newPathEx.GetParentPath().empty())
		{
			newPathEx = currDir.c_str();;
			newPathEx /= lpExistingFileName;
			ProtectControllerHolder().RemoveWatchedExportFile(newPathEx.ToString());
		}
	}

	return ret;
}

DEFINE_HOOK_CALLBACK(DeleteFileW)
BOOL DeleteFileW_hooked(
	LPCWSTR lpFileName)
{
	if (DeleteFileWInHook ||
		CProtectController::GetInstance().IsInProcess() || !CProtectController::GetInstance().IsWatchingExport())
		return DeleteFileW_next(lpFileName);

	RMX_SCOPE_GUARD_ENTER(&DeleteFileWInHook);

	BOOL ret = DeleteFileW_next(lpFileName);
	CPathEx pathEx(lpFileName);
	if ((wcsicmp(pathEx.GetExtension().c_str(), L".dxf") == 0) &&
		(wcsicmp(pathEx.GetFileName().c_str(), L"temp_dwg") == 0))
	{
		auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
		if (pCurrMdl)
		{
			COtkXFile srcMainFile(CXS2W(pCurrMdl->GetOrigin()));
			if (srcMainFile.IsDrw())
			{
				LOG_DEBUG_FMT(L"Drw2DwgHandler: %s", pathEx.c_str());
				ProtectControllerHolder().ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
			}
		}
	}
	
	return ret;
}

//int _JtkCADExporter::exportJt(class JtkHierarchy *)
typedef void* JtkHierarchy_ptr;
typedef void* JtkCADExporter_ptr;
#define JtkCADExporter_exportJt_FULLNAME "?exportJt@_JtkCADExporter@@UEAAHPEAVJtkHierarchy@@@Z"
#define JtkCADExporter_exportJt_ORDINAL 2509
typedef int(*pfJtkCADExporter_exportJt)(JtkCADExporter_ptr, JtkHierarchy_ptr);
static pfJtkCADExporter_exportJt JtkCADExporter_exportJt_original;
static pfJtkCADExporter_exportJt JtkCADExporter_exportJt_next;
static bool g_exportJtInHook = false; // Flag to prevent nested hook
static int JtkCADExporter_exportJt_hooked(JtkCADExporter_ptr exporter, JtkHierarchy_ptr hier)
{
	if (g_exportJtInHook)
		return JtkCADExporter_exportJt_next(exporter, hier);

	// Since JT Translator 15.0, translation will be executed asynchronously. Cannot determine if the operation is completed
	// based on the command bracket listener. 
	RMX_SCOPE_GUARD_ENTER(&g_exportJtInHook);
	bool startWatcher = false;
	DWORD watcherTypeForJTCurrent = PROTECT_WATCHER_EXPORT_JTCURRENT | PROTECT_WATCHER_EXPORT;
	LOG_DEBUG(L"JtkCADExporter_exportJt_hooked");
	if (!CProtectController::GetInstance().IsWatchingJTExport())
	{
		CProtectController::GetInstance().StartFileWatcher(watcherTypeForJTCurrent);
		startWatcher = true;
	}

	int ret = JtkCADExporter_exportJt_next(exporter, hier);
	if (startWatcher || CProtectController::GetInstance().IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTCURRENT))
	{
		CProtectController::GetInstance().ProcessWatchedFiles(watcherTypeForJTCurrent);
	}
	if (startWatcher)
	{
		CProtectController::GetInstance().StopFileWatcher(watcherTypeForJTCurrent);
	}
	return ret;
}

DEFINE_HOOK_CALLBACK(MAPISendMail)
ULONG MAPISendMail_hooked(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
)
{
	LOG_DEBUG_FMT(L"MAPISendMail_hooked(%d)", lpMessage->nFileCount);
	if (g_denyZipFiles.empty() && g_denyEditFilesInZip.empty())
		return MAPISendMail_next(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

	for (ULONG i = 0; i < lpMessage->nFileCount; i++)
	{
		auto szFilePath = lpMessage->lpFiles[i].lpszPathName;
		const wstring& mapiFilePath = RMXUtils::s2ws(const_cast<char*>(szFilePath));
		if (g_denyZipFiles.count(mapiFilePath) > 0)
		{
			auto range = g_denyZipFiles.equal_range(mapiFilePath);
			xstring msg(IDS_ERR_DENY_MAILATTACH);
			xstring fileInfo;
			for (auto elem = range.first; elem != range.second; ++elem)
			{
				// Populate error message for source file
				// And delete the exported files generated based on this source file.
				fileInfo = xstring::Printf("\n- %s", RMXUtils::ws2s(elem->second).c_str());
				msg += fileInfo;
			}
			LOG_ERROR_FMT(L"MAPISendMail_hooked: %s", RMXUtils::s2ws(msg));
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
			g_denyZipFiles.erase(mapiFilePath);
			return MAPI_E_FAILURE;
		}
		if (g_denyEditFilesInZip.count(mapiFilePath) > 0)
		{
			auto range = g_denyEditFilesInZip.equal_range(mapiFilePath);
			xstring msg(IDS_ERR_DENY_EDIT_MAILATTACH);
			xstring fileInfo;
			for (auto elem = range.first; elem != range.second; ++elem)
			{
				// Populate error message for source file
				// And delete the exported files generated based on this source file.
				fileInfo = xstring::Printf("\n- %s", RMXUtils::ws2s(elem->second).c_str());
				msg += fileInfo;
			}
			LOG_ERROR_FMT(L"MAPISendMail_hooked: %s", RMXUtils::s2ws(msg));
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
			g_denyEditFilesInZip.erase(mapiFilePath);
			return MAPI_E_FAILURE;
		}
	}

	return MAPISendMail_next(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

DEFINE_HOOK_CALLBACK(CopyFileExW)
BOOL CopyFileExW_hooked(
	LPCWSTR            lpExistingFileName,
	LPCWSTR            lpNewFileName,
	LPPROGRESS_ROUTINE lpProgressRoutine,
	LPVOID             lpData,
	LPBOOL             pbCancel,
	DWORD              dwCopyFlags
)
{
	//LOG_DEBUG_FMT(L"CopyFileExW_hooked(lpExistingFileName:%s, lpNewFileName:%s)", lpExistingFileName, lpNewFileName);
	BOOL ret = CopyFileExW_next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	if (ret)
	{
		// Copy nxl file from rpm folder via trusted process will cause protection lost on target file. 
		// RMX has to reapply protection on the target file
		CPathEx existingFilePath(lpExistingFileName);
		if (OtkX_IsSupportedFileType(lpExistingFileName) &&
			RMX_IsRPMFolder(existingFilePath.GetParentPath().c_str()))
		{
			LOG_INFO_FMT(L"CopyFileExW_hooked(lpExistingFileName:%s, lpNewFileName:%s): ProtectControl triggered to process copy file...", lpExistingFileName, lpNewFileName);
			CProtectController::GetInstance().ProcessCopyFile(lpExistingFileName, lpNewFileName);
		}
	}
	
	return ret;
}


DEFINE_HOOK_CALLBACK(ShellExecuteExW)
static BOOL ShellExecuteExW_hooked(SHELLEXECUTEINFOW *pExecInfo)
{
	if (CProtectController::GetInstance().IsInProcess() || !CProtectController::GetInstance().IsWatchingExport())
		return ShellExecuteExW_next(pExecInfo);

	if (pExecInfo->lpFile != nullptr) {
		CPathEx filePath(pExecInfo->lpFile);
		if (_wcsicmp(filePath.GetExtension().c_str(), L".pdf") == 0) {
			LOG_INFO_FMT(L"ShellExecuteExW_hooked('%s'): Apply protection control to '%s'", pExecInfo->lpFile);
			CProtectController::GetInstance().WatchFile(filePath.ToString());
			LOG_INFO_FMT(L"ShellExecuteExW_hooked('%s'): Trigger protection control before opennning pdf in the viewer", pExecInfo->lpFile);
			CProtectController::GetInstance().ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
		}
	}
	return ShellExecuteExW_next(pExecInfo);
}

DEFINE_HOOK_CALLBACK(GetFileAttributesW)
DWORD WINAPI  GetFileAttributesW_hooked(_In_ LPCWSTR lpFileName)
{
	if (GetFileAttributesWInHook ||
		CProtectController::GetInstance().IsInProcess() || !CProtectController::GetInstance().IsWatchingExport())
		return GetFileAttributesW_next(lpFileName);

	RMX_SCOPE_GUARD_ENTER(&GetFileAttributesWInHook);
	if (wcsstr(lpFileName, L".3mf") != NULL)
	{
		LOG_INFO_FMT(L"GetFileAttributesW_hooked: Apply protection control to exported file '%s'", lpFileName);
		CProtectController::GetInstance().WatchFile(lpFileName);
	}
	return GetFileAttributesW_next(lpFileName);

}

void HookInitializer::Startup()
{
	InitializeMadCHook();
	LOG_INFO(L"MadCHook initialized");
	SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);

	HOOK_API("KernelBase.dll", CreateFileW);
	HOOK_API("kernel32.dll", CreateProcessW);
	HOOK_API("MAPI32", MAPISendMail);
	if (IsWindows8OrGreater())
	{
		// Hook from kernel32 not working on win10
		HOOK_API("KernelBase.dll", CopyFileExW);
	}
	else
	{
		HOOK_API("kernel32.dll", CopyFileExW);
	}

	if (OtkX_RunInGraphicMode())
	{
#if defined(CREO_VER) && CREO_VER == 7
		HOOK_API("JtTk107.dll", JtkCADExporter_exportJt);
#else
		if (GetModuleHandle(L"JtTk103.dll") != NULL)
		{
			HOOK_API("JtTk103.dll", JtkCADExporter_exportJt);
		}
		else
		{
			HOOK_API("JtTk107.dll", JtkCADExporter_exportJt);
		}
#endif
	}

}

void HookInitializer::Shutdown()
{
	UNHOOK(JtkCADExporter_exportJt);
	UNHOOK(CreateFileW);
	UNHOOK(CreateProcessW);
	UNHOOK(MAPISendMail);
	UNHOOK(CopyFileExW);
	FinalizeMadCHook();
	LOG_INFO(L"MadCHook Finalized");
}

void HookInitializer::InstallHook_Export()
{
	FileClosedHooks::Install_Export();

	HOOK_API("Kernel32.dll", MoveFileExW);
	HOOK_API("kernelbase.dll", DeleteFileW);
	HOOK_API("Shell32.dll", ShellExecuteExW);
#if defined(CREO_VER) && CREO_VER > 4
	HOOK_API("kernelbase.dll", GetFileAttributesW);
#endif
}

void HookInitializer::UnstallHook_Export()
{
	FileClosedHooks::Uninstall_Export();
	UNHOOK(MoveFileExW);
	UNHOOK(DeleteFileW);
	UNHOOK(ShellExecuteExW);
#if defined(CREO_VER) && CREO_VER > 4
	UNHOOK(GetFileAttributesW);
#endif
}


