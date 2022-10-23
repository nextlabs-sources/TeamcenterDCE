#pragma once

#include <map>
#include <vector>
#include <Windows.h>

#include "..\common\CommonTypes.h"

#include <pfcSession.h>

//*****************************************************************************
class COverlaySessionActionListener : public pfcDefaultSessionActionListener
{
public:
	void  OnAfterModelDisplay();
};

//*****************************************************************************
class COverlayModelEventActionListener : public pfcDefaultModelEventActionListener
{
public:
	void OnAfterModelErase(pfcModelDescriptor_ptr pMdlDescr);
};

//*****************************************************************************
class COverlayCommandBracketListener : public virtual pfcUICommandBracketListener
{
public:
	COverlayCommandBracketListener(const std::string& cmdName);
	void OnBeforeCommand();
	void OnAfterCommand() {};

private:
	std::string m_cmdName;

};


class CWatermarkController : public RMXControllerBase<CWatermarkController>
{
public:
	void Start();
	void Stop();

private:
	void DisableScreenCapture(std::vector<const wchar_t*>& files, HWND hWnd);

private:
	std::vector<pfcActionListener_ptr> m_listeners;
	std::map<std::string, pfcActionListener_ptr> m_cmdListeners;
};

