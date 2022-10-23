#pragma once
#include <set>
#include <string>

#include "..\common\CommonTypes.h"

#include <pfcBase.h>
#include <pfcSession.h>

//*****************************************************************************
class CDirectoryChangeListener : public pfcDefaultSessionActionListener
{
public:
	void  OnAfterDirectoryChange(xrstring path);
};

class CRPMDirectoryMgr : public RMXControllerBase<CRPMDirectoryMgr>
{
public:
	void Start();
	void Stop();

	void UpdateCurrentDirectory(const std::wstring& newDir);
	void SyncCurrentDirectory();
	void AddRPMDir(const std::wstring& dir);
private:
	std::wstring m_currentDirectory;
	std::set<std::wstring, ICaseKeyLess> m_addedRPMDirs;
	pfcActionListener_ptr m_pDirChangeListener;
	std::wstring m_initialWorkDir;
};

DEFINE_RMXCONTROLLER_GET(RPMDirectoryMgr)
