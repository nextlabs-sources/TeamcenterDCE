#pragma once

#include <string>
#include <windows.h>

class CSysErrorMessage;
class SDWLResult;

class CRMXResult
{
public:
	CRMXResult() :m_code(0) {}
	explicit CRMXResult(DWORD code, const std::wstring& message);

	CRMXResult(const SDWLResult& sdResult);

	operator bool() const { return (0 == m_code); }
	CRMXResult& operator = (const SDWLResult& sdResult);
	CRMXResult& operator = (const CRMXResult& other);
	void Assign(DWORD code, const std::wstring& message);

	std::wstring ToString() const;
	DWORD GetCode() const { return m_code; }
	std::wstring GetMessage() const { return m_message; }

private:
	DWORD m_code;
	std::wstring m_message;
};

class CRMXLastResult
{
public:
	CRMXLastResult();
	~CRMXLastResult();

	CRMXLastResult& operator = (const CRMXResult& rmxResult);
	CRMXLastResult& operator = (const SDWLResult& sdResult);
	CRMXLastResult& operator = (const CSysErrorMessage& sysError);
	operator bool() const { return (0 == m_currentResult.GetCode()); }
	std::wstring ToString() const { return m_currentResult.ToString();  }

private:
	CRMXResult m_currentResult;
};
