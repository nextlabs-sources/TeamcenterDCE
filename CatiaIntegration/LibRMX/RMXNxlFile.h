#pragma once

#include <string>
#include <vector>
#include "RMXTypes.h"
#include <SDLTypeDef.h>

class CRMXNxlFile
{
public:
	CRMXNxlFile(const std::wstring& filePath);
	virtual ~CRMXNxlFile();

	std::wstring GetOrigFilePath() const { return m_origFilePath; }
	std::wstring GetNxlFilePath() const { return m_nxlFilePath; }
	std::wstring GetPlainFilePath() const { return m_plainFilePath; }
	std::wstring GetDirectory() const { return m_directory;  }

	bool operator == (const std::wstring &nxlFilePath);

	bool IsProtected() const;
	bool IsValidNxl() const;
	bool IsInRPMFolder() const;
	
	std::string GetTags();
	static std::wstring GetRightName(unsigned long rights);
	bool GetRights(std::vector<RMXFileRight>& rights) const;
	bool CheckRights(unsigned long rights, bool audit);
	bool HasRights(unsigned long allRights, unsigned long rightsToCheck, bool audit);
	bool CopyTo(const std::wstring & destFilePath, bool deleteSource /*= false*/);
	bool MoveTo(const std::wstring& destFilePath);
	bool Delete();

	bool AddActivityLog(RMXActivityLogOperation op, RMXActivityLogResult result, bool testNxl = true);
	bool AddActivityLogByRight(unsigned long r, RMXActivityLogResult result, bool testNxl = true);

	bool EnsureNxlExtension();
	bool AddAttributes(unsigned long attrs);
	bool SetAttributes(unsigned long attrs);

private:
	std::string  ReadTagsFromFile() const;

private:
	std::wstring m_origFilePath;
	std::wstring m_nxlFilePath;
	std::wstring m_plainFilePath;
	std::wstring m_directory;
};

