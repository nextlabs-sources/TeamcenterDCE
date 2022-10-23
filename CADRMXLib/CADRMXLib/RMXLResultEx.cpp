#include "RMXLResultEx.h"

#include <RMXUtils.h>

#include <SDLResult.h>

CRMXLResultEx::CRMXLResultEx()
{
}

CRMXLResultEx::CRMXLResultEx(RMXErrCode_t code, const std::wstring & msg)
	: RMXResult(code, msg)
{
}

CRMXLResultEx::CRMXLResultEx(const SDWLResult & sdResult)
	: RMXResult(sdResult.GetCode(), RMXUtils::s2ws(sdResult.GetMsg()))
{
}

CRMXLResultEx & CRMXLResultEx::operator=(const SDWLResult & sdResult)
{
	m_code = sdResult.GetCode();
	m_message = RMXUtils::s2ws(sdResult.GetMsg());
	return *this;
}

CRMXLResultEx::operator RMXResult() const
{
	return RMXResult(m_code, m_message);
}

std::wstring CRMXLResultEx::ToString() const
{
	return std::wstring(L"{") + std::to_wstring(m_code) + L", \"" + m_message + L"\"}";
}
