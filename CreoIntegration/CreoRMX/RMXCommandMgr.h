#pragma once
#include "..\common\CommonTypes.h"

class CRMXCommandMgr : public RMXControllerBase<CRMXCommandMgr>
{
public:
	void Start();
	void Stop();
};

DEFINE_RMXCONTROLLER_GET(RMXCommandMgr)