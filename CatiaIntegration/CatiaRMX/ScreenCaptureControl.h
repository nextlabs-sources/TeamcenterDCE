#pragma once
#include "CatEventMgr.h"

class CScreenCaptureControl : public RMXControllerBase<CScreenCaptureControl>, public CCatEventLisener
{
public:
	void Start();
	void Stop();

	// Event handler
	void OnAfterCommand(LPCatCommandNotify pParam);
	void OnFrmLayoutChanged(LPCatFrmNotify pParam);
	void OnFullScreenChanged(int nFullScreen);

private:
	bool ApplyToLayout(const void* pFrmLayout);
	bool ApplyToCurrent(bool bFullScreen);
	void ApplyToOpenDocs();
};

#define curScnCaptureControl CScreenCaptureControl::GetInstance()