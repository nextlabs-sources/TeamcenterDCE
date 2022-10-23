#include "IpemRMX.h"

#include <exception>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>

#include <ProArray.h>
#include <ProElement.h>
#include <ProMdl.h>
#include <ProUtil.h>
#include <ProWstring.h>

#include <wfcSession.h>
#include <xstring.h>

#include "..\common\CreoRMXLogger.h"
#include "OtkXModel.h"
#include "OtkXTypes.h"
#include "OtkXUIStrings.h"
#include "OtkXUtils.h"
#include <PathEx.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>
#include <XmlParser.h>

#include <LibRMX.h>
#include "UsageController.h"

using namespace std;
using namespace OtkXUtils;

namespace IpemRMX
{
	static const wchar_t* ID_FILENAME_PREFIX = L"creormx_pid_";
	static ProName ARG_XML_FILENAME = L"FILE_NAME";
	static ProName ARG_FILENAME = L"FILE_NAME";
	static ProName ARG_RECURSIVE = L"RECURSIVE";
	static ProName ARG_DIR = L"DIR";
	static RegistryFile s_registryFile;
	static const wchar_t* REGISTRY_FILENAME = L"ipem.txr";
	static bool g_isEAIBatchModel = false;
	static const wchar_t* ENV_IPEMRMX_JTDISPATCH = L"IPEMRMX_JTDISPATCH";

	enum eIpemOperation
	{
		IpemOp_Unknown,
		IpemOp_Save,
		IpemOp_SaveAll,
		IpemOp_SaveAs,
		IpemOp_Replace,
		IpemOp_New,
		IpemOp_Revise
	};

	enum eIpemAction
	{
		IpemAction_Validation,
		IpemAction_New,
		IpemAction_Replace
	};

	enum IpemRightStatus
	{
		Ipem_Right_Grant,
		Ipem_Right_Deny_Save,
		Ipem_Right_Deny_SaveWithAuxi,
		Ipem_Right_Deny_SaveAs,
		Ipem_Right_Deny_EditOnSaveAs,
		Ipem_Right_Deny_FileNotExists
	};

	typedef IpemRightStatus(*pfIpemCheckRight)(const COtkXModel& xMdl);
	
	struct IpemModel
	{
		TModelId id;
		eIpemOperation eOpType;
		DWORD dwAccessRight;
		pfcModel_ptr pMdl;
		pfIpemCheckRight checkRightCB;
	};

	IpemRightStatus CheckRightOnSave(const COtkXModel& xMdl)
	{
		IpemRightStatus ret = Ipem_Right_Grant;
		auto usageRight = CUsageController::CheckRight(xMdl, RMX_RIGHT_EDIT);
		if (usageRight == Usage_Right_Deny)
			ret = Ipem_Right_Deny_Save;
		else if (usageRight == Usage_Right_Deny_FileNotExists)
			ret = Ipem_Right_Deny_FileNotExists;

		return ret;
	}

	IpemRightStatus CheckRightOnCopy(const COtkXModel& xMdl)
	{
		IpemRightStatus ret = Ipem_Right_Grant;
		// Ipem always make model dirty after new/replace as copy file. 
		// So always check edit permission for these operations
		auto usageRight = CUsageController::CheckRight(xMdl, RMX_RIGHT_DOWNLOAD, true);
		if (usageRight == Usage_Right_Deny)
			ret = Ipem_Right_Deny_SaveAs;
		else if (usageRight == Usage_Right_Deny_FileNotExists)
			ret = Ipem_Right_Deny_FileNotExists;
		else if (usageRight == Usage_Right_DenyEdit)
			ret = Ipem_Right_Deny_EditOnSaveAs;
		
		return ret;
	}

	//IpemRightStatus CheckAuxiRightOnSave(const COtkXModel& xMdl)
	//{
	//	// This is used to check if file has save as permission to export auxi file
	//	// such as JT on save operation.
	//	// TODO: Ipem bug. Save JT Files option is always "no". Cannot determine if this option
	//	// is selected and check right for it. 
	//	if (!xMdl.CheckRights(RMX_RIGHT_DOWNLOAD))
	//		return Ipem_Right_Edit_AuxiDeny;

	//	return Ipem_Right_Grant;
	//}

	/*!
	 * Check if "Save JT Files" option is selected.
	 */
	bool SaveJTEnabled(std::shared_ptr<RMXUtils::xml_node> pRootNode)
	{
		auto pAuxNode = pRootNode->find_child_element(L"auxiliary_files");
		if (pAuxNode != nullptr)
		{
			auto children = pAuxNode->get_children();
			for (const auto& p : children)
			{
				const wstring& cadtopdmCtrl = p->get_attribute(L"label");
				if (_wcsicmp(L"Save JT Files", cadtopdmCtrl.c_str()) == 0)
				{
					const wstring& value = p->get_attribute(L"value");
					return (_wcsicmp(value.c_str(), L"yes") == 0);
				}
			}
		}
		
		return false;
	}

	bool BuildProcessModel(std::shared_ptr<RMXUtils::xml_node> pNode,
		const wstring& op,
		const wstring& action,
		IpemModel& model)
	{
		if ((pNode == nullptr) || !pNode->is_element_node() ||
			(wcsicmp(pNode->get_name().c_str(), L"model") != 0))
			return false;

		eIpemOperation opType = IpemOp_Unknown;
		if (wcsicmp(action.c_str(), L"validation") == 0)
		{
			if (wcsicmp(op.c_str(), L"Save") == 0)
			{
				opType = IpemOp_Save;
			}	
			else if (wcsicmp(op.c_str(), L"Save As") == 0)
			{
				opType = IpemOp_SaveAs;
			}		
			else if (wcsicmp(op.c_str(), L"Save All") == 0)
			{
				opType = IpemOp_SaveAll;
			}	
		}
		else if ((wcsicmp(action.c_str(), L"new") == 0))
		{
			opType = IpemOp_New;
		}
		else if ((wcsicmp(action.c_str(), L"replace") == 0))
		{
			opType = IpemOp_Replace;
		}

		if (opType == IpemOp_Unknown)
		{
			LOG_WARN_FMT(L"Ignore. Unsupported action or operation on the mode(action: %s, operation: %s", action.c_str(), op.c_str());
			return false;
		}
	
		auto IsTagYes = [&](const std::wstring& tagName) -> bool {
			auto attr = pNode->get_attribute(tagName);
			if (!attr.empty() && wcsicmp(attr.c_str(), L"yes") == 0)
				return true;

			return false;
		};

		DWORD rightToCheck = RMX_RIGHT_Nil;
		if (opType == IpemOp_Save || opType == IpemOp_SaveAs || opType == IpemOp_SaveAll)
		{
			if (!IsTagYes(L"save"))
				return false;

			model.checkRightCB = &CheckRightOnSave;
		}
		else if (opType == IpemOp_New || opType == IpemOp_Replace)
		{
			if (!IsTagYes(L"selected"))
				return false;

			model.checkRightCB = &CheckRightOnCopy;
		}

		const wstring& mdlName = pNode->get_attribute(L"name");
		if (mdlName.empty())
		{
			LOG_ERROR(L"name tag not found in in mode node");
			return false;
		}

		const wstring& mdlType = pNode->get_attribute(L"type");
		if (mdlType.empty())
		{
			LOG_ERROR(L"type tag not found in in mode node");
			return false;
		}

		OTKX_MAKE_ID(mdlName, mdlType, model.id);
		// Look through all model nodes
		model.pMdl = OtkX_GetModelFromFileName(model.id);
		if (model.pMdl == nullptr)
		{
			LOG_ERROR_FMT(L"IPEMRMX - Ignored. Model not found in session (%s)", model.id.c_str());
			return false;
		}
		
		model.eOpType = opType;
		model.dwAccessRight = rightToCheck;

		return true;
	}

	std::wstring GetRMXDllIDFilePath()
	{
		static wstring s_idFilePath;
		if (s_idFilePath.empty())
		{
			wstring dllIDFile = GetTempFolder();
			if (!dllIDFile.empty())
			{
				CPathEx dllIDPath(dllIDFile.c_str());
				dllIDPath /= ID_FILENAME_PREFIX;
				dllIDPath += std::to_wstring(GetCurrentProcessId()).c_str();
				dllIDPath += L".txt";
				s_idFilePath = dllIDPath.ToString();
			}
			else
			{
				LOG_ERROR(L"$LOCALAPPDATA env var not found");
			}
		}

		return s_idFilePath;
	}

	void DeleteRMXDllIdFile(const wstring& dllFilePath)
	{
		if (CPathEx::IsFile(dllFilePath))
		{
			if (!CPathEx::RemoveFile(dllFilePath))
			{
				DWORD dwErrCode = (errno == EACCES) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND;
				LOG_ERROR_FMT(L"IPEMRMX - Cannot delete application id file: '%s' (error: %s)", dllFilePath.c_str(), (LPCTSTR)CSysErrorMessage(dwErrCode));
			}
			else
			{
				LOG_DEBUG_FMT(L"IPEMRMX - Application id file deleted: %s", dllFilePath.c_str());
			}
		}
	}

	void SaveRMXDllIdToFile(const std::wstring& appId)
	{
		const wstring& dllFilePath = GetRMXDllIDFilePath();
		if (!dllFilePath.empty())
		{
			DeleteRMXDllIdFile(dllFilePath);

			std::wofstream logFile(dllFilePath, ios_base::out);
			if (logFile.fail())
			{
				LOG_ERROR_FMT(L"IPEMRMX - Failed to write application id: %s", dllFilePath.c_str());
			}
			else
			{
				logFile << appId << endl;
				logFile.close();
				LOG_INFO_FMT(L"IPEMRMX - Application id saved in %s", dllFilePath.c_str());
			}
		}
	}

	void DeleteRMXDllIdFile()
	{
		DeleteRMXDllIdFile(GetRMXDllIDFilePath());
	}

	void UpdateRegistryFile(const wstring& registryFile)
	{
		IPEMRMX_LOG_ENTER

		if (!CPathEx::IsFile(registryFile))
			return;

		LOG_DEBUG_FMT(L"IPEMRMX - Parsing registry: %s", registryFile.c_str());
		try
		{
			RMXUtils::xml_document xmlDoc;
			xmlDoc.load_from_file(registryFile);

			std::shared_ptr<RMXUtils::xml_node> pRoot = xmlDoc.document_root();
			if (pRoot == nullptr)
			{
				LOG_WARN_FMT(L"IPEMRMX - Ignore. Root node not found: %s", registryFile.c_str());
				return;
			}

			bool changed = false;
			for (auto pNode : pRoot->get_children())
			{
				if ((pNode == nullptr) || !pNode->is_element_node() ||
					(wcsicmp(pNode->get_name().c_str(), L"cad_file") != 0))
					continue;

				wstring name = pNode->get_attribute(L"name");
				LOG_DEBUG_FMT(L"IPEMRMX - Processing cad_file: %s", name.c_str());
				CPathEx namePath(name.c_str());
				if (wcsicmp(namePath.GetExtension().c_str(), NXL_FILE_EXT) == 0)
				{
					name = name.substr(0, name.length() - 4);
					OtkXFileNameData nameData(name);
					pNode->set_attribute(L"name", nameData.GetFileName());
					LOG_INFO_FMT(L"IPEMRMX - cad_file name adjusted to: %s", nameData.GetFileName().c_str());
					changed = true;
				}
			}

			if (changed)
				xmlDoc.to_file(registryFile);
		}
		catch (const std::exception& e) {
			LOG_ERROR_FMT(L"IPEMRMX - Exception when parsing xml file: %s (error: %s)", registryFile.c_str(), RMXUtils::s2ws(e.what()).c_str());
		}
	}

	std::wstring GetProcessCommandLine(unsigned long process_id)
	{
		HANDLE h = NULL;
		std::wstring commandline;

		typedef NTSTATUS(WINAPI* NtQueryInformationProcess_t)(
			_In_      HANDLE           ProcessHandle,
			_In_      PROCESSINFOCLASS ProcessInformationClass,
			_Out_     PVOID            ProcessInformation,
			_In_      ULONG            ProcessInformationLength,
			_Out_opt_ PULONG           ReturnLength
			);

		static NtQueryInformationProcess_t PtrNtQueryInformationProcess = (NtQueryInformationProcess_t)::GetProcAddress(LoadLibraryW(L"ntdll.dll"), "NtQueryInformationProcess");

		if (NULL == PtrNtQueryInformationProcess) {
			return commandline;
		}

		do {

			PROCESS_BASIC_INFORMATION pbi = { 0 };
			ULONG_PTR returned_length = 0;

			h = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
			if (NULL == h) {
				break;
			}

			LONG status = PtrNtQueryInformationProcess(h, ProcessBasicInformation, &pbi, sizeof(pbi), (PULONG)&returned_length);
			if (0 != status) {
				break;
			}

			PEB peb = { 0 };
			if (!ReadProcessMemory(h, pbi.PebBaseAddress, &peb, sizeof(PEB), &returned_length)) {
				break;
			}

			RTL_USER_PROCESS_PARAMETERS upp = { 0 };
			if (!ReadProcessMemory(h, peb.ProcessParameters, &upp, sizeof(RTL_USER_PROCESS_PARAMETERS), &returned_length)) {
				break;
			}

			if (0 == upp.CommandLine.Length) {
				break;
			}

			std::vector<wchar_t> buf;
			buf.resize((upp.CommandLine.Length + 3) / 2, 0);
			if (!ReadProcessMemory(h, upp.CommandLine.Buffer, buf.data(), upp.CommandLine.Length, &returned_length)) {
				break;
			}

			commandline = buf.data();

		} while (false);

		if (h != NULL) {
			CloseHandle(h);
			h = NULL;
		}

		return std::move(commandline);
	}

	DWORD GetIpemProcessId()
	{
		DWORD xtopPId = GetCurrentProcessId();
		PROCESSENTRY32 pe32;

		// Take a snapshot of all processes in the system.
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			return 0;
		}

		// Set the size of the structure before using it.
		pe32.dwSize = sizeof(PROCESSENTRY32);

		// Retrieve information about the first process,
		// and exit if unsuccessful
		if (!Process32First(hProcessSnap, &pe32))
		{
			CloseHandle(hProcessSnap);          // clean the snapshot object
			return 0;
		}

		// Now walk the snapshot of processes, and
		// find out the ipem java.exe
		DWORD parentId = 0;
		do
		{
			if (pe32.th32ParentProcessID == xtopPId && 
				wcsstr(pe32.szExeFile, L"cmd.exe") != NULL) {
				const std::wstring& cmdLine = GetProcessCommandLine(pe32.th32ProcessID);
				if (wcsstr(cmdLine.c_str(), L"ipemrunner_sync.bat") != NULL)
				{
					parentId = pe32.th32ProcessID;
				}
			}
			else if (pe32.th32ParentProcessID == parentId &&
				wcsstr(pe32.szExeFile, L"java.exe") != NULL)
			{
				return pe32.th32ProcessID;
			}
		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);

		return 0;
	}

	bool Register()
	{
		IPEMRMX_LOG_ENTER

		s_registryFile.Clear();

		// Save the dll id into a text file so that ipemrmx can 
		// access from user exit
		ProName dllName;
		if (ProToolkitDllIdGet(NULL, dllName) == PRO_TK_NO_ERROR)
		{
			SaveRMXDllIdToFile(dllName);
		}
		else
		{
			LOG_ERROR(L"IPEMRMX - Cannot retrieve CreoRMX application id. Usage control for IPEM will lose");
		}

		// Add ipem process as trusted
		auto pid = GetIpemProcessId();
		if (pid == 0)
		{
			LOG_ERROR_FMT(L"IPEMRMX - Cannot register IPEM process(error: %s).", (LPCTSTR)CSysErrorMessage(GetLastError()));
		}
		else if (RMX_RPMAddTrustedProcess(pid))
		{
			LOG_INFO_FMT(L"IPEMRMX - IPEM process registered as trusted. PID=%u", pid);
			return true;
		}
		else
		{
			LOG_ERROR_FMT(L"IPEMRMX - Failed to add IPEM process as trusted. PID=%u", pid);
		}

		return false;
	}

	void Unregister()
	{
		IPEMRMX_LOG_ENTER
		DeleteRMXDllIdFile();

		s_registryFile.Clear();
		
		// Clean up decrypt files generated by dispatch translating job
		if (IpemRMX::JTDispRunning())
			CleanupTrans();
	}

	std::wstring GetCurrentRegistryFilePath()
	{
		CPathEx path(OtkX_GetCurrentDirectory().c_str());
		path /= REGISTRY_FILENAME;
		return path.ToString();
	}

	void RefreshCurrentRegistryFile()
	{
		IPEMRMX_LOG_ENTER

		const auto& lastFilePath = GetCurrentRegistryFilePath();;
		UpdateRegistryFile(lastFilePath);

		struct _stat status;
		if (_wstat(lastFilePath.c_str(), &status) == 0)
		{
			///*if ((wcsicmp(s_registryFile.path.c_str(), lastFilePath.c_str()) != 0) ||
			//	(s_registryFile.modifiedTime != status.st_mtime))
			//{*/
			
			//}
			s_registryFile.path = lastFilePath;
			s_registryFile.modifiedTime = status.st_mtime;
		}
		else
		{
			s_registryFile.Clear();
		}
	}

	void EnsureNxlExtInWorkspace(bool force)
	{
		IPEMRMX_LOG_ENTER

		// Only process if the registry file is modified. 
		CPathEx registryPath(GetCurrentRegistryFilePath().c_str());
		struct _stat status;
		if (_wstat(registryPath.c_str(), &status) != 0)
		{
			LOG_DEBUG_FMT(L"IPEMRMX - Ignore. Registry file not found: %s", registryPath.c_str());
			return;
		}

		if (!force) {
			
			if ((wcsicmp(s_registryFile.path.c_str(), registryPath.c_str()) == 0) &&
				(s_registryFile.modifiedTime == status.st_mtime))
			{
				LOG_DEBUG_FMT(L"IPEMRMX - Ignore. Registry file not modified: %s", registryPath.c_str());
				return;
			}
		}

		// Refresh registry file cache
		s_registryFile.path = registryPath.ToString();
		s_registryFile.modifiedTime = status.st_mtime;
		
		// Walk thru cad files to fix the .nxl extension
		// if it's missing
		const wstring& currDir = registryPath.GetParentPath();
		
		bool changed = false;
		LOG_DEBUG_FMT(L"IPEMRMX - Validating protected files from current workspace: %s", currDir.c_str());
		WIN32_FIND_DATA fData;
		HANDLE hFind;
		const wstring searchDir(currDir + L"\\*");
		if ((hFind = FindFirstFile(searchDir.c_str(), &fData)) != INVALID_HANDLE_VALUE) {
			do {
				if (wcscmp(fData.cFileName, L".") == 0 || wcscmp(fData.cFileName, L"..") == 0)
					continue;

				CPathEx foundPath(currDir.c_str());
				foundPath /= fData.cFileName;
				if ((fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					LOG_DEBUG_FMT(L"IPEMRMX - Validating file: %s", fData.cFileName);

					OtkXFileNameData nameData(foundPath.ToString());
					if (wcsicmp(nameData.GetExtension().c_str(), NXL_FILE_EXT_WITHOUT_DOT) == 0)
						continue;

					if (!OtkX_IsNativeModelType(nameData.GetExtension()))
						continue;

					RMX_EnsureNxlExtension(foundPath.c_str());
					
				}
			} while (FindNextFile(hFind, &fData) != 0);

			FindClose(hFind);
		}
	}

	void SetEAIBatchModel()
	{
		g_isEAIBatchModel = true;
	}

	bool IsEAIBatchModel() { return g_isEAIBatchModel;  }

	std::wstring GetTempFolder()
	{
		wstring ret;
		wchar_t szTempDir[MAX_PATH];
		DWORD dwRetVal = 0;
		dwRetVal = GetTempPath(MAX_PATH, szTempDir);
		if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
		{
			LOG_ERROR("Failed to get temp folder.");
		}
		else
		{
			CPathEx tempPath(szTempDir);
			tempPath /= L"ipemrmx";
			if (!CPathEx::IsDir(tempPath.ToString()) && CPathEx::CreateDirectories(tempPath.ToString()) != 0)
			{
				LOG_ERROR("Failed to create temp ipemrmx folder." << tempPath.c_str());
			}
			else
			{
				ret = tempPath.c_str();
			}
		}

		return ret;
	}

	void CleanupTrans()
	{
		CPathEx processFilePath(GetTempFolder().c_str());
		processFilePath /= L"trans_process";
		if (!CPathEx::IsFile(processFilePath.ToString()))
		{
			LOG_DEBUG(processFilePath.c_str() << " not found. Ignore");
			return;
		}
		static const std::wstring rmxRoot = RMXUtils::getEnv(ENV_CREO_RMX_ROOT);
		if (!rmxRoot.empty())
		{
			CPathEx scriptFile(rmxRoot.c_str());
#if defined(CREO_VER) && CREO_VER == 4
			scriptFile /= L"Creo 4.0\\ipem\\transclean.bat";
#else
			scriptFile /= L"Creo 7.0\\ipem\\transclean.bat";
#endif
			if (!CPathEx::IsFile(scriptFile.ToString()))
			{
				LOG_ERROR(scriptFile.c_str() << " not found.");
				return;
			}
			else
			{
				wifstream  ifs(processFilePath.ToString());
				wstring line;
				while (ifs.good() && std::getline(ifs, line)) {

					if (!line.empty())
					{
						const wstring cmd = L"\"" + scriptFile.ToString() + L"\" " + line;
						string ret;
						RMXUtils::ExecuteCmd(cmd, ret);
					}
					break;
				}
			}
		}
	
		if (CPathEx::RemoveFile(processFilePath.c_str()))
			LOG_INFO(processFilePath.c_str() << L" deleted");
	}

	bool JTDispEnabled()
	{
		static const std::wstring jtDispEnable = RMXUtils::getEnv(ENV_IPEMRMX_JTDISPATCH);
		if (!jtDispEnable.empty() && jtDispEnable.compare(L"1") == 0)
			return true;
		return false;
	}

	bool JTDispRunning()
	{
		if (IpemRMX::JTDispEnabled() && IpemRMX::IsEAIBatchModel() && !OtkXUtils::OtkX_RunInGraphicMode())
			return true;

		return false;
	}
} //namespace IpemRMX

namespace ProTkHlp
{
	/*====================================================================*\
	FUNCTION :  ProUtilWstrcmp()
	PURPOSE  :  strcmp() but for wide strings, and case-sensitive.
	\*====================================================================*/
	int ProTk_Wstrcmp(
		wchar_t *ws1,
		wchar_t *ws2)
	{
		int result, status;

		status = ProWstringCompare(ws1, ws2, PRO_VALUE_UNUSED, &result);

		return (result);
	}

	ProError ProTk_ArgumentValueGet(ProArgument* args, ProName label, ProValueData* data)
	{
		int size, i;
		ProError status;
		status = ProArraySizeGet(args, &size);
		if (status != PRO_TK_NO_ERROR)
			return PRO_TK_INVALID_TYPE;

		for (i = 0; i < size; i++)
		{
			if (ProTk_Wstrcmp(args[i].label, label) == 0)
			{
				*data = args[i].value;
				return PRO_TK_NO_ERROR;
			}
		}
		return PRO_TK_E_NOT_FOUND;
	}

	ProError ProTk_ModelNameGetByArg(ProArgument* input_arguments, ProName label, ProMdlName name)
	{
		ProValueData data;
		int length;
		ProError status;

		/*--------------------------------------------------------------------*\
		Get the value of argument
		\*--------------------------------------------------------------------*/

		//status = ProArgumentByLabelGet(input_arguments, label, &data);
		status = ProTk_ArgumentValueGet(input_arguments, label, &data);
		if (status != PRO_TK_NO_ERROR)
			return status;

		/*--------------------------------------------------------------------*\
		Data will always be of wstring type
		\*--------------------------------------------------------------------*/

		if (data.type != PRO_VALUE_TYPE_WSTRING)
		{
			status = PRO_TK_INVALID_TYPE;
			return status;
		}
		ProWstringLengthGet(data.v.w, &length);

		if (length > PRO_NAME_SIZE - 1)
		{
			status = PRO_TK_LINE_TOO_LONG;
			return status;
		}

		/*--------------------------------------------------------------------*\
		Copy the data in output value
		\*--------------------------------------------------------------------*/

		ProWstringCopy(data.v.w, name, PRO_VALUE_UNUSED);
		status = PRO_TK_NO_ERROR;
		return status;

	}

	ProError ProTk_ModelExtensionGetByArg(ProArgument* input_arguments, ProName label, ProMdlExtension ext)
	{
		ProValueData data;
		int length;
		ProError status;

		/*--------------------------------------------------------------------*\
		Get the value of argument
		\*--------------------------------------------------------------------*/

		//status = ProArgumentByLabelGet(input_arguments, label, &data);
		status = ProTk_ArgumentValueGet(input_arguments, label, &data);
		if (status != PRO_TK_NO_ERROR)
			return status;

		/*--------------------------------------------------------------------*\
		Data will always be of wstring type
		\*--------------------------------------------------------------------*/

		if (data.type != PRO_VALUE_TYPE_WSTRING)
		{
			status = PRO_TK_INVALID_TYPE;
			return status;
		}
		ProWstringLengthGet(data.v.w, &length);

		if (length > PRO_EXTENSION_SIZE - 1)
		{
			status = PRO_TK_LINE_TOO_LONG;
			return status;
		}

		/*--------------------------------------------------------------------*\
		Copy the data in output value
		\*--------------------------------------------------------------------*/

		ProWstringCopy(data.v.w, ext, PRO_VALUE_UNUSED);
		status = PRO_TK_NO_ERROR;
		return status;
	}

} // ProTkHlp

extern "C" IPEMRMX_API ProError RMX_ProCheckRight(ProArgument * inputs, ProArgument ** output)
{
	IPEMRMX_LOG_ENTER
	ProError status = PRO_TK_NO_ERROR;

	std::map<TModelId, wstring> denyCopyFiles;
	std::map<TModelId, wstring> denyCopyWithoutEditFiles;
	std::map<TModelId, wstring> denyEditFiles;
	std::map<TModelId, wstring> denySaveJTFiles;
	std::set<TModelId> processedFiles;
	std::map<IpemRMX::IpemRightStatus, std::set<wstring>> denyFiles;

	ProValueData data;
	ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_XML_FILENAME, &data);
	wstring fileName = data.v.w;
	if (fileName.empty())
	{
		LOG_ERROR(L"IPEMRMX - Failed to call RMX_ProCheckRight. FILE_NAME argument not specified");
		return status;
	}
	
	try
	{
		LOG_DEBUG_FMT(L"IPEMRMX - RMX_ProCheckRight: Parsing xml file %s...", fileName.c_str());
		RMXUtils::xml_document xmlDoc;
		xmlDoc.load_from_file(fileName);
		std::shared_ptr<RMXUtils::xml_node> pRoot = xmlDoc.document_root();
		if (pRoot == nullptr)
		{
			LOG_WARN_FMT(L"IPEMRMX - Ignore. Root node not found: %s", fileName.c_str());
			return status;
		}

		const std::map<std::wstring, RMXUtils::xml_attribute>& attrs = pRoot->get_attributes();
		wstring action = attrs.at(L"action").value();
		wstring opType = attrs.at(L"operation").value();
		bool saveAsJT = IpemRMX::SaveJTEnabled(pRoot);

		LOG_DEBUG_FMT(L"IPEMRMX - RMX_ProCheckRight: Processing all models(action: %s, operation: %s, saveJT: %d)...", action.c_str(), opType.c_str(), saveAsJT);

		for (auto pChild : pRoot->get_children())
		{
			IpemRMX::IpemModel model;
			if (!IpemRMX::BuildProcessModel(pChild, opType, action, model))
				continue;
					
			if (processedFiles.find(model.id) != processedFiles.end())
				continue;

			
			COtkXModel xMdl(model.pMdl);
			LOG_DEBUG_FMT(L"IPEMRMX - RMX_ProCheckRight:  Processing for model %s : %s", xMdl.GetId().c_str(), xMdl.GetOrigin().c_str());
			auto usageRight = model.checkRightCB(xMdl);
			if (usageRight != Usage_Right_Grant)
			{
				denyFiles[usageRight].insert(xMdl.GetOrigin());
				LOG_ERROR_FMT(L"IPEMRMX - RMX_ProCheckRight - Permission not found (%s : %s). Reason: %d", xMdl.GetId().c_str(), xMdl.GetOrigin().c_str(), usageRight);
			}
			else
			{
				/*if (saveAsJT && (model.dwAccessRight != RMX_RIGHT_DOWNLOAD))
				{
					if(xMdl.CheckRights(RMX_RIGHT_DOWNLOAD))
					{
						LOG_ERROR_FMT(L"IPEMRMX - RMX_ProCheckRight - 'SAVEAS' permission not granted to 'Save JT Files'(%s : %s)", xMdl.GetId().c_str(), xMdl.GetOrigin().c_str());
						denySaveJTFiles[xMdl.GetId()] = xMdl.GetOrigin();
					}
				}*/
				LOG_DEBUG_FMT(L"IPEMRMX - RMX_ProCheckRight - Permission granted %s", model.id.c_str());
			}
			processedFiles.insert(model.id);
		}
	}
	OTKX_EXCEPTION_HANDLER()
	catch (const std::exception& e) {
		LOG_ERROR_FMT(L"IPEMRMX - Exception when parsing xml file: %s (error: %s)", fileName.c_str(), RMXUtils::s2ws(e.what()).c_str());
		return status;
	}

	ProArgument *op_args;
	status = ProArrayAlloc(2, sizeof(ProArgument), 2, (ProArray*)&op_args);
	op_args[0].value.type = PRO_VALUE_TYPE_BOOLEAN;
	op_args[0].value.v.b = PRO_B_TRUE;
	op_args[1].value.type = PRO_VALUE_TYPE_WSTRING;
	// Build error message which will be accessed by ipem user exit and pop up error dialog
	wstring retErrMsg;
	if (denyFiles.size() == 1)
	{
		op_args[0].value.v.b = PRO_B_FALSE;
		auto f = denyFiles.begin();
		if (f->first == IpemRMX::Ipem_Right_Deny_FileNotExists)
		{
			retErrMsg = IDS_ERR_DENY_IPEM_OP_LOSEFILE;
		}
		else if (f->first == IpemRMX::Ipem_Right_Deny_Save)
		{
			retErrMsg = RMXUtils::FormatString(IDS_ERR_DENY_IPEM_OP_FMT, L"Edit");		
		}
	/*	else if (f->first == IpemRMX::Ipem_Right_Deny_SaveWithAuxi)
		{
			retErrMsg = retErrMsg + IDS_ERR_DENY_IPEM_OP_SAVEJT;
		}*/
		else if (f->first == IpemRMX::Ipem_Right_Deny_SaveAs)
		{
			retErrMsg = RMXUtils::FormatString(IDS_ERR_DENY_IPEM_OP_FMT, L"Save As");
		}
		else if (f->first == IpemRMX::Ipem_Right_Deny_EditOnSaveAs)
		{
			retErrMsg = IDS_ERR_DENY_IPEM_OP_SAVEAS_NOEDIT;
		}
		for (const auto& elem : f->second)
		{
			retErrMsg = retErrMsg + L"\n    - " + elem;
		}
	}
	else if (denyFiles.size() > 1)
	{
		op_args[0].value.v.b = PRO_B_FALSE;
		retErrMsg = IDS_ERR_DENY_IPEM_OP;
		for (const auto& f : denyFiles)
		{
			if (f.first == IpemRMX::Ipem_Right_Deny_FileNotExists)
			{
				retErrMsg += IDS_ERR_DENY_IPEM_OP_LOSTFILE_REQUIRED;
			}
			else if (f.first == IpemRMX::Ipem_Right_Deny_Save || f.first == IpemRMX::Ipem_Right_Deny_EditOnSaveAs)
			{
				retErrMsg += IDS_ERR_DENY_IPEM_OP_EDIT_REQUIRED;			
			}
			/*if (f.first == IpemRMX::Ipem_Right_Edit_AuxiDeny)
			{
				retErrMsg = retErrMsg + IDS_ERR_DENY_IPEM_OP_SAVEASJT_REQUIRED;
			}*/
			else if (f.first == IpemRMX::Ipem_Right_Deny_SaveAs)
			{
				retErrMsg += IDS_ERR_DENY_IPEM_OP_SAVEAS_REQUIRED;
			}

			for (const auto& elem : f.second)
			{
				retErrMsg = retErrMsg + L"\n- " + elem;
			}
		}
	}
	
	if (!op_args[0].value.v.b)
	{
		ProValuedataWstringSet(&op_args[1].value, const_cast< wchar_t* >(retErrMsg.c_str()));
	}	

	*output = op_args;

	return status;
}

extern "C" IPEMRMX_API ProError RMX_ProFileCheckSaveRight(ProArgument * inputs, ProArgument ** output)
{
	IPEMRMX_LOG_ENTER
	ProError status = PRO_TK_NO_ERROR;

	ProValueData data;
	ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_FILENAME, &data);
	wstring fileArg(data.v.w);
	if (fileArg.empty())
	{
		LOG_ERROR(L"IPEMRMX - Failed to call RMX_ProFileCheckSaveRight. FILE_NAME argument not specified");
		return status;
	}
	
	try
	{
		LOG_DEBUG_FMT(L"IPEMRMX - RMX_ProFileCheckSaveRight: Checking right for %s...", fileArg.c_str());
		// Ipem always make model dirty after new/replace as copy file. 
		// So always check edit permission for these operations
		auto usageRight = CUsageController::CheckRight(fileArg, RMX_RIGHT_EDIT);

		ProArgument *op_args;
		status = ProArrayAlloc(1, sizeof(ProArgument), 1, (ProArray*)&op_args);
		op_args[0].value.type = PRO_VALUE_TYPE_BOOLEAN;
		op_args[0].value.v.b = (usageRight == Usage_Right_Grant) ? PRO_B_TRUE : PRO_B_FALSE;

		*output = op_args;
	}

	OTKX_EXCEPTION_HANDLER()

	return status;
}

extern "C" IPEMRMX_API ProError RMX_ProFileCheckSaveAsRight(ProArgument * inputs, ProArgument ** output)
{
	IPEMRMX_LOG_ENTER
	ProError status = PRO_TK_NO_ERROR;

	ProValueData data;
	ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_FILENAME, &data);
	wstring fileArg(data.v.w);
	if (fileArg.empty())
	{
		LOG_ERROR(L"IPEMRMX - Failed to call RMX_ProFileCheckSaveAsRight. FILE_NAME argument not specified");
		return status;
	}
	ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_RECURSIVE, &data);
	bool recursive(data.v.b);

	try
	{
		LOG_DEBUG_FMT(L"IPEMRMX - RMX_ProFileCheckSaveAsRight: Checking right for %s...", fileArg.c_str());
		ProArgument* op_args;
		status = ProArrayAlloc(2, sizeof(ProArgument), 2, (ProArray*)&op_args);
		op_args[0].value.type = PRO_VALUE_TYPE_BOOLEAN;
		op_args[0].value.v.b = PRO_B_TRUE;
		op_args[1].value.type = PRO_VALUE_TYPE_WSTRING;

		// Ipem always make model dirty after new/replace as copy file. 
		// So always check edit permission for these operations
		auto usageRight = CUsageController::CheckRight(fileArg, RMX_RIGHT_DOWNLOAD, true);
		if (usageRight != Usage_Right_Grant)
		{
			op_args[0].value.v.b = PRO_B_FALSE;
			wstring retMsg;
			ProValuedataWstringSet(&op_args[1].value, const_cast<wchar_t*>(retMsg.c_str()));
		}
		else if(recursive) {
			// Check dependencies recursively
			OtkXFileNameData fNameData(fileArg.c_str());
			const wstring& fileName = fNameData.GetFileName();
			auto pMdl = OtkX_GetModelFromFileName(fileName);
			if (pMdl)
			{
				COtkXModel xMdl(pMdl);
				std::set<std::wstring> denyFiles;
				usageRight = CUsageController::CheckDepRight(xMdl, RMX_RIGHT_DOWNLOAD, denyFiles, true);
				if (usageRight != Usage_Right_Grant)
				{
					wstring retMsg;
					op_args[0].value.v.b = PRO_B_FALSE;
					for (const auto& f : denyFiles)
					{
						retMsg = retMsg + L"\n    - " + f;
					}

					ProValuedataWstringSet(&op_args[1].value, const_cast<wchar_t*>(retMsg.c_str()));
				}
			}
		}
		

		*output = op_args;
	}

	OTKX_EXCEPTION_HANDLER()

	return status;
}

extern "C" IPEMRMX_API ProError RMX_ProFileSetReadOnly(ProArgument * inputs, ProArgument ** output)
{
	IPEMRMX_LOG_ENTER

	ProValueData data;
	ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_FILENAME, &data);
	wstring fileArg(data.v.w);
	if (fileArg.empty())
	{
		LOG_ERROR(L"IPEMRMX - Failed to call RMX_ProFileSetReadOnly. FILE_NAME argument not specified");
		return PRO_TK_NO_ERROR;
	}

	try
	{
		bool result = RMX_AddFileAttributes(fileArg.c_str(), FILE_ATTRIBUTE_READONLY);
		CHK_ERROR(!result, L"IPEMRMX - RMX_ProFileSetReadOnly failed for %s", fileArg.c_str());
	}

	OTKX_EXCEPTION_HANDLER()

	return PRO_TK_NO_ERROR;
}

extern "C" IPEMRMX_API ProError RMX_ProIsEAIBatchMode(ProArgument * inputs, ProArgument * *output)
{
	IPEMRMX_LOG_ENTER
	ProError status = PRO_TK_NO_ERROR;

	try
	{
		ProArgument* op_args;
		status = ProArrayAlloc(1, sizeof(ProArgument), 1, (ProArray*)&op_args);
		op_args[0].value.type = PRO_VALUE_TYPE_BOOLEAN;
		op_args[0].value.v.b = IpemRMX::IsEAIBatchModel() ? PRO_B_TRUE : PRO_B_FALSE;

		*output = op_args;
	}

	OTKX_EXCEPTION_HANDLER()

	return status;
}

extern "C" IPEMRMX_API ProError RMX_ProIsRPMFolder(ProArgument * inputs, ProArgument * *output)
{
	IPEMRMX_LOG_ENTER
	ProError status = PRO_TK_NO_ERROR;

	try
	{
		ProValueData data;
		ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_DIR, &data);
		wstring dir(data.v.w);

		bool isRPMDir = RMX_IsRPMFolder(dir.c_str());
		ProArgument* op_args;
		status = ProArrayAlloc(1, sizeof(ProArgument), 1, (ProArray*)&op_args);
		op_args[0].value.type = PRO_VALUE_TYPE_BOOLEAN;
		op_args[0].value.v.b = isRPMDir ? PRO_B_TRUE : PRO_B_FALSE;

		*output = op_args;
	}

	OTKX_EXCEPTION_HANDLER()

	return status;
}

extern "C" IPEMRMX_API ProError RMX_ProFixNxlExtInWorkspace(ProArgument * inputs, ProArgument * *output)
{
	IPEMRMX_LOG_ENTER
	
	IpemRMX::EnsureNxlExtInWorkspace(true);

	return PRO_TK_NO_ERROR;
}

extern "C" IPEMRMX_API ProError RMX_ProSetCacheFolder(ProArgument * inputs, ProArgument * *output)
{
	IPEMRMX_LOG_ENTER

	try
	{
		ProValueData data;
		ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_DIR, &data);
		wstring dir(data.v.w);

		if (!RMX_IsRPMFolder(dir.c_str()))
		{
			RMX_AddRPMDir(dir.c_str());
		}
	}

	OTKX_EXCEPTION_HANDLER()

	return PRO_TK_NO_ERROR;
}

extern "C" IPEMRMX_API ProError RMX_ProFixNxlExt(ProArgument * inputs, ProArgument * *output)
{
	IPEMRMX_LOG_ENTER

	ProValueData data;
	ProTkHlp::ProTk_ArgumentValueGet(inputs, IpemRMX::ARG_FILENAME, &data);
	wstring fileArg(data.v.w);
	if (fileArg.empty())
	{
		LOG_ERROR(L"IPEMRMX - Failed to call RMX_ProFixNxlExt. FILE_NAME argument not specified");
		return PRO_TK_NO_ERROR;
	}

	LOG_DEBUG_FMT(L"IPEMRMX - Checking and fixing nxl extension for %s", fileArg.c_str());
	RMX_EnsureNxlExtension(fileArg.c_str());

	return PRO_TK_NO_ERROR;
}
