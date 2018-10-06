#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define PORT 8000
#define BUFF_LEN 512

#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdio.h>
#include <iostream>

void Start()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("ERROR with WSAStartup.\n");
	}
}

void EndApp()
{
	WSACleanup();
}

void UDP_Client()
{
	// UDP protocol
	printf(" --- UDP CONNECTION ---\n\n");
	SOCKET my_socket = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(PORT); // Port
	const char *remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &bindAddr.sin_addr);

	int slen = sizeof(bindAddr);

	// Send Message
	for (int i = 0; i < 5; i++)
	{
		Sleep(500);
		const char* buffer = "PING";
		char recived_buffer[250];
		memset(recived_buffer, '\0', sizeof(recived_buffer));

		// Send info
		if (sendto(my_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == -1)
		{
			printf("CLIENT ERROR: sending message failed\n");
			break;
		}
		printf("Client sended the message: %s\n", buffer);
		printf("Client waiting for data\n");
		// Recive info
		if (recvfrom(my_socket, recived_buffer, BUFF_LEN, 0, (struct sockaddr*)&bindAddr, &slen) == -1)
		{
			printf("CLIENT ERROR: reciving message failed\n");
			break;
		}
		printf("Client recived the message: %s\n", recived_buffer);
	}
	closesocket(my_socket);
}

void TCP_Client()
{
	// TCP protocol
	printf(" --- TCP CONNECTION ---\n\n");
	SOCKET my_socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(PORT); // Port
	const char *remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &bindAddr.sin_addr);
	int slen = sizeof(bindAddr);

	// Conect to server
	if (connect(my_socket, (sockaddr*)&bindAddr, slen) == SOCKET_ERROR)
	{
		printf("SERVER ERROR %i: connecting to server failed\n", WSAGetLastError());
	}
	else
		printf("Client connected to server\n");
	// Send Message
	for (int i = 0; i < 5; i++)
	{
		Sleep(500);
		const char* buffer = "PING\n";
		char recived_buffer[250];
		memset(recived_buffer, '\0', sizeof(recived_buffer));

		// Send info
		if (send(my_socket, buffer, strlen(buffer), 0) == SOCKET_ERROR)
		{
			printf("SERVER ERROR %i: sending message failed\n", WSAGetLastError());
			break;
		}

		printf("CLIENT SENDED: %s", buffer);

		// Recive info
		if (recv(my_socket, recived_buffer, BUFF_LEN, 0) == SOCKET_ERROR)
		{
			printf("SERVER ERROR %i: reciving message failed\n", WSAGetLastError());
			break;
		}

		printf("CLIENT RECIVED: %s\n", recived_buffer);
	}
	shutdown(my_socket, SD_BOTH);
	printf("          *- CLIENT DISCONNECTED -*\n");
	closesocket(my_socket);
}

int main()
{
	// Start library
	Start();

	// Creates UDP server
	UDP_Client();

	// Creates TCP server
	TCP_Client();

	// Closes library
	EndApp();

	// Pause app before close it
	getchar();
}


