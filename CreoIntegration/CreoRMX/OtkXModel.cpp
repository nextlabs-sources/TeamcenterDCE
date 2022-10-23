#include "OtkXModel.h"

#include <algorithm>

#include "OtkXTypes.h"
#include <PathEx.h>
#include <RMXUtils.h>
#include <LibRMX.h>

using namespace std;
using namespace OtkXUtils;

static int OTKX_MAX_PATH = 340;
static int OTKX_MAX_STRING_PARAM_LEN = 80;
static int OTKX_MAX_PATH_PARAM_NUM = 5;

COtkXModel::COtkXModel(pfcModel_ptr pMdl)
	: m_pMdl(pMdl)
{
	if (m_pMdl)
	{
		m_origin = CXS2W(m_pMdl->GetOrigin());
		pfcModelDescriptor_ptr pDescr = pMdl->GetDescr();
		if (pDescr)
		{
			std::wstring name = CXS2W(pDescr->GetInstanceName());
			std::wstring type = CXS2W(pDescr->GetExtension());
			if (!name.empty() && name.find(L"#") != std::wstring::npos)
			{
				std::wstring generic = pDescr->GetGenericName();
				if (!generic.empty())
					name = generic;
			}

			OTKX_MAKE_ID(name, type, m_id);
		}
	}
}

//void COtkXModel::AddNxlParam(const std::string& nxlOrigin)
//{
//	if (m_pMdl == nullptr || nxlOrigin.empty())
//		return;
//
//	RemoveNxlParam();
//
//	size_t len = nxlOrigin.length();
//	int n = int(len / OTKX_MAX_STRING_PARAM_LEN + 1);
//	size_t startPos = 0, endPos = 0;
//	for (int i = 0; i < n; i++)
//	{
//		endPos = startPos + OTKX_MAX_STRING_PARAM_LEN - 1;
//		if (endPos > len || endPos == len)
//			endPos = len ;
//		xstring paramName(OTKX_PARAM_NXL_ORIGIN);
//		paramName += std::to_string(i).c_str();
//		string parameValue = nxlOrigin.substr(startPos, endPos - startPos + 1);
//		if (parameValue.length() > OTKX_MAX_STRING_PARAM_LEN)
//			LOG_ERROR(L"Too long parameter value for NXL_ORIGIN");
//		OtkX_SetStringParamValue(m_pMdl, paramName, CA2XS(parameValue.c_str()));
//		startPos = endPos + 1;
//		if (startPos > len || startPos == len)
//			break;
//	}
//}

//void COtkXModel::RemoveNxlParam()
//{
//	if (m_pMdl == nullptr)
//		return;
//
//	for (int i = 0; i < OTKX_MAX_PATH_PARAM_NUM; i++)
//	{
//		xstring paramName(OTKX_PARAM_NXL_ORIGIN);
//		paramName += std::to_string(i).c_str();
//		OtkX_RemoveParam(m_pMdl, paramName);
//	}
//}

pfcModel_ptr COtkXModel::FindDepModelByName(const std::wstring & searchFileName) const
{
	pfcModel_ptr pDepModel = nullptr;
	TraverseDependencies([&](COtkXModel& xDepMdl) -> bool {
		auto pMdl = xDepMdl.GetModel();
		wstring mdlName = CXS2W(pMdl->GetInstanceName());
		if (wcsicmp(mdlName.c_str(), searchFileName.c_str()) == 0)
		{
			pDepModel = pMdl;
			return true;
		}

		return false;
	});

	return pDepModel;
}

bool COtkXModel::IsReadOnly() const
{
	if (IsValid())
	{
		DWORD attr = GetFileAttributes(m_origin.c_str());
		if ((INVALID_FILE_ATTRIBUTES != attr) && (attr & FILE_ATTRIBUTE_READONLY))
		{
			return true;
		}
	}
	
	return false;
}

bool COtkXModel::FileExists() const
{
	return CPathEx::IsFile(m_origin);
}

bool COtkXModel::IsSaveAllowed() const
{
	if (m_pMdl)
		return m_pMdl->CheckIsSaveAllowed(false);

	return false;
}

//std::wstring COtkXModel::GetNxlOrigin()
//{
//	wstring retNxlOrigin;
//	if (IsValid())
//	{	
//		for (int i = 0; i < OTKX_MAX_PATH_PARAM_NUM; i++)
//		{
//			xstring paramName(OTKX_PARAM_NXL_ORIGIN);
//			paramName += std::to_string(i).c_str();
//			xstring paramNxlOrigin = OtkX_GetStringParamValue(m_pMdl, paramName);
//			if (!paramNxlOrigin.IsEmpty())
//				retNxlOrigin += CXS2W(paramNxlOrigin);
//		}	
//	}
//
//	return retNxlOrigin;
//}

bool COtkXModel::IsProtected() const
{
	if (!IsValid())
		return false;

	return RMX_IsProtectedFile(m_origin.c_str());
}

bool COtkXModel::IsDepProtected() const
{
	bool IsAnyDepProtected = false;
	TraverseDependencies([&](COtkXModel& xDepMdl) -> bool {
		if (xDepMdl.IsProtected())
		{
			IsAnyDepProtected = true;
			return true;
		}
		return false;
	});

	return IsAnyDepProtected;
}

//bool COtkXModel::HasNxlParam()
//{
//	const wstring& nxlOrigin = GetNxlOrigin();
//	return nxlOrigin.empty() ? false : true;
//}

bool COtkXModel::CheckRights(DWORD rights) const
{
	if (!IsProtected())
		return true;

	return RMX_CheckRights(m_origin.c_str(), rights, true);
}

bool COtkXModel::CheckDepRights(DWORD rights, std::set<std::wstring>& denyFiles) const
{
	TraverseDependencies([&](const COtkXModel& xDepMdl) -> bool {
		if (!xDepMdl.CheckRights(rights))
		{
			denyFiles.insert(xDepMdl.GetOrigin());
		}
		return false;
	});

	return (denyFiles.size() == 0);
}

std::string COtkXModel::GetTags() const
{
	string retTags;
	//if (IsProtected())
	//{
		// TODO: For read-only file, there's not nxl_origin param 
		// added. Access tags from model origin.
		// In next releases, rework to remove nxl_origin param because 
		// it always causes model modified and affect save behavior.
		wstring nxlOrigin = GetOrigin();
		if (!nxlOrigin.empty())
		{
			char* szTags = nullptr;
			RMX_GetTags(nxlOrigin.c_str(), &szTags);
			if (szTags != nullptr)
			{
				retTags = szTags;
				RMX_ReleaseArray((void*)szTags);
			}
		}
	//}
	return retTags;
}

void COtkXModel::GetTagsWithDep(std::vector<std::pair<std::wstring, std::string> >& out_tags) const
{
	const string& selfTags = GetTags();
	if (!selfTags.empty())
		out_tags.push_back(std::make_pair(m_origin, selfTags));

	TraverseDependencies([&]( const COtkXModel& xDepMdl) -> bool {
		const wstring& origin = xDepMdl.GetOrigin();
		if (!origin.empty())
		{
			auto it = std::find_if(out_tags.cbegin(), out_tags.cend(), [&](const std::pair<std::wstring, std::string>& val) {
				return wcsicmp(origin.c_str(), val.first.c_str()) == 0;
			});
			if (it == out_tags.cend())
			{
				const string& selfTags = xDepMdl.GetTags();
				if (!selfTags.empty())
					out_tags.push_back(std::make_pair(origin, selfTags));
			}
		}

		return false;
	});
}

void COtkXModel::GetDepNxlFiles(std::set<std::wstring>& out_depFiles) const
{
	TraverseDependencies([&](COtkXModel& xDepMdl) -> bool {
		if (xDepMdl.IsProtected())
		{
			out_depFiles.insert(xDepMdl.GetOrigin());
		}

		return false;
	});
}

COtkXCacheModel::COtkXCacheModel(pfcModel_ptr pMdl)
{
	COtkXModel xMdl(pMdl);
	m_id = xMdl.GetId();
	m_origin = xMdl.GetOrigin();
	m_isProtected = xMdl.IsProtected();
	m_tags = xMdl.GetTags();
	m_modified = pMdl->GetIsModified();

	xMdl.TraverseDependencies([&](COtkXModel& xDepMdl) -> bool {
		const wstring& origin = xDepMdl.GetOrigin();
		if (!origin.empty())
		{
			auto depNxlItr = m_depNxlFiles.find(origin);
			if (depNxlItr == m_depNxlFiles.end())
			{
				if (xDepMdl.IsProtected())
					m_depNxlFiles[origin] = xDepMdl.GetTags();
			}
			m_deps.insert(origin);
		}

		return false;
	});
}

bool COtkXCacheModel::CheckRights(DWORD rights) const
{
	if (!m_isProtected) return true;

	return RMX_CheckRights(m_origin.c_str(), rights, true);
}

bool COtkXCacheModel::CheckDepRights(DWORD rights, std::set<std::wstring>& denyFiles) const
{
	for (const auto& depNxl : m_depNxlFiles)
	{
		if (!RMX_CheckRights(depNxl.first.c_str(), rights, true))
			denyFiles.insert(depNxl.first);
	}

	return (denyFiles.size() == 0);
}

std::string COtkXCacheModel::GetTags() const
{
	return m_tags;
}

void COtkXCacheModel::GetTagsWithDep(std::vector<std::pair<std::wstring, std::string>>& out_tags) const
{
	const string& selfTags = GetTags();
	if (!selfTags.empty())
		out_tags.push_back(std::make_pair(m_origin, selfTags));

	for (const auto& nxlFile : m_depNxlFiles)
	{
		if (!nxlFile.second.empty())
			out_tags.push_back(std::make_pair(nxlFile.first, nxlFile.second));
	}
}

