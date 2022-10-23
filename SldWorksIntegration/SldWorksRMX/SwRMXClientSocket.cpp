#include "stdafx.h"
#include "SwRMXClientSocket.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27216"

SOCKET CSwRMXClientSocket::m_connectedSocket = INVALID_SOCKET;

CSwRMXClientSocket::CSwRMXClientSocket() {
	//Constructor
}


CSwRMXClientSocket:: ~CSwRMXClientSocket() {
	//Destructor
}



int CSwRMXClientSocket::InitSocket() {
	LOG_INFO("CSwRMXClientSocket::InitClientSocket called");
	if (m_connectedSocket != INVALID_SOCKET) {
		LOG_INFO("A client socket handle to swim server socket already exists");
		return 0;
	}
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,	*ptr = NULL, hints;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	m_connectedSocket = ConnectSocket;
	freeaddrinfo(result);
	return 0;
}


int CSwRMXClientSocket::Send(string data){
	int iResult;
	if (m_connectedSocket == INVALID_SOCKET) {
		LOG_ERROR("Unable to connect to server!. This could be due to the server socket on the SwimRMX has not started listening yet.");
		WSACleanup();
		return 1;
	}
	const char *sendbuf = data.c_str();
	// Send an initial buffer
	iResult = send(m_connectedSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		LOG_ERROR("send failed with error:" << WSAGetLastError());
		closesocket(m_connectedSocket);
		WSACleanup();
		return 1;
	}

	LOG_INFO("Bytes Sent:" << iResult);
	return 0;
}


int CSwRMXClientSocket::Receive(string &data) {
	int iResult = 0;
	if (m_connectedSocket != INVALID_SOCKET) {
		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;
		iResult = recv(m_connectedSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			data = string(recvbuf,iResult);
			LOG_ERROR("Bytes received: " << iResult);
		}
		else if (iResult == 0)
			LOG_ERROR("Connection closed");
		else
			LOG_ERROR("recv failed with error: " << WSAGetLastError());
	}
	return iResult;
}


int CSwRMXClientSocket::CloseSend()
{
	int iResult;
	// shutdown the connection since no more data will be sent
	iResult = shutdown(m_connectedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		LOG_ERROR("shutdown failed with error: " << WSAGetLastError());
		closesocket(m_connectedSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

int CSwRMXClientSocket::Close() {
	// cleanup
	if (m_connectedSocket != INVALID_SOCKET) {
		m_connectedSocket = INVALID_SOCKET;
		closesocket(m_connectedSocket);
		WSACleanup();
	}
	return 0;
}