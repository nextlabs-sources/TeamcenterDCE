#include "stdafx.h"
#include "FileInfoCache.h"
#include <unordered_map>
#include "NXDRMIntegration.h"

static std::unordered_map<std::wstring, FileInfoCache> s_cache;
const FileInfoCache *FileInfoCache::Add(const wchar_t * file)
{
	RPMSession *rpmSession = NXL::NX::DRM::GetRPMSession();
	bool isProtected;
	NxlMetadataCollection tags;
	if (rpmSession->ReadFileTags(file, &isProtected, tags) == 0
		&& isProtected)
	{
		s_cache.erase(file);
		s_cache.insert(std::make_pair(file, FileInfoCache(file, isProtected, tags)));
			DBGLOG("'%s':cache added(%d)", file, s_cache.size());
		return Find(file);
	}
	return nullptr;
}

const FileInfoCache *FileInfoCache::Find(const wchar_t * file)
{
	// TODO: insert return statement here
	auto item = s_cache.find(file);
	if (s_cache.end() != item) {
		return &(item->second);
	}
	DBGLOG("'%s':cache not found", file);
	return nullptr;
}
