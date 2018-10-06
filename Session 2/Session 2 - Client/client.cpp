#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <cstdlib>
#include <iostream>

// Globals
bool g_End = false;
SOCKET g_Socket = INVALID_SOCKET;

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

void handleIncomingData()
{
	// Input buffer
	const int inputBufferLen = 1300;
	char inputBuffer[inputBufferLen];

	// Recv
	int bytesRecv = recv(g_Socket, inputBuffer, inputBufferLen, 0);
	if (bytesRecv == SOCKET_ERROR)
	{
		int lastError = WSAGetLastError();
		if (lastError == WSAEWOULDBLOCK) {
			// Do nothing
		}
		else if (lastError == WSAECONNRESET) {
			// Server disconnected
			g_End = true;
		}
		else {
			logSocketErrorAndExit("recv");
		}
	}
	else // Success
	{
		if (bytesRecv == 0) { // Server socket disconnected
			g_End = true;
		}
		else {
			std::cout << " - recv from server: " << inputBuffer << std::endl;
		}
	}
}

void handleOutgoingData()
{
	// Output buffer
	const char *outputBuffer = "Input packet";
	const int outputBufferLen = strlen(outputBuffer) + 1;

	// Send
	int bytesSent = send(g_Socket, outputBuffer, outputBufferLen, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		int lastError = WSAGetLastError();
		if (lastError == WSAEWOULDBLOCK) {
			// Do nothing
		}
		else if (lastError == WSAECONNRESET) {
			// Server disconnected
			g_End = true;
		}
		else {
			logSocketErrorAndExit("send");
		}
	}
	else // Success
	{
		// Nothing to do
	}
}

void client(const char *serverAddressStr, int port)
{
	// Startup
	WSAData wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("WSAStartup");
	}

	// Create
	g_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_Socket == INVALID_SOCKET) {
		logSocketErrorAndExit("socket");
	}

	// Connect
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);
	res = connect(g_Socket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("connect");
	}
	
	// Set non-blocking socket
	u_long nonBlocking = 1;
	res = ioctlsocket(g_Socket, FIONBIO, &nonBlocking);
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("ioctlsocket");
	}

	// Loop
	while (!g_End)
	{
		handleIncomingData();

		// updateGame();
		// renderGame();
		// ...

		handleOutgoingData();

		// Print iteration
		static int iteration = 0;
		std::cout << "Client iteration " << iteration++ << std::endl;

		// Wait a second
		Sleep(1000);
	}

	// Delete
	closesocket(g_Socket);
	std::cout << "closesocket done" << std::endl;

	// Cleanup
	res = WSACleanup();
	if (res == SOCKET_ERROR) {
		logSocketErrorAndExit("WSACleanup");
	}
}

int main(int argc, char **argv)
{
	const char *address = "127.0.0.1";
	const int port = 8000;
	client(address, port);
	system("pause");
	return 0;
}
