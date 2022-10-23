#pragma once

typedef LONG_PTR            LPCatNotify;

enum eCatNotifyType
{
	eCatNotify_BeforeCommand = 1,
	eCatNotify_AfterCommand = 2,
	eCatNotify_BeforeSave = 4,
	eCatNotify_AfterSave = 6,
	eCatNotify_FrmLayoutChanged = 8,
	eCatNotify_FullScreenChanged = 10
	
};

struct CatCommandNotify {
	wstring wsCmdName;
	wstring wsFileFullName;
	std::set<wstring, ICaseKeyLess> ctxFiles;
	bool bCancel = false;
};
typedef CatCommandNotify* LPCatCommandNotify;

struct CatSaveNotify {
	wstring wsFileFullName;
	bool bPostSaveDone = false;
};
typedef CatSaveNotify* LPCatSaveNotify;

struct CatFrmNotify {
	wstring wsCmdName;
	const void* pFrmLayout;
};
typedef CatFrmNotify* LPCatFrmNotify;

class CCatEventLisener
{
public:
	virtual void OnBeforeCommand(LPCatCommandNotify pParam) {};
	virtual void OnAfterCommand(LPCatCommandNotify pParam) {};
	virtual void OnBeforeSave(LPCatSaveNotify pParam) {};
	virtual void OnAfterSave(LPCatSaveNotify pParam) {};
	virtual void OnFrmLayoutChanged(LPCatFrmNotify pParam) {};
	virtual void OnFullScreenChanged(int nFullScree/*1: Fullscreen*/) {};
};

class CCatEventMgr : public RMXControllerBase<CCatEventMgr>
{
public:
	void Start();
	void Stop();

	void AddListener(CCatEventLisener* listener, DWORD dwEvents);
	void RemoveListener(CCatEventLisener* listener, DWORD dwEvents);

	// Event handlers
public:
	void Notify(eCatNotifyType eNotifyType, LPCatNotify lParam);

private:
	std::vector<std::pair<CCatEventLisener*, DWORD>> m_listeners;

};

#define curEventMgr CCatEventMgr::GetInstance()

