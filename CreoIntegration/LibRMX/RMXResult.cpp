#include "RMXResult.h"

#include <RMXUtils.h>
#include <SysErrorMessage.h>

#include <SDLResult.h>

#include "RMXInstance.h"

CRMXResult::CRMXResult(DWORD code, const std::wstring & message)
	: m_code(code), m_message(message)
{
}

CRMXResult::CRMXResult(const SDWLResult & sdResult)
{
	m_code = sdResult.GetCode();
	m_message = RMXUtils::s2ws(sdResult.GetMsg());
}

CRMXResult & CRMXResult::operator=(const SDWLResult & sdResult)
{
	m_code = sdResult.GetCode();
	m_message = RMXUtils::s2ws(sdResult.GetMsg());
	return *this;
}

CRMXResult & CRMXResult::operator=(const CRMXResult & other)
{
	if (this != &other)
	{
		m_code = other.m_code;
		m_message = other.m_message;
	}
	return *this;
}

void CRMXResult::Assign(DWORD code, const std::wstring & message)
{
	m_code = code;
	m_message = message;
}

std::wstring CRMXResult::ToString() const
{
	return std::wstring(L"{") + std::to_wstring(m_code) + L", \"" + m_message + L"\"}";
}

CRMXLastResult::CRMXLastResult()
{
}

CRMXLastResult::~CRMXLastResult()
{
	if (m_currentResult.GetCode() != 0)
	{
		CRMXInstance::GetInstance().SetLastError(m_currentResult);
	}	
}

CRMXLastResult & CRMXLastResult::operator=(const CRMXResult & rmxResult)
{
	m_currentResult = rmxResult;
	return *this;
}

CRMXLastResult & CRMXLastResult::operator=(const SDWLResult & sdResult)
{
	m_currentResult = sdResult;
	return *this;
}

CRMXLastResult & CRMXLastResult::operator=(const CSysErrorMessage & sysError)
{
	m_currentResult.Assign(sysError.GetCode(), sysError.GetMsg());
	return *this;
}
