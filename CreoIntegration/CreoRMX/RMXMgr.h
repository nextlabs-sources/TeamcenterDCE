#pragma once

class CRMXMgr
{
public:
	static CRMXMgr& GetInstance();

	bool ProtectFile(const char* fileFullName);

private:
	CRMXMgr();
	~CRMXMgr();

	CRMXMgr(const CRMXMgr&) {};
	CRMXMgr& operator = (const CRMXMgr&) {};
};

