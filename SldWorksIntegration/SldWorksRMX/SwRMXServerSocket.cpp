#undef UNICODE

#define WIN32_LEAN_AND_MEAN


#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include "SwRMXServerSocket.h"
#include "SwRMXSwimInterface.h"
#include "SwResult.h"
#include "RMXUtils.h"

#define SUCCESS "1\r\n"
#define NO_SUCCESS "0\r\n"

SOCKET CSwRMXServerSocket::m_connectedSocket = INVALID_SOCKET;
SOCKET CSwRMXServerSocket::m_listeningSocket = INVALID_SOCKET;

CSwRMXServerSocket::CSwRMXServerSocket() {
	//Constructor
}


CSwRMXServerSocket:: ~CSwRMXServerSocket() {
	//Destructor
}

unsigned int __stdcall CSwRMXServerSocket::InitServerSocket(void *) {
	LOG_INFO("CSwRMXServerSocket::InitServerSocket called");
	//Create the Server Socket
	DWORD threadId = GetCurrentThreadId();
	int status = CreateServerSocket();
	LOG_INFO_FMT("Thread = %lu is returning from CSwRMXServerSocket::InitServerSocket with status = %d: ",threadId, status);
	return status ;
}


// Need to link with Ws2_32.libs
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "27215"

//int __cdecl main(void)
int CSwRMXServerSocket::CreateServerSocket()
{
	LOG_INFO("CSwRMXServerSocket::CreateServerSocket called");
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LOG_ERROR("WSAStartup failed with error: %d\n" << iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		LOG_ERROR("getaddrinfo failed with error:" << iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		LOG_ERROR("socket failed with error: " << WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	m_listeningSocket = ListenSocket;

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		LOG_ERROR("bind failed with error: " << WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		m_listeningSocket = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		LOG_ERROR("listen failed with error: " << WSAGetLastError());
		closesocket(ListenSocket);
		m_listeningSocket = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		LOG_ERROR("accept failed with error: " << WSAGetLastError());
		closesocket(ListenSocket);
		m_listeningSocket = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	m_connectedSocket = ClientSocket;

	// No longer need server socket
	closesocket(ListenSocket);
	m_listeningSocket = INVALID_SOCKET;

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) { 
			LOG_INFO("Bytes received: " << iResult);
			//Do authoriation check at this point.
			string recvStr = string(recvbuf);
			size_t pos = recvStr.find("|");
			if (pos == string::npos)
			{
				LOG_ERROR_FMT("Invalid data read from socket: %s", recvStr.c_str());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			// Parse socket argument with format like "rightName"|"fileName"
			string right = recvStr.substr(0, pos);
			string fileName = recvStr.substr(pos+1, (recvStr.find("\r")) - pos - 1);
			CSwAuthResult authResultObj;
			CSwRMXSwimInterface rmxSwimInterfaceObj;
			
			int errCode = rmxSwimInterfaceObj.ProcessSwimRequest(fileName, right, authResultObj);
			string err_msg;
			if (errCode)
			{
				err_msg = std::to_string(errCode) + "\r\n";
			}
			else
			{
				err_msg = "0\r\n";
			}
			
			char *sendbuf;
			size_t len = err_msg.size()+1;
			sendbuf = (char *)malloc(len * sizeof(char));
			memset(sendbuf, 0, len);
			strcpy_s(sendbuf, len, err_msg.c_str());

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, sendbuf, (int)(strlen(sendbuf)), 0);
			free(sendbuf);
			if (iSendResult == SOCKET_ERROR) {
				LOG_ERROR("send failed with error: " << WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			LOG_INFO("Bytes sent: "<< iSendResult);
		}
		else if (iResult == 0) {
			LOG_INFO("Connection closing...\n");
		}
		else {
			LOG_ERROR("recv failed with error: "<< WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutDownSocket();
	if (iResult)
		return 1;

	return 0;
}

int CSwRMXServerSocket::shutDownSocket() {
	LOG_INFO("CSwRMXServerSocket::shutDownSocket called");
	int iResult;
	if (m_listeningSocket != INVALID_SOCKET) {
		//Handles the case if user launch SolidWorks and then disables the RMX in current active session before any client can connect to the server.
		LOG_INFO("Closing listening for incoming client connections.");
		closesocket(m_listeningSocket);
		m_listeningSocket = INVALID_SOCKET;
	}
	if (m_connectedSocket != INVALID_SOCKET) {
		//Handles the case when the client has already established connection with server socket, later user disables the RMX in current active session ,then need to tear the connection of server socket with client socket
		LOG_INFO("Tearing down the connection with client socket");
		iResult = shutdown(m_connectedSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			LOG_ERROR("shutdown failed with error: " << WSAGetLastError());
			closesocket(m_connectedSocket);
			m_connectedSocket = INVALID_SOCKET;
			WSACleanup();
			return 1;
		}
		else {
			// cleanup
			closesocket(m_connectedSocket);
			m_connectedSocket = INVALID_SOCKET;
			WSACleanup();
		}
		
	}

	return 0;

}