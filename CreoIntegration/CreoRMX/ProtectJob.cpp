#include "ProtectJob.h"

#include <regex>

#include <pfcGlobal.h>
#include <pfcModel.h>

#include "..\common\CreoRMXLogger.h"
#include "OtkXModel.h"
#include "OtkXUtils.h"
#include <PathEx.h>
#include <RMXLogger.h>
#include <RMXUtils.h>

#include <LibRMX.h>
#include "OtkXSessionCache.h"
#include "ProtectController.h"
#include "UsageController.h"
#include "..\CreoRMXTestFwk\sources\XTestAPI.h"
#include "OtkXUIStrings.h"

using namespace std;
using namespace OtkXUtils;

#define PROTECTJOB_TYPENAME m_actionType == SOURCEACTION_SAVE? L"SAVE" : (m_actionType == SOURCEACTION_COPY ? L"COPY" : L"EXPORT")

//
// CProtectJob class
//
CProtectJob::CProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: m_pMdlDescr(pMdlDescr)
{
	Init(pMdlDescr);
}

CProtectJob::~CProtectJob()
{
}

std::wstring CProtectJob::GetInstanceName() const
{
	return m_instanceName;
}

wstring CProtectJob::GetFileName() const
{
	return m_fileName;
}

wstring CProtectJob::GetFileOrigin() const
{
	return m_fileOrigin;
}

wstring CProtectJob::GetFileDest() const
{
	return m_fileDest;
}

ProtectSourceAction CProtectJob::GetSourceActionType() const
{
	return m_actionType;
}

std::string CProtectJob::GetTags() const
{
	return m_tags;
}

void CProtectJob::SetFileOrigin(const wstring& fileOrigin)
{
	m_fileOrigin = fileOrigin;
}

void CProtectJob::SetFileDest(const wstring& destFilePath)
{
	m_fileDest = destFilePath;
}

void CProtectJob::SetTags(const string& tags)
{
	m_tags = tags;
}

void CProtectJob::SetActionType(ProtectSourceAction actionType)
{
	m_actionType = actionType;
}

bool CProtectJob::IsFileZeroKB()
{
	// Don't protect file if it's 0KB.
	HANDLE hFile = ::CreateFileW(m_fileDest.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER fileSize = { 0 };
		if (GetFileSizeEx(hFile, &fileSize) && fileSize.QuadPart == 0)
		{
			CloseHandle(hFile);
			return true;
		}
		CloseHandle(hFile);
	}

	return false;
}

bool CProtectJob::Execute()
{
	LOG_INFO(L"Starting new protection job...");

	Dump();

	// Don't protect file if it's 0KB.
	if (IsFileZeroKB())
	{
		LOG_ERROR(L"Ignore protection request. Invalid file with 0KB size.");
		return false;
	}
	
	bool succeeded = true;
	CPathEx newProtectedPath(m_fileDest.c_str());
	
	if (newProtectedPath.GetExtension().compare(NXL_FILE_EXT) != 0)
	{
		newProtectedPath += NXL_FILE_EXT;
	}
	
	// Re-protect the target file
#if defined(ENABLE_AUTOTEST) 
	// Test purpose only: Fill the original source file as tag
	// so that test framework can access source tags and validate them.
	XTestAPI::NotifyXTest(m_fileDest, m_fileOrigin, m_tags);
#endif // ENABLE_AUTOTEST
	// In case the target file is same to the original file, call RPMEditSaveFile
	// to reprotect the file with the latest modification in plain file
	// Note: 
	// 1. Do not call ProtectFile API to reprotect file because the modification in plain file will be lost.
	// 2: Do not call ReprotectFile API as it does not support rpm folder.
	// 3. If the original file is lost(Creo allows to delete file when file is openning), call ProtectFile instead.
	if (CPathEx::IsFile(m_fileOrigin) && (wcsicmp(m_fileOrigin.c_str(), m_fileDest.c_str()) == 0)
		&& RMX_IsProtectedFile(m_fileOrigin.c_str()))
	{
		if (!RMX_RPMEditSaveFile(m_fileDest.c_str(), nullptr, false, RMX_EditSaveMode_SaveAndExit))
		{
			succeeded = false;
		}
	}
	else if (!RMX_ProtectFile(m_fileDest.c_str(), m_tags.empty() ? nullptr : m_tags.c_str(), newProtectedPath.c_str()))
	{
		succeeded = false;
	}
	if (succeeded)
		LOG_INFO(L"Protection job finished");
	else
		LOG_ERROR(L"Protection job failed.File lost protection");

#if !defined(NDEBUG) || defined(ENABLE_AUTOTEST) 
	std::wofstream logFile(L"CreoRMX_Job.log", ios_base::out | ios_base::app);
	if (!logFile.fail())
	{
		static wstring processID = std::to_wstring((unsigned long)GetCurrentProcessId());
		const wstring& currTime = RMXUtils::getCurrentTime();
		wstring jobType(PROTECTJOB_TYPENAME);
		wstring msgPrefix = currTime + L"[" + processID + L"] ";
		logFile << msgPrefix.c_str() << L"Projection job information:" << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Job name: " << m_jobName.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Action type: " << jobType.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-File name: " << m_fileName.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Instance name: " << m_instanceName.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Nxl origin " << m_fileOrigin.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Nxl tags: " << RMXUtils::s2ws(m_tags).c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-File full path: " << m_fileDest.c_str() << endl;
		if (m_tags.empty())
			logFile << msgPrefix.c_str() << L"Error! Please investigate the empty tags." << endl;
		logFile << msgPrefix.c_str() << resultMsg.c_str() << endl;
		logFile.close();
	}
#endif // !NDEBUG

	return succeeded;
}

void CProtectJob::Dump()
{
	LOG_INFO(L"\t->Projection job information:");
	wstring type(PROTECTJOB_TYPENAME);
	LOG_INFO(L"\t\t-Job name: " << m_jobName.c_str());
	LOG_INFO(L"\t\t-Action type: " << type.c_str());
	LOG_INFO(L"\t\t-File name: " << m_fileName.c_str());
	LOG_INFO(L"\t\t-Instance name: " << m_instanceName.c_str());
	LOG_INFO(L"\t\t-Nxl origin: " << m_fileOrigin.c_str());
	LOG_INFO(L"\t\t-Nxl tags: " << RMXUtils::s2ws(m_tags).c_str());
	LOG_INFO(L"\t\t-File full path: " << m_fileDest.c_str());	
}

void CProtectJob::Init(pfcModelDescriptor_ptr pDescr)
{
	if (pDescr == nullptr)
		return;
	m_instanceName = CXS2W(pDescr->GetInstanceName());
	m_fileName = CXS2W(pDescr->GetFileName());
}

//
// CSaveProtectJob class
//
CSaveProtectJob::CSaveProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_actionType = SOURCEACTION_SAVE;
	m_jobName = L"SaveProtectJob";
}

ProtectJobPtr CSaveProtectJob::Create(pfcModelDescriptor_ptr pMdlDescr)
{
	pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pMdlDescr);
	if (pMdl == nullptr)
		return nullptr;

	// Only create the job in case of the file is already protected.
	// The goal is to track the newly copied file and ensure it's protected.
	COtkXModel xModel(pMdl);
	const wstring& nxlOrigin = OtkXSessionCacheHolder().FindNxlOrigin(xModel);
	if (!nxlOrigin.empty())
	{
		auto pJob = std::make_shared<CSaveProtectJob>(pMdlDescr);
		pJob->SetFileDest(xModel.GetOrigin());
		pJob->SetFileOrigin(nxlOrigin);
		pJob->SetTags(OtkXSessionCacheHolder().GetTags(nxlOrigin.c_str()));

		return pJob;
	}
	
	LOG_INFO_FMT(L"Skip protection control. Model(%s) to be saved not protected ", CXS2W(pMdl->GetInstanceName()));
	return nullptr;
}

//
// CCopyProtectJob class
//
CCopyProtectJob::CCopyProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_actionType = SOURCEACTION_COPY;
	m_jobName = L"CopyProtectJob";
}

ProtectJobPtr CCopyProtectJob::Create(pfcModelDescriptor_ptr pToDescr, const wstring& targetFile)
{
	pfcModel_ptr pToMdl = OtkX_GetModelFromDescr(pToDescr);
	if (pToMdl == nullptr)
		return nullptr;

	COtkXModel xModel(pToMdl);
	const wstring& nxlOrigin = OtkXSessionCacheHolder().FindNxlOrigin(xModel);
	if (!nxlOrigin.empty())
	{
		auto pJob = std::make_shared<CCopyProtectJob>(pToDescr);
		pJob->SetFileOrigin(nxlOrigin);
		pJob->SetFileDest(targetFile);
		pJob->SetTags(OtkXSessionCacheHolder().GetTags(nxlOrigin));

		return pJob;
	}

	LOG_INFO_FMT(L"Skip protection control. NXL_ORIGIN not found in the model(%s)", CXS2W(pToMdl->GetInstanceName()));
	return nullptr;
}

//
// CExportProtectJob class
//
static wstring g_exportFileNamePatterns[] = {
	wstring(L"(.*)(_asm_cpy_\\d{1}.igs)"),
	wstring(L"(.*)(_cpy_\\d{1}.igs)"),	
	wstring(L"(.*)(_asm.igs)"),
	wstring(L"(.*)(.igs)"),
	wstring(L"(.*)(_asm(.*).neu)"),
	wstring(L"(.*)(.neu)"),
	wstring(L"(.*)(_asm_cpy_\\d{1}.vda)"),
	wstring(L"(.*)(_cpy_\\d{1}.vda)"),
	wstring(L"(.*)(_asm.vda)"),
	wstring(L"(.*)(.vda)"),
	wstring(L"(.*)(_(asm|prt).wrl)"),
	wstring(L"(.*)(_(asm|prt).jt)"),
	wstring(L"(.*)(.jt)"),
	wstring(L"(.*)(.asm)"),
	wstring(L"(.*)(.prt)")
};

CExportProtectJob::CExportProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_actionType = SOURCEACTION_EXPORT;
	m_jobName = L"ExportProtectJob";
}

ProtectJobPtr CExportProtectJob::Create(pfcModelDescriptor_ptr pMdlDescr, const wstring& targetFile)
{
	if (pMdlDescr == nullptr)
		return nullptr;

	pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pMdlDescr);
	if (pMdl == nullptr)
		return nullptr;

	pfcModel_ptr pSrcMdl = nullptr;
	pfcModelType targetMdlType = pfcModelType_nil;
	// Simply apply same tags in case single part is exported.
	// Or export as shrinkwrap to single part or lightweight part
	if (pMdl->GetType() == pfcMDL_PART ||
		ProtectControllerHolder().IsWatcherRunning(PROTECT_WATCHER_SHRINKWRAP))
	{
		pSrcMdl = pMdl;
		targetMdlType = pfcMDL_PART;
	}
	else
	{
		OtkXFileNameData fNameData(targetFile.c_str());
		const wstring& fileName = fNameData.GetFileName();
		const wstring& fileExt = fNameData.GetExtension();

		if ((targetMdlType = OtkX_GetModelType(fileExt)) != pfcModelType_nil)
		{
			// Export as same model type

			// Find the model in session
			pSrcMdl = OtkX_GetModelFromFileName(fileName);
		}
		else if ((pMdl->GetType() == pfcMDL_DRAWING) && (wcsicmp(fileExt.c_str(), L"igs") == 0))
		{
			// Temporary solution for bug 60408 
			// For drawing file, only single igs can be exported.
			// No need to look up source file from dependencies in case any dependent objects 
			// TODO: Better design pattern for different file types 
			pSrcMdl = pMdl;
		}
		if (pSrcMdl == nullptr)
		{		
			for (const auto& p : g_exportFileNamePatterns)
			{
				std::wsmatch mResults;
				if (std::regex_match(fileName, mResults, std::wregex(p)) && mResults.size() > 2)
				{
					// Some export file type supports to export assembly with parts as multiple files.
					// RMX has to determine the origin file and apply respective tags to the part/sub-assembly files
					COtkXModel xMdl(pMdl);
					pSrcMdl = xMdl.FindDepModelByName(mResults[1].str());
					if (pSrcMdl != nullptr)
					{
						LOG_INFO_FMT(L"Dependent mode found: %s", mResults[1].str().c_str());
						break;
					}
				}
			}	
		}	

		if (pSrcMdl == nullptr)
		{
			// Simply apply same tags 
			pSrcMdl = pMdl;
		}
	}

	bool needToProtect = false;
	// recursive - Only recursively grab tags from dependencies in case file is export to other format
	bool recursive = (targetMdlType != pSrcMdl->GetType());

	string tags;
	ProtectControllerHolder().GetExportTags(pSrcMdl, recursive, tags, needToProtect);
	wstring srcFile = CXS2W(pSrcMdl->GetOrigin());
	if (needToProtect)
	{
		auto pJob = std::make_shared<CExportProtectJob>(pMdlDescr);
		pJob->SetFileDest(targetFile);
		pJob->SetFileOrigin(srcFile);
		pJob->SetTags(tags);
		
		return pJob;

	}
	LOG_INFO_FMT(L"Skip protection control. Exported model is not protected:\n\t- Source: '%s'\n\t- Export: '%s'", srcFile.c_str(), targetFile.c_str());

	return nullptr;
}

CJTCurrentProtectJob::CJTCurrentProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_actionType = SOURCEACTION_EXPORT;
	m_jobName = L"JTCurrentProtectJob";
}

ProtectJobPtr CJTCurrentProtectJob::Create(pfcModelDescriptor_ptr pMdlDescr, const wstring& jtFile)
{
	if (pMdlDescr == nullptr)
		return nullptr;

	pfcModel_ptr pMdl = OtkX_GetModelFromDescr(pMdlDescr);
	if (pMdl == nullptr)
		return nullptr;

	pfcModel_ptr pSrcMdl = nullptr;
	// Try to find out the source object of JT file from the 
	// assembly dependent objects 
	if (pMdl->GetType() == pfcMDL_ASSEMBLY)
	{
		OtkXFileNameData jtFNameData(jtFile.c_str());
		const wstring& fileName = jtFNameData.GetFileName();
		const wstring& fileExt = jtFNameData.GetExtension();
		
		static wchar_t* jtNamePatterns[] = { L"(.*)(_(asm|prt).jt)", L"(.*)(.jt)" };
		for (const auto& p : jtNamePatterns)
		{
			std::wsmatch mResults;
			if (std::regex_match(fileName, mResults, std::wregex(p)) && mResults.size() > 2)
			{
				// Some export file type supports to export assembly with parts as multiple files.
				// RMX has to determine the origin file and apply respective tags to the part/sub-assembly files
				COtkXModel xMdl(pMdl);
				pSrcMdl = xMdl.FindDepModelByName(mResults[1].str().c_str());
				if (pSrcMdl != nullptr)
				{
					LOG_INFO_FMT(L"Dependent mode found: %s", mResults[1].str().c_str());
					break;
				}

				wstring fixedName = mResults[1];
				std::replace(fixedName.begin(), fixedName.end(), L'_', L'-');
				pSrcMdl = xMdl.FindDepModelByName(fixedName.c_str());
				if (pSrcMdl != nullptr)
				{
					LOG_INFO_FMT(L"Dependent mode found: %s", fixedName.c_str());
					break;
				}
			}
		}
	}
	// Simply treat specified model as source CAD object of JT
	if (pSrcMdl == nullptr)
		pSrcMdl = pMdl;
	
	COtkXModel xSrcMdl(pSrcMdl);
	wstring srcFile = xSrcMdl.GetOrigin();
	// "srcFile(i.e.:pfcModel::GetOrigin())" may be empty if model is newly created/minorred in session memory 
	// but has never saved as volume file yet.
	// Use model name to dedicate the source object.
	srcFile = srcFile.empty() ? CXS2W(pSrcMdl->GetInstanceName()) : srcFile.c_str();

	string tags;
	bool needToProtect;
	CProtectController::GetInstance().GetExportTags(pSrcMdl, true, tags, needToProtect);
	if (needToProtect)
	{
		auto pJob = std::make_shared<CJTCurrentProtectJob>(pSrcMdl->GetDescr());
		pJob->SetFileDest(jtFile);
		pJob->SetFileOrigin(srcFile);
		pJob->SetTags(OtkXSessionCacheHolder().GetTags(srcFile.c_str()));

		return pJob;
	}
	
	LOG_INFO_FMT(L"Skip protection control. Exported model is not protected:\n\t- Source: '%s'\n\t- Export: '%s'", srcFile.c_str(), jtFile.c_str());
	return nullptr;
}

CJTBatchProtectJob::CJTBatchProtectJob()
	: CProtectJob(nullptr)
{
	m_actionType = SOURCEACTION_EXPORT;
	m_jobName = L"JTBatchProtectJob";
}

//
// CJTBatchProtectJob class
//
ProtectJobPtr CJTBatchProtectJob::Create(const COtkXCacheModel& cadFile, const wstring& jtFile)
{
	auto pJob = std::make_shared<CJTBatchProtectJob>();
	pJob->SetFileDest(jtFile);
	pJob->SetFileOrigin(cadFile.GetOrigin());

	string combinedTags;
	std::vector<std::pair<std::wstring, std::string> > vecTagsWithDep;
	cadFile.GetTagsWithDep(vecTagsWithDep);
	for (const auto& tags : vecTagsWithDep)
	{
		combinedTags += tags.second;
	}
	pJob->SetTags(combinedTags);

	return pJob;
}

CAuxiImageFileProtectJob::CAuxiImageFileProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_jobName = L"AuxiImageFileProtectJob";
}

//
// CAuxiImageFileProtectJob class
//
ProtectJobPtr CAuxiImageFileProtectJob::Create(ProtectJobPtr pMdlJob, const wstring& imageFilePath)
{
	if (pMdlJob == nullptr || imageFilePath.empty())
		return nullptr;

	if (!CPathEx::IsFile(imageFilePath))
	{
		LOG_INFO_FMT(L"Skip protection control for auxiliary image file(File not found): '%s'", imageFilePath.c_str());
		return  nullptr;
	}
	auto pAuxiFileJob = std::make_shared<CAuxiImageFileProtectJob>(pMdlJob->GetModelDescr());
	pAuxiFileJob->SetActionType(pMdlJob->GetSourceActionType());
	pAuxiFileJob->SetFileOrigin(pMdlJob->GetFileDest());
	pAuxiFileJob->SetFileDest(imageFilePath);
	pAuxiFileJob->SetTags(pMdlJob->GetTags());
	
	return pAuxiFileJob;
}

std::wstring CAuxiImageFileProtectJob::BuildImageFilePath(const wstring & mdlFilePath, const wstring& imageExt)
{
	if (imageExt.empty())
		return wstring();

	wstring imageFilePath;
	// Protect save bitmap file (Only for assembly and part)
	OtkXFileNameData fNameData(mdlFilePath);
	if (wcsicmp(fNameData.GetExtension().c_str(), L"asm") == 0 || wcsicmp(fNameData.GetExtension().c_str(), L"prt") == 0)
	{
		// Match file name like [MdlFileName][prt|asm].[bitmapExt]. e.g.: test.part -> testprt.jpg
		wstring bitmapFName = fNameData.GetBaseName() + fNameData.GetExtension();
		OtkXFileNameData bitmapFNameData(fNameData.GetDirectoryPath(), bitmapFName, imageExt, -1);
		imageFilePath = bitmapFNameData.GetFullPath();
	}

	return imageFilePath;
}

CTCAuxiFileProtectJob::CTCAuxiFileProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_jobName = L"TCAuxiFileProtectJob";
}

ProtectJobPtr CTCAuxiFileProtectJob::Create(const wstring & auxiFilePath)
{
	if (!CPathEx::IsFile(auxiFilePath))
	{
		LOG_INFO_FMT(L"Skip protection control for file(File not found): '%s'", auxiFilePath.c_str());
		return  nullptr;
	}

	pfcModel_ptr pSrcMdl = nullptr;
	OtkXFileNameData nameData(auxiFilePath);
	std::wsmatch mResults;
	static wstring s_namePattern(wstring(L"(.*)((asm|prt|d).jpg)"));
	wstring fileName(nameData.GetFileName());
	RMXUtils::ToLower<wchar_t>(fileName);
	if (std::regex_match(fileName, mResults, std::wregex(s_namePattern)) && mResults.size() > 2)
	{
		// Some export file type supports to export assembly with parts as multiple files.
		// RMX has to determine the origin file and apply respective tags to the part/sub-assembly files
		string searchName = RMXUtils::ws2s(mResults[1].str());
		auto pSess = pfcGetProESession();
		pSrcMdl = pSess->GetModel(CA2XS(searchName.c_str()), pfcMDL_PART);
		if (pSrcMdl == nullptr)
			pSrcMdl = pSess->GetModel(CA2XS(searchName.c_str()), pfcMDL_ASSEMBLY);
		if (pSrcMdl == nullptr)
			pSrcMdl = pSess->GetModel(CA2XS(searchName.c_str()), pfcMDL_DRAWING);
	}
	else
	{
		LOG_INFO_FMT(L"Skip protection control. Invalid auxiliary file: '%s'", auxiFilePath.c_str());
		return  nullptr;
	}
	if (pSrcMdl == nullptr)
	{
		LOG_INFO_FMT(L"Skip protection control. Source model not found: '%s'", auxiFilePath.c_str());
		return nullptr;
	}

	wstring srcFile = CXS2W(pSrcMdl->GetOrigin());
	if (!CProtectController::NeedToProtect(pSrcMdl))
	{
		LOG_INFO_FMT(L"Skip protection control. Source model not protected(Source: '%s', Auxiliary file: '%s')",
			srcFile.empty() ? CXS2W(pSrcMdl->GetInstanceName()) : srcFile.c_str(), auxiFilePath.c_str());
		//CProtectController::GetInstance().RemoveNxlParam(pSrcMdl);
		return nullptr;
	}

	auto pJob = std::make_shared<CTCAuxiFileProtectJob>(pSrcMdl->GetDescr());
	pJob->SetActionType(SOURCEACTION_SAVE);
	pJob->SetFileDest(auxiFilePath);
	pJob->SetFileOrigin(srcFile.c_str());
	pJob->SetTags(OtkXSessionCacheHolder().GetTags(srcFile.c_str()));

	return pJob;
}

CMotionEnvlpProtectJob::CMotionEnvlpProtectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_actionType = SOURCEACTION_EXPORT;
	m_jobName = L"MotionEnvlpProtectJob";
}

ProtectJobPtr CMotionEnvlpProtectJob::Create(pfcModelDescriptor_ptr pMdlDescr, const wstring & targetFile)
{
	if (pMdlDescr == nullptr)
		return nullptr;

	pfcModel_ptr pSrcMdl = OtkX_GetModelFromDescr(pMdlDescr);
	if (pSrcMdl == nullptr)
		return nullptr;

	string tags;
	bool needToProtect = false;
	ProtectControllerHolder().GetExportTags(pSrcMdl, true, tags, needToProtect);
	wstring srcFile = CXS2W(pSrcMdl->GetOrigin());
	// "srcFile(i.e.:pfcModel::GetOrigin())" may be empty if model is newly created/minorred in session memory 
	// but has never saved as volume file yet.
	// Use model name to dedicate the source object.
	srcFile = srcFile.empty() ? CXS2W(pSrcMdl->GetInstanceName()) : srcFile.c_str();
	if (needToProtect)
	{
		// Usage control: For Playback->Export motion envelop, check if the permission after the operation is executed, because 
		// before the command executed, target source models will be specified during the operation.
		COtkXModel xSrcMdl(pSrcMdl);
		auto handleError = [&](DWORD right) {
			CProtectController::GetInstance().NotifyActionDeny(right, srcFile, targetFile);
			if (!CPathEx::RemoveFile(targetFile))
			{
				LOG_ERROR_FMT(L"Cannot delete exported file: %s", targetFile.c_str());
			}
			else
			{
				LOG_INFO_FMT(L"Exported file deleted. %s", targetFile.c_str());
			}
		};
		auto usageRight = CUsageController::CheckRight(xSrcMdl, RMX_RIGHT_EXPORT);
		if (usageRight == Usage_Right_Deny_FileNotExists)
		{
			LOG_ERROR_STR(L"Operation Denied. The protected file for this model no longer exists in disk.");
			handleError(RMX_RIGHT_NONE);
			return nullptr;
		}
		if (usageRight == Usage_Right_Deny)
		{
			LOG_ERROR_FMT(L"Operation Denied. You do not have 'Save As' permission to export the source file(%s). Exported file(%s) will be deleted", srcFile.c_str(), targetFile.c_str());
			handleError(RMX_RIGHT_EXPORT);
			return nullptr;
		}
		if (usageRight == Usage_Right_DenyEdit)
		{
			LOG_ERROR_FMT(L"Operation Denied. You do not have 'Edit' permission to change the source file(%s). Exported file(%s) will be deleted", srcFile.c_str(), targetFile.c_str());
			handleError(RMX_RIGHT_EDIT);
			return nullptr;
		}

		std::set<wstring> denyFiles;
		auto handleDepError = [&](DWORD right) {
			for (const auto& f : denyFiles)
			{
				LOG_ERROR_FMT(L"\t%s", f.c_str());
				CProtectController::GetInstance().NotifyActionDeny(right, f.c_str(), targetFile);
			}
			if (!CPathEx::RemoveFile(targetFile))
			{
				LOG_ERROR_FMT(L"Cannot delete exported file: %s", targetFile.c_str());
			}
			else
			{
				LOG_INFO_FMT(L"Exported file deleted. %s", targetFile.c_str());
			}
		};
		usageRight = CUsageController::CheckDepRight(xSrcMdl, RMX_RIGHT_EXPORT, denyFiles);
		if (usageRight == Usage_Right_Deny_FileNotExists)
		{
			LOG_ERROR_STR(L"Operation Denied. The following protected files no longer exist in disk. Exported file(%s) will be deleted");
			handleDepError(RMX_RIGHT_NONE);
			return nullptr;
		}
		if (usageRight == Usage_Right_Deny)
		{
			LOG_ERROR_FMT(L"Operation Denied. You do not have 'Save As' permission to export the following dependent file(s). Exported file(%s) will be deleted", targetFile.c_str());
			handleDepError(RMX_RIGHT_EXPORT);
			return nullptr;
		}

		if (usageRight == Usage_Right_DenyEdit)
		{
			LOG_ERROR_FMT(L"Operation Denied. You do not have 'Edit' permission to change the following dependent file(s). Exported file(%s) will be deleted", targetFile.c_str());
			handleDepError(RMX_RIGHT_EDIT);
			return nullptr;
		}

		auto pJob = std::make_shared<CMotionEnvlpProtectJob>(pMdlDescr);
		pJob->SetFileDest(targetFile);
		pJob->SetFileOrigin(srcFile);
		pJob->SetTags(tags);

		return pJob;

	}
	
	LOG_INFO_FMT(L"Skip protection control. Exported model is not protected:\n\t- Source: '%s'\n\t- Export: '%s'", srcFile.c_str(), targetFile.c_str());
	
	return nullptr;
}

CFileCopyProtectJob::CFileCopyProtectJob()
	: CProtectJob(nullptr)
{
	m_actionType = SOURCEACTION_EXPORT;
	m_jobName = L"FileCopyProtectJob";
}

ProtectJobPtr CFileCopyProtectJob::Create(const wstring & sourceFile, const wstring & targetFile)
{
	if (!RMX_IsProtectedFile(sourceFile.c_str()))
		return nullptr;

	auto pJob = std::make_shared<CFileCopyProtectJob>();
	pJob->SetFileDest(targetFile);
	pJob->SetFileOrigin(sourceFile);
	char* szTags = nullptr;
	RMX_GetTags(sourceFile.c_str(), &szTags);
	if (szTags != nullptr)
	{
		pJob->SetTags(szTags);
		RMX_ReleaseArray(szTags);
	}
	
	return pJob;
}

CCommandProjectJob::CCommandProjectJob(pfcModelDescriptor_ptr pMdlDescr)
	: CProtectJob(pMdlDescr)
{
	m_actionType = SOURCEACTION_SAVE;
	m_jobName = L"CommandProtectJob";
}

ProtectJobPtr CCommandProjectJob::Create(pfcModel_ptr pMdl, const string& tags)
{
	// No need to do sanity check here, as it's done before command triggered.
	auto pJob = std::make_shared<CCommandProjectJob>(pMdl->GetDescr());
	COtkXModel xMdl(pMdl);
	pJob->SetFileDest(xMdl.GetOrigin());
	pJob->SetFileOrigin(xMdl.GetOrigin());
	pJob->SetTags(tags);

	return pJob;
}

bool CCommandProjectJob::Execute()
{
	LOG_INFO(L"Starting new protection job...");

	Dump();

	// Don't protect file if it's 0KB.
	if (IsFileZeroKB())
	{
		LOG_ERROR(L"Ignore protection request. Invalid file with 0KB size.");
		return false;
	}

	bool succeeded = true;
	CPathEx newProtectedPath(m_fileDest.c_str());
	if (newProtectedPath.GetExtension().compare(NXL_FILE_EXT) != 0)
	{
		newProtectedPath += NXL_FILE_EXT;
	}

		// Re-protect the target file
#if defined(ENABLE_AUTOTEST) 
	// Test purpose only: Fill the original source file as tag
	// so that test framework can access source tags and validate them.
	XTestAPI::NotifyXTest(m_fileDest, m_fileOrigin, m_tags);
#endif // ENABLE_AUTOTEST
		
	if (!RMX_ProtectFile(m_fileDest.c_str(), m_tags.empty() ? nullptr : m_tags.c_str(), newProtectedPath.c_str()))
	{
		LOG_ERROR(L"Protection job failed. File lost protection");
		succeeded = false;
	}
	else
	{
		LOG_INFO(L"Protection job finished");

		// Set target dir as rpm folder if it's not existing
		// These rpm dir will not be unset when RMX unloaded.
		CPathEx filePathEx(m_fileDest.c_str());
		if (!RMX_AddRPMDir(filePathEx.GetParentPath().c_str()))
		{
			xstring msg(IDS_WARN_SETRPMDIR_AFTER_PROTECT);
			LOG_ERROR(RMXUtils::s2ws(msg).c_str());
		}
	}

#if !defined(NDEBUG) || defined(ENABLE_AUTOTEST) 
	std::wofstream logFile(L"CreoRMX_Job.log", ios_base::out | ios_base::app);
	if (!logFile.fail())
	{
		static wstring processID = std::to_wstring((unsigned long)GetCurrentProcessId());
		const wstring& currTime = RMXUtils::getCurrentTime();
		wstring jobType(PROTECTJOB_TYPENAME);
		wstring msgPrefix = currTime + L"[" + processID + L"] ";
		logFile << msgPrefix.c_str() << L"Projection job information:" << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Job name: " << m_jobName.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Action type: " << jobType.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-File name: " << m_fileName.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Instance name: " << m_instanceName.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Nxl origin " << m_fileOrigin.c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-Nxl tags: " << RMXUtils::s2ws(m_tags).c_str() << endl;
		logFile << msgPrefix.c_str() << L"\t\t-File full path: " << m_fileDest.c_str() << endl;
		if (m_tags.empty())
			logFile << msgPrefix.c_str() << L"Error! Please investigate the empty tags." << endl;
		logFile.close();
	}
#endif // !NDEBUG

	return succeeded;
}
