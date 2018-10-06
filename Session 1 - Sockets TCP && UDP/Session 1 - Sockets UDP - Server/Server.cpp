#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define PORT 8000
#define MAX_CALLS 1
#define BUFF_LEN 512

#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdio.h>
#include <iostream>

void Start()
{
	// Init library
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("ERROR with WSAStartup.\n");
	}
}

void UDP_Server()
{
	// UDP protocol
	printf(" --- UDP CONNECTION ---\n\n");
	SOCKET my_socket = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in server;
	server.sin_family = AF_INET; // IPv4
	server.sin_port = htons(PORT); // Port
	const char *remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &server.sin_addr);

	// Prevent errors with bind
	int enable = 1;
	int res = setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&enable, sizeof(int));
	if (res == SOCKET_ERROR) 
	{
	// Log and handle error
	}
	// Bind
	res = bind(my_socket, (const struct sockaddr *)&server, sizeof(server));
	int slen = sizeof(server);

	// Send Message
	for (int i = 0; i < 5; i++)
	{
		Sleep(500);
		const char* buffer = "PONG\n";
		char recived_buffer[250];
		memset(recived_buffer, '\0', sizeof(recived_buffer));
		
		// Recive info
		printf("Server waiting for data...\n");
		if (recvfrom(my_socket, recived_buffer, BUFF_LEN, 0, (struct sockaddr*)&server, &slen) == SOCKET_ERROR)
		{
			printf("SERVER ERROR: reciving message failed\n");
			getchar();
			break;
		}
		printf("Server recived the message: %s\n", recived_buffer);
		// Send info
		if (sendto(my_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		{
			printf("SERVER ERROR: sending message failed\n");
			getchar();
			break;
		}
		printf("Server sended the message: %s\n", buffer);
	}
	closesocket(my_socket);
}

void TCP_Server()
{
	// Set socket
	printf(" --- UDP CONNECTION ---\n\n");
	SOCKET my_socket = socket(AF_INET, SOCK_STREAM, 0);

	// Set server
	sockaddr_in server;
	server.sin_family = AF_INET; //IPv4
	server.sin_port = htons(PORT); //Port
	server.sin_addr.S_un.S_addr = INADDR_ANY; //Local IP
											  // Prevent errors with bind
	if (bind(my_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("BIND ERROR %i", WSAGetLastError());
	}

	// Server/Client variables
	sockaddr_in client;
	char client_msg[BUFF_LEN];
	char server_msg[BUFF_LEN] = "PONG\n";
	int slen = sizeof(client);

	// Listen
	if (listen(my_socket, MAX_CALLS) == SOCKET_ERROR)
	{
		printf("SERVER ERROR %i: listening client failed\n", WSAGetLastError());
	}

	// Accept client
	SOCKET new_socket;
	if ((new_socket = accept(my_socket, (sockaddr *)&client, &slen)) == SOCKET_ERROR)
	{
		printf("SERVER ERROR %i: accepting client failed\n", WSAGetLastError());
	}

	// Send/Recive data
	int finish = 1;
	while (1)
	{
		memset(client_msg, '\0', BUFF_LEN);
		// Recive data from client
		if ((finish = recv(new_socket, client_msg, BUFF_LEN, 0)) == SOCKET_ERROR || finish == 0)
		{
			if (finish == 0)
			{
				// Exit loop
				printf("          *- CLIENT DISCONNECTED FROM SERVER -*\n");
				break;
			}
			printf("SERVER ERROR %i: reciving message failed\n", WSAGetLastError());
			break;
		}
		
		printf("SERVER SAID: %s", client_msg);

		// Send data to client
		if (send(new_socket, server_msg, strlen(server_msg), 0) == SOCKET_ERROR)
		{
			printf("SERVER ERROR %i: sending message failed\n", WSAGetLastError());
			break;
		}

		printf("SERVER SENDED: %s\n", server_msg);
	}
	closesocket(my_socket);
	closesocket(new_socket);
}

void EndApp()
{
	WSACleanup();
}

void main()
{
	// Start library
	Start();

	// Creates UDP server
	UDP_Server();

	// Creates TCP server
	TCP_Server();

	// Closes library
	EndApp();

	// Pause app before close it
	getchar();
}
