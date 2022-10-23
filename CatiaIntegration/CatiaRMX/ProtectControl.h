#pragma once

#include "CatEventMgr.h"

class CProtectControl : public RMXControllerBase<CProtectControl>, public CCatEventLisener
{
public:
	void Start();
	void Stop();

	virtual void OnAfterSave(LPCatSaveNotify pParam);
};

#define curProtectControl CProtectControl::GetInstance()
