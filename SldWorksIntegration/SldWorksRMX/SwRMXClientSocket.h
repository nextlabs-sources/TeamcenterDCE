#pragma once

class CSwRMXClientSocket {

private:
	static SOCKET m_connectedSocket;
public:
	CSwRMXClientSocket();
	~CSwRMXClientSocket();
	int InitSocket();
	int Send(string data);
	int Receive(string &data);
	int CloseSend();
	int Close();
};