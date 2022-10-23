#pragma once
#include "stdafx.h"
#include "SwRMXMgr.h"


enum class MsgType {
	MSG_INFO = 1,
	MSG_WARN = 2,
	MSG_ERROR = 3
};


class CSwResult {
public : 
	CSwResult() {

	}
	virtual ~CSwResult() {
	
	}

	//Getters
	inline MsgType GetMessageType() {
		return m_msgType;
	}
	virtual wstring GetErrorMsg();

	//Setters
	inline void SetErrorMsg(wstring msg) {
		m_Msg = msg;
	}
	inline void SetErrorMsg(UINT msgID) {
		CComBSTR bstr;
		bstr.LoadString(msgID);
		m_Msg = bstr;
	}
	inline void SetMessageType(MsgType msgType) {
		m_msgType = msgType;
	}

protected:
	wstring m_Msg;
	MsgType m_msgType;

	
};

class  CSwAuthResult : public CSwResult {
private :
	wstring m_fileName;
	FileRight m_fileRight;
	bool m_authResult;
	vector<wstring> m_fileDenyAccessList;

public:
	CSwAuthResult() {
		//Default Constructor
	}
	CSwAuthResult(wstring fileName) : m_fileName(fileName) {
		
	}

	~CSwAuthResult(){

	}

	//Getters
	inline wstring FileName() {
		return m_fileName;
	}

	inline FileRight GetFileRight() {
		return m_fileRight;
	}

	inline bool GetAuthResult() {
		return (m_fileDenyAccessList.size() > 0) ? false : true;
	}

	inline vector<wstring> GetFileDenyAccessList() {
		return m_fileDenyAccessList;
	}

	wstring GetErrorMsg();

	
	//Setters

	inline void SetFileName(wstring fileName) {
		m_fileName = fileName;
	}

	inline void SetFileRight(FileRight fileRight) {
		m_fileRight = fileRight;
	}

	inline void SetAuthResult(bool authResult) {
		m_authResult = authResult;
	}
	
	inline void AddFilesToDenyAccessList(const wstring& fileName) {
		for (auto f : m_fileDenyAccessList) {
			if (!_wcsicmp(f.c_str(),fileName.c_str()))
				return ;
		}
		m_fileDenyAccessList.push_back(fileName);
	}
};