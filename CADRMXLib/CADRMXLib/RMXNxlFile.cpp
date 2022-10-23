#include "RMXNxlFile.h"
// System
#include <map>

#include <PathEx.h>
#include "RMXLLogger.h"
#include "RMXLResultEx.h"
#include <RMXUtils.h>
#include <SysErrorMessage.h>

using namespace std;

static const wchar_t* NXL_FILE_EXT = L".nxl";

CRMXNxlFile::CRMXNxlFile(const std::wstring & filePath)
{
	m_origFilePath = filePath;
	m_nxlFilePath = filePath;
	if (!m_nxlFilePath.empty())
	{
		// Format the path delimiter
		std::replace(m_nxlFilePath.begin(), m_nxlFilePath.end(), L'/', L'\\');
		CPathEx origFile(m_nxlFilePath.c_str());
		// In case the specified path contains .nxl file ext
		if (_wcsnicmp(origFile.GetExtension().c_str(), NXL_FILE_EXT, origFile.GetExtension().size()) == 0)
		{
			m_nativeFilePath = m_nxlFilePath.substr(0, wcslen(m_nxlFilePath.c_str()) - 4);
			m_nxlFileName = origFile.GetFileName();
			m_nativeFileName = m_nxlFileName.substr(0, wcslen(m_nxlFileName.c_str()) - 4);
		}
		else
		{		
			// In case the specified path does not contain .nxl ext
			m_nativeFilePath = m_nxlFilePath;
			m_nxlFilePath += NXL_FILE_EXT;
			m_nativeFileName = origFile.GetFileName();
			m_nxlFileName = m_nativeFileName + NXL_FILE_EXT;
		}
		m_directory = origFile.GetParentPath();
	}
}

CRMXNxlFile::~CRMXNxlFile()
{
}

bool CRMXNxlFile::operator==(const std::wstring & nxlFilePath)
{
	return (_wcsnicmp(m_nxlFilePath.c_str(), nxlFilePath.c_str(), nxlFilePath.size()) == 0);
}

bool CRMXNxlFile::Exist() const
{
	return CPathEx::IsFile(m_nxlFilePath.c_str());
}

bool CRMXNxlFile::NativeFileExist() const
{
	return CPathEx::IsFile(m_nativeFilePath.c_str());
}
