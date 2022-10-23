#include "OtkXSessionCache.h"

#include <RMXUtils.h>

#include "OtkXModel.h"

#include <LibRMX.h>

#include "..\CreoRMXTestFwk\sources\XTestAPI.h"

using namespace std;
using namespace OtkXUtils;

#pragma region COtkXFile class


COtkXFile::COtkXFile(const std::wstring & fileName)
{
	m_nameData = OtkXFileNameData(fileName);
	OTKX_MAKE_ID(m_nameData.GetBaseName(), m_nameData.GetExtension(), m_id);
	m_mdlType = OtkX_GetModelType(m_nameData.GetExtension());
}

bool COtkXFile::operator == (const COtkXFile& other) const
{
	return (wcsicmp(m_nameData.GetFullPath().c_str(), other.GetFullPath().c_str()) == 0);
}

bool COtkXFile::operator == (const std::wstring& filePath) const
{
	return (wcsicmp(m_nameData.GetFullPath().c_str(), filePath.c_str()) == 0);
}

bool COtkXFile::IsVersionFile(const COtkXFile & other)
{
	return m_nameData.IsSameVersionFile(other.m_nameData);
}

#pragma endregion COtkXFile class


void COtkXSessionCache::Start()
{
	LOG_INFO(L"OtkXSessionCache intialized");
}

void COtkXSessionCache::Stop()
{
	m_nxlModelCache.clear();
	m_nxlFileTagCache.clear();
	m_dirtyToReloadNxl.clear();
	LOG_INFO(L"OtkXSessionCache erased");
}

bool COtkXSessionCache::IsNxlModel(const COtkXModel & xMdl) const
{
	return (m_nxlModelCache.find(xMdl.GetId()) != m_nxlModelCache.end());
}

bool COtkXSessionCache::IsNxlFile(const wstring & filePath) const
{
	auto foundFile = std::find_if(m_nxlModelCache.begin(), m_nxlModelCache.end(), [&filePath](const std::pair<TModelId, COtkXFile>& elem) {
		return (elem.second == filePath);
	});
	
	return (foundFile != m_nxlModelCache.end());
}

bool COtkXSessionCache::HasNxlDepModel(const COtkXModel& xMdl) const
{
	bool isAnyDep = false;
	xMdl.TraverseDependencies([&](const COtkXModel& xDepMdl) -> bool {	
		if (IsNxlModel(xDepMdl))
		{
			isAnyDep = true;
			return true;
		}
		return false;
	});

	return isAnyDep;
}

void COtkXSessionCache::EraseNxlModel(const COtkXModel & xMdl)
{
	if (xMdl.GetId().empty()) return;

	LOG_DEBUG_FMT(L"Erase Nxl model from cache: %s", xMdl.GetId().c_str());
	auto oldElem = m_nxlModelCache.find(xMdl.GetId());
	if (oldElem != m_nxlModelCache.end())
	{
		m_nxlFileTagCache.erase(oldElem->second.GetFullPath());
		m_nxlModelCache.erase(oldElem);
		m_dirtyToReloadNxl.erase(xMdl.GetId());
	}
}

void COtkXSessionCache::AddNxlModel(const TModelId & id, const wstring & filePath, bool rpmReadTags /*= true*/)
{
	if (id.empty()) return;

	LOG_DEBUG_FMT(L"Add Nxl model to cache: %s : %s", id.c_str(), filePath.c_str());
	m_nxlModelCache[id] = COtkXFile(filePath);
	if (rpmReadTags)
	{
		// Extract tags which will be used when protecting the file.
		char* szTags = nullptr;
		RMX_GetTags(filePath.c_str(), &szTags);
		if (szTags != nullptr)
		{
			SaveTags(filePath, string(szTags));
			RMX_ReleaseArray((void*)szTags);
		}
	}

	m_dirtyToReloadNxl.erase(id);
}

void COtkXSessionCache::RenameNxlModel(const COtkXModel & oldXMdl, const COtkXModel & newXMdl)
{
	LOG_DEBUG_FMT(L"Rename Nxl model in cache: %s to %s ", oldXMdl.GetId().c_str(), newXMdl.GetId().c_str());
	auto oldElem = m_nxlModelCache.find(oldXMdl.GetId());
	if (oldElem != m_nxlModelCache.end())
	{
		AddNxlModel(newXMdl.GetId(), oldElem->second.GetFullPath());
		m_nxlModelCache.erase(oldElem);
		if (IsDirtyToReload(oldXMdl.GetId()))
		{
			m_dirtyToReloadNxl.erase(oldXMdl.GetId());
		}
	}
}

void COtkXSessionCache::CopyNxlModel(const COtkXModel & srcXMdl, const COtkXModel & newXMdl)
{
	LOG_DEBUG_FMT(L"Copy Nxl model in cache: %s to %s ", srcXMdl.GetId().c_str(), newXMdl.GetId().c_str());
	auto oldElem = m_nxlModelCache.find(srcXMdl.GetId());
	if (oldElem != m_nxlModelCache.end())
	{
		AddNxlModel(newXMdl.GetId(), oldElem->second.GetFullPath(), false);
	}
}

std::wstring COtkXSessionCache::FindNxlOrigin(const COtkXModel & xMdl)
{
#if	defined(ENABLE_AUTOTEST)
	std::map<wstring, wstring> nxlMlds;
	for (const auto& m : m_nxlModelCache)
	{
		nxlMlds[m.first] = m.second.GetFullPath();
	}
	XTestAPI::DumpCachedNxlModel(nxlMlds);
#endif
	auto foundElem = m_nxlModelCache.find(xMdl.GetId());
	if (foundElem != m_nxlModelCache.end())
		return foundElem->second.GetFullPath();

	return wstring();
}

void COtkXSessionCache::AddDirtyNxlToReload(const TModelId& id)
{
	LOG_INFO(L"Dirty nxl added to cache: " << id.c_str());
	m_dirtyToReloadNxl.insert(id);
}

void COtkXSessionCache::SaveTags(const std::wstring& filePath, const string& tags)
{
	m_nxlFileTagCache[filePath] = tags;
}

std::string COtkXSessionCache::GetTags(const std::wstring & filePath)
{
	auto elem = m_nxlFileTagCache.find(filePath);
	if (elem != m_nxlFileTagCache.end())
		return elem->second;

	return string();
}
