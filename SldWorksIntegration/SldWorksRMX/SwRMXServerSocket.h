#pragma once

class CSwRMXServerSocket {

private:
	CSwRMXServerSocket();
	CSwRMXServerSocket(const CSwRMXServerSocket &);
	~CSwRMXServerSocket();
	static int CreateServerSocket();
	static SOCKET m_listeningSocket;
	static SOCKET m_connectedSocket;
public :
	static unsigned int __stdcall InitServerSocket(void *); 
	static int shutDownSocket();

};