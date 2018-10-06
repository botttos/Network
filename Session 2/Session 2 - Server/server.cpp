#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <list>
#include <cstdlib>
#include <iostream>

void logSocketErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	system("pause");
	exit(-1);
}

SOCKET serverSocket; // Listen socket
std::list<SOCKET> sockets; // All connected sockets

void handleIncomingData()
{
	// Input buffer
	const int inputBufferLen = 1300;
	char inputBuffer[inputBufferLen];

	// Configure socket sets
	fd_set readfds;
	FD_ZERO(&readfds);

	// Fill the set
	for (auto s : sockets) {
		FD_SET(s, &readfds);
	}

	// Timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	//struct timeval *timeoutPtr = nullptr; // Infinite wait

	// Select
	int res = select(0, &readfds, nullptr, nullptr, &timeout);
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("select 4 read");
	}

	// Sockets to remove
	std::list<SOCKET> disconnectedSockets;

	// Read selected sockets
	for (auto s : sockets)
	{
		if (FD_ISSET(s, &readfds)) {

			// Is the server socket
			if (s == serverSocket) {
				// Accept
				struct sockaddr_in clientAddress;
				int clientAddressLen = sizeof(clientAddress);
				SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
				if (clientSocket == INVALID_SOCKET) {
					logSocketErrorAndExit("accept");
				}
				sockets.push_back(clientSocket);
				std::cout << "Client connected" << std::endl;
			}

			// Is a client socket
			else {

				// Receive data
				int bytesRecv = recv(s, inputBuffer, inputBufferLen, 0);
				if (bytesRecv == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAECONNRESET) {
						std::cout << "Client disconnected forcibly" << std::endl;
						disconnectedSockets.push_back(s);
					} else {
						logSocketErrorAndExit("recv");
					}
				}
				else if (bytesRecv == 0) {
					std::cout << "Client disconnected properly" << std::endl;
					disconnectedSockets.push_back(s);
				}
				else {
					std::cout << " - recv from client: " << inputBuffer << std::endl;
				}
			}
		}
	}

	// Remove disconnected sockets
	for (auto s : disconnectedSockets) {
		sockets.remove(s);
	}
}

void handleOutgoingData()
{
	// Avoid doing this if there is no data to send to any socket
	if (sockets.empty()) {
		return;
	}

	// Configure socket sets
	fd_set sendfds;
	FD_ZERO(&sendfds);

	// Fill the set
	for (auto s : sockets) {
		FD_SET(s, &sendfds);
	}

	// Timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	//struct timeval *timeoutPtr = nullptr; // Infinite wait

	// Select
	int res = select(0, nullptr, &sendfds, nullptr, &timeout);
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("select 4 send");
	}

	// Sockets that disconnected
	std::list<SOCKET> disconnectedSockets;

	// Send data to selected sockets
	for (auto s : sockets)
	{
		if (FD_ISSET(s, &sendfds)) {

			// Send data
			const char *outputBuffer = "Update state packet";
			const int outputBufferLen = strlen(outputBuffer) + 1;
			int bytesSent = send(s, outputBuffer, outputBufferLen, 0);
			if (bytesSent == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAECONNRESET) {
					std::cout << "Client disconnected forcibly" << std::endl;
					disconnectedSockets.push_back(s);
				}
				else {
					logSocketErrorAndExit("send");
				}
			}
		}
	}

	// Remove disconnected sockets
	for (auto s : disconnectedSockets) {
		sockets.remove(s);
	}
}

void server(int port)
{
	// Startup
	WSAData wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("WSAStartup");
	}

	// Create
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		logSocketErrorAndExit("socket");
	}

	// Add the socket to the Socket list
	sockets.push_back(serverSocket);

	// Socket option
	int enable = 1;
	res = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&enable, sizeof(int));
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("setsockopt(SO_REUSEADDR)");
	}

	// Bind
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	//serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	res = bind(serverSocket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("bind");
	}

	// Listen
	res = listen(serverSocket, 32);
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("listen");
	}

	// Loop
	while (true)
	{
		handleIncomingData();

		// handleInput();
		// simulatePhysics();
		// simulateAI();
		// ...

		handleOutgoingData();

		// Print iteration
		static int iteration = 0;
		std::cout << "Server iteration " << iteration++ << std::endl;

		// Wait a second
		Sleep(2000);
	}

	// Delete
	closesocket(serverSocket);

	// Cleanup
	res = WSACleanup();
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("WSACleanup");
	}
}

int main(int argc, char **argv)
{
	const int port = 8000;
	server(port);
	system("pause");
	return 0;
}
