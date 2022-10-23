#include "TestFwkHlp.h"

#include <fstream>      // std::ifstream
#include <iterator>

#include <PathEx.h>
#include "XTestLogger.h"

#include <OtkXModel.h>
#include <OtkXUtils.h>
#include <RMXUtils.h>

#include <pfcGlobal.h>
#include <pfcComponentFeat.h>

#include <LibRMX.h>

using namespace std;
using namespace OtkXUtils;

#if !defined(__STRINGIZE)
#define __STRINGIZE(X) #X
#endif
#if !defined(_STRINGIZE)
#define _STRINGIZE(X) __STRINGIZE(X)
#endif

#if !defined(_W)
#define __WTEXT(x) L ## x
#define _W(x) __WTEXT(x)
#endif

namespace TestFwkHlp
{
	void _CloseAllModels()
	{
		try
		{
			auto pSess = pfcGetProESession();
			if (pSess)
			{
				auto pMdls = pSess->ListModels();
				for (xint i = 0; i < pMdls->getarraysize(); ++i)
				{
					auto pMdl = pMdls->get(i);
					if (pMdl)
						pMdl->EraseWithDependencies();
				}

				pSess->EraseUndisplayedModels();
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}

	void GetDepNxlFiles(pfcModel_ptr pParentMdl, set<wstring>& nxlFiles)
	{
		COtkXModel xMdl(pParentMdl);
		xMdl.TraverseDependencies([&](COtkXModel& xDepMdl) {
			if (xDepMdl.IsProtected())
				nxlFiles.insert(xDepMdl.GetOrigin());

			return false;
		});
	}

	void GetDepNxlFiles(const std::wstring & filePath, std::set<std::wstring>& nxlFiles)
	{
		_CloseAllModels();
		try
		{
			auto pSess = pfcGetProESession();
			if (pSess)
			{
				CPathEx filePathToOpen(filePath.c_str());
				if (filePathToOpen.GetExtension().compare(L".nxl") == 0)
				{
					filePathToOpen = filePath.substr(0, wcslen(filePath.c_str()) - 4).c_str();
				}
				OtkXUtils::OtkXFileNameData nameData(filePathToOpen.ToString());
				xstring fileName(RMXUtils::ws2s(filePathToOpen.c_str()).c_str());
				pfcModelDescriptor_ptr pDesc = pfcModelDescriptor::CreateFromFileName(fileName);

				auto ops = pfcRetrieveModelOptions::Create();
				ops->SetAskUserAboutReps(false);
				pfcModel_ptr pMdl = pSess->RetrieveModelWithOpts(pDesc, ops);
				if (pMdl)
				{
					GetDepNxlFiles(pMdl, nxlFiles);
					pMdl->EraseWithDependencies();
				}
				/*xstring dir = CA2XS(RMXUtils::ws2s(nameData.GetDirectoryPath()).c_str());
				xstring saveDir = pSess->GetCurrentDirectory();
				bool currDirChanged = false;
				if (saveDir != dir)
				{
					pSess->ChangeDirectory(dir);
					currDirChanged = true;
					RMX_AddRPMDir(CXS2W(saveDir));
				}

				pfcModelType mdlType = OtkX_GetModelType(RMXUtils::ws2s(nameData.GetExtension()).c_str());
				xstring fileName(RMXUtils::ws2s(nameData.GetFileName()).c_str());
				pfcModelDescriptor_ptr pDesc = pfcModelDescriptor::Create(mdlType, fileName, "");
				if (pDesc)
				{
					pfcModel_ptr pMdl = pSess->GetModelFromDescr(pDesc);
					if(pMdl != nullptr)
					{
						pMdl->EraseWithDependencies();
					}
					pMdl = pSess->RetrieveModel(pDesc);
					GetDepNxlFiles(pMdl, nxlFiles);
					pMdl->EraseWithDependencies();
				}

				if (currDirChanged)
				{
					pSess->ChangeDirectory(saveDir);
					RMX_AddRPMDir(CXS2W(dir));
				}*/
			}
		}
		OTKX_EXCEPTION_HANDLER();
	}

	bool AnyProtectedInModel(pfcModel_ptr pParentMdl)
	{
		COtkXModel xMdl(pParentMdl);
		return xMdl.IsProtected() || xMdl.IsDepProtected();
	}

	bool UnzipFile(const wstring& zipFile, const wstring& destDir)
	{
		static const wstring& cmdEXE = RMXUtils::getEnv(L"COMSPEC");
		static const wstring& creoCommonFiles = RMXUtils::getEnv(L"CREO_COMMON_FILES");
		if (creoCommonFiles.empty())
		{
			XLOG_ERROR(L"CREO_COMMON_FILES env var not found. Cannot find unzip.exe installed by Creo");
			return false;
		}
		wstring unzipCmd = creoCommonFiles + L"\\x86e_win64\\cedirect\\binx64\\unzip.exe";
		//const std::wstring cmd = cmdEXE + L" /c \"" + unzipCmd + L"\" \"" + zipFile + L"\" -d \"" + destDir + L"\"";
		const std::wstring cmd = L"\"" + unzipCmd + L"\" \"" + zipFile + L"\" -d \"" + destDir + L"\"";
		// Create the child process.  
		LPWSTR szCmd = const_cast<wchar_t*>(cmd.c_str());
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

		if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			DWORD dwWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);

			// Close process and thread handles. 
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			if (dwWaitResult == WAIT_OBJECT_0 && CPathEx::IsDir(destDir.c_str()))
			{
				return true;
			}
			else
			{
				XLOG_ERROR(L"Cannot unzip file: " << zipFile.c_str());
			}
		}
		else
		{
			XLOG_ERROR(L"Cannot CreateProcess: " << szCmd);
		}

		return false;
	}

	pfcModel_ptr RetrieveModel(const wstring& filePath)
	{
		pfcModel_ptr  pMdl = nullptr;
		try
		{
			auto pSess = pfcGetProESession();
			if (pSess)
			{
				CPathEx filePathToOpen(filePath.c_str());
				if (filePathToOpen.GetExtension().compare(L".nxl") == 0)
				{
					filePathToOpen = filePath.substr(0, wcslen(filePath.c_str()) - 4).c_str();
				}
				OtkXUtils::OtkXFileNameData nameData(filePathToOpen.ToString());
				xstring dir = CA2XS(RMXUtils::ws2s(nameData.GetDirectoryPath()).c_str());
				xstring saveDir = pSess->GetCurrentDirectory();
				bool currDirChanged = false;
				if (saveDir != dir)
				{
					pSess->ChangeDirectory(dir);
					currDirChanged = true;
					RMX_AddRPMDir(CXS2W(saveDir));
				}
	
				pfcModelType mdlType = OtkX_GetModelType(nameData.GetExtension());
				xstring fileName(RMXUtils::ws2s(nameData.GetFileName()).c_str());
				pfcModelDescriptor_ptr pDesc = pfcModelDescriptor::Create(mdlType, fileName, "");
				if (pDesc)
				{
					pMdl = pSess->RetrieveModel(pDesc);
				}

				/*if (currDirChanged)
				{
					pSess->ChangeDirectory(saveDir);
					RMX_AddRPMDir(CXS2W(dir));
				}*/
			}
		}
		OTKX_EXCEPTION_HANDLER();

		return pMdl;
	}

	std::string GetNxlFileTags(const std::wstring & filePath)
	{
		std::string tags;
		if (!CPathEx::IsFile(filePath))
			return tags;
		
		char* szTags = nullptr;
		RMX_GetTags(filePath.c_str(), &szTags);
		if (szTags != nullptr)
		{
			tags = string(szTags);
			RMX_ReleaseArray((void*)szTags);
		}
		
		return tags;
	}

	wstring GetNativeFilePath(const std::wstring & filePath)
	{
		OtkXFileNameData nameData(filePath);
		wstring nativeFilePath(filePath);
		if (nameData.GetExtension().compare(L".nxl") == 0)
		{
			nativeFilePath = filePath.substr(0, wcslen(filePath.c_str()) - 4).c_str();
		}

		return nativeFilePath;
	}

	bool IsAsm(const std::wstring & filePath)
	{	
		const wstring& nativeFilePath = GetNativeFilePath(filePath);
		OtkXFileNameData nameData(nativeFilePath);
		return (wcsicmp(nameData.GetExtension().c_str(), L"asm") == 0);	
	}

	bool IsDrw(const std::wstring & filePath)
	{
		const wstring& nativeFilePath = GetNativeFilePath(filePath);
		OtkXFileNameData nameData(nativeFilePath);
		return (wcsicmp(nameData.GetExtension().c_str(), L"drw") == 0);
	}

	bool IsSaveFileIterationEnabled()
	{
		auto pSess = pfcGetProESession();
		xstring fileIterEnabled = pSess->GetConfigOption("save_file_iterations");

		return (fileIterEnabled.ToLower() == "yes");
	}

	wstring EnsureNxlExt(const std::wstring & filePath)
	{
		CPathEx testFile(filePath.c_str());
		if (testFile.GetExtension() != L".nxl")
			testFile += L".nxl";

		return testFile.ToString();
	}

	void RemoveFiles(const wstring& dir, bool removeDir)
	{
		if (dir.empty())
		{
			LOG_ERROR(L"RemoveFiles: No dir specified");
			return;
		}
		
		WIN32_FIND_DATA findData;
		wstring searchDir(dir + L"\\*");
		HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			// No file exists in the folder
			return;
		}
		CPathEx srcFile;
		do
		{
			srcFile = dir.c_str();
			srcFile /= findData.cFileName;
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0)
				{
					RemoveFiles(srcFile.ToString(), true);
				}
			}
			else
			{
				wstring nxlFile = EnsureNxlExt(srcFile.ToString());
				if (CPathEx::IsFile(nxlFile))
				{
					if (!RMX_DeleteNxlFile(nxlFile.c_str()))
						XLOG_ERROR(L"Cannot remove nxl file: " << nxlFile.c_str());
				}
				else
				{
					if (!CPathEx::RemoveFile(srcFile.ToString()))
						XLOG_ERROR(L"Cannot remove file: " << srcFile.c_str());
				}
			}
		} while (FindNextFile(hFind, &findData));

		if (removeDir)
		{
			if (RMX_IsRPMFolder(dir.c_str()))
				RMX_RemoveRPMDir(dir.c_str());
			if (_wrmdir(dir.c_str()) == 0)
				LOG_INFO(L"Folder removed: " << dir.c_str());
			else
				XLOG_ERROR(L"Cannot remove folder: " << dir.c_str());
		}
		FindClose(hFind);
	}

	string ReadScripts(const wstring& fileName)
	{
		if (!CPathEx::IsFile(fileName))
		{
			XLOG_ERROR(L"Cannot read scripts. File not found: " << fileName.c_str());
			return string("");
		}
		ifstream ifs(RMXUtils::ws2s(fileName));
		string scripts((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		scripts.erase(std::remove(scripts.begin(), scripts.end(), '\n'), scripts.end());
		scripts.erase(std::remove(scripts.begin(), scripts.end(), '\r'), scripts.end());
		return scripts;
	}
	
	bool OpenNotepad(const std::wstring & filePath)
	{
		LOG_INFO_FMT(L"Open in Notepad: '%s'", filePath.c_str());
		WCHAR wzCmdLine[MAX_PATH];
		wstring appPath = RMXUtils::getEnv(L"SystemRoot") +  L"\\notepad.exe";
		wsprintfW(wzCmdLine, L"%s %s", appPath.c_str(), filePath.c_str());

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = TRUE;
		
		if (CreateProcessW(
			NULL,
			wzCmdLine,
			NULL,
			NULL,
			false,
			CREATE_NEW_PROCESS_GROUP,
			NULL,
			NULL,
			&si,
			&pi))
		{
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return true;
		}
		
		return false;
	}

	wfcStatus AnyProtectedInCurrentModel()
	{
		auto pMdl = OtkX_GetCurrentModel();
		FAILED_RETURN(pMdl != nullptr, wfcTK_BAD_INPUTS, L"Current model not found");

		return AnyProtectedInModel(pMdl) ? wfcTK_NO_ERROR : wfcTK_GENERAL_ERROR;
	}

	wfcStatus IsCurrentModelProtected()
	{
		auto pMdl = OtkX_GetCurrentModel();
		FAILED_RETURN(pMdl != nullptr, wfcTK_BAD_INPUTS, L"Current model not found");

		wstring mdlPath = CXS2W(pMdl->GetOrigin());
		return RMX_IsProtectedFile(mdlPath.c_str()) ? wfcTK_NO_ERROR : wfcTK_BAD_INPUTS;
	}

	wfcStatus ValidateInputBeforeRun()
	{
		auto pMdl = OtkX_GetCurrentModel();
		FAILED_RETURN(pMdl != nullptr, wfcTK_BAD_INPUTS, L"Current model not found");
		wfcStatus ret = wfcTK_NO_ERROR;
		if (pMdl->GetType() == pfcMDL_ASSEMBLY ||
			pMdl->GetType() == pfcMDL_DRAWING) 
			return AnyProtectedInCurrentModel();
		else
			return IsCurrentModelProtected();
	}

	wfcStatus AnyProtectedInSelection()
	{
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			pfcSelectionBuffer_ptr pSelBuffer = pSess->GetCurrentSelectionBuffer();
			pfcSelections_ptr pSels = pSelBuffer->GetContents();
	
			FAILED_RETURN(pSels != nullptr, wfcTK_BAD_INPUTS, L"Selection not found");
			xint numSels = pSels->getarraysize();
			FAILED_RETURN(numSels > 0, wfcTK_BAD_INPUTS, L"Selection not found");

			for (int i = 0; i < numSels; ++i)
			{
				pfcModel_ptr pSelMdl = pSels->get(i)->GetSelModel();
				if (pSelMdl != nullptr)
				{
					xstring newFilePath = pSelMdl->GetOrigin();
					if (RMX_IsProtectedFile(CXS2W(newFilePath)))
					{
						return wfcTK_NO_ERROR;
					}
				}
			}
		}
		OTKX_EXCEPTION_HANDLER();

		return wfcTK_GENERAL_ERROR;
	}

	

} // namespace TestHlp
