#pragma once
#include <Windows.h>
#include <string>
class CSysErrorMessage
{
public:
	explicit CSysErrorMessage(DWORD dwError):m_lpszMsg(nullptr) { Assign(dwError); }
	CSysErrorMessage(const CSysErrorMessage& other) { Assign(other.GetCode()); }
	~CSysErrorMessage() { Clear();  };
	
	CSysErrorMessage& operator= (DWORD dwError); 
	CSysErrorMessage& operator= (const CSysErrorMessage& other);

	DWORD GetCode() const { return m_dwError; }
	LPTSTR GetMsg() const { return m_lpszMsg; }

	operator bool() const { return m_dwError == ERROR_SUCCESS; }
	operator LPTSTR() const { return GetMsg(); }
	operator LPCTSTR() const { return GetMsg(); }

	void Clear();	 
	void Assign(DWORD dwError);

protected:
	DWORD m_dwError;
	LPTSTR m_lpszMsg;

};

inline CSysErrorMessage & CSysErrorMessage::operator=(DWORD dwError)
{
	if (dwError != m_dwError)
		Assign(dwError);
	return *this;
}

inline CSysErrorMessage & CSysErrorMessage::operator=(const CSysErrorMessage & other)
{
	if (this != &other)
		Assign(other.GetCode());
	return *this;
}

inline void CSysErrorMessage::Clear()
{
	if (m_lpszMsg != NULL)
	{
		LocalFree(m_lpszMsg);
		m_lpszMsg = NULL;
	}
	m_dwError = ERROR_SUCCESS;
}

inline void CSysErrorMessage::Assign(DWORD dwError)
{
	Clear();
	m_dwError = dwError;
	::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		m_dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&m_lpszMsg,
		0,
		NULL
	);

	// Remove the newline characters in the end
	LPTSTR p = wcschr(m_lpszMsg, L'\r');
	if (p != NULL)
	{
		*p = L'\0';
	}
}