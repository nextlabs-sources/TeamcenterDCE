#pragma once

#include "External\RMXLResult.h"

class CRMXNxlFile
{
public:
	CRMXNxlFile(const std::wstring& filePath);
	virtual ~CRMXNxlFile();

	std::wstring GetOrigFilePath() const { return m_origFilePath; }
	std::wstring GetNxlFilePath() const { return m_nxlFilePath; }
	std::wstring GetNativeFilePath() const { return m_nativeFilePath; }
	std::wstring GetDirectory() const { return m_directory;  }
	std::wstring GetNativeFileName() const { return m_nativeFileName; }
	std::wstring GetNxlFileName() const { return m_nxlFileName; }

	bool operator == (const std::wstring &nxlFilePath);

	bool Exist() const;
	bool NativeFileExist() const;

private:
	std::wstring m_origFilePath;
	std::wstring m_nxlFilePath;
	std::wstring m_nativeFilePath;
	std::wstring m_directory;
	std::wstring m_nxlFileName;
	std::wstring m_nativeFileName;
};

