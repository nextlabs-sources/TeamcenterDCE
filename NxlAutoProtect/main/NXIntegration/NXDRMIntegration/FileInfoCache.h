#pragma once
#include <NxlUtils/NxlPath.hpp>
#include <RPMUtils/RPMUtils.h>

class FileInfoCache
{
public:
	static const FileInfoCache *Add(const wchar_t *file);
	static const FileInfoCache *Find(const wchar_t *file);
	const NxlPath &path() const { return _filePath; }
	bool isprotected() const { return _isProtected; }
	const NxlMetadataCollection &tags() const { return _tags; }
protected:
	FileInfoCache(const wchar_t*file, bool isprotected, NxlMetadataCollection tags) :_filePath(NxlPath(file)), _isProtected(isprotected), _tags(tags) {}
	NxlPath _filePath;
	bool _isProtected;
	NxlMetadataCollection _tags;
};