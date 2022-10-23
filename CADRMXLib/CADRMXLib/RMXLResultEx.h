#pragma once

#include "External\RMXLResult.h"

class SDWLResult;

class CRMXLResultEx : public RMXResult
{
public:
	CRMXLResultEx();

	explicit CRMXLResultEx(RMXErrCode_t code, const std::wstring& msg);
	explicit CRMXLResultEx(const SDWLResult& other);

	CRMXLResultEx& operator = (const SDWLResult& other);

	operator RMXResult() const;
	std::wstring ToString() const;
};
// TODO:
//#define RMXL_RESULT(c)       CRMXLResult(c, __LINE__, __FILE__, __FUNCTION__, L"")
//#define RMXL_RESULT2(c, m)   CRMXLResult(c, __LINE__, __FILE__, __FUNCTION__, m)
#define RMX_RESULT(c)       CRMXLResultEx(c, L"")
#define RMX_RESULT2(c, m)   CRMXLResultEx(c, m)
