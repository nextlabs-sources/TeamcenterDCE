#pragma once
#include "SwResult.h"
#include "SwRMXMgr.h"
// CSldWorksRMXDialog dialog

class CSldWorksRMXDialog 
{

public:
	CSldWorksRMXDialog();
	~CSldWorksRMXDialog();
	int ShowMessageDialog(CSwResult *swResultObj,int options = -1);
	int ShowAboutDialog();
};
