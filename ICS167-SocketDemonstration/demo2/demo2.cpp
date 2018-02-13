/*
demo2: create a TCP/IP socket

usage:	demo2

create a socket. Bind it to any available port.
Then use getsockname to print the port.
*/

#include <stdio.h>	/* defines printf */

/*
Use the Winsock API by including the Winsock 2 header files.
The Winsock2.h header file contains most of the Winsock functions, structures, and definitions
*/
#include <winsock2.h>

/*
The Ws2tcpip.h header file contains definitions introduced in the WinSock 2 Protocol-Specific Annex document
for TCP/IP that includes newer functions and structures used to retrieve IP addresses.
*/
#include <ws2tcpip.h>

/*
Ensure that the build environment links to the Winsock Library file Ws2_32.lib.
Applications that use Winsock must be linked with the Ws2_32.lib library file.
The #pragma comment indicates to the linker that the Ws2_32.lib file is needed.
*/
#pragma comment(lib, "Ws2_32.lib")

/* 
Define a port number that your program will use.
The port number associated with the server that the client will connect to
*/
#define DEFAULT_PORT 33547

int main()
{
	/* 1. Initializing Winsock */
	WSADATA wsaData;

	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		perror("cannot initialize Winsock");
		return 1;
	}

	/* 2. Create a tcp/ip socket */
	/* request the Internet address protocol */
	/* and a reliable 2-way byte stream (TCP/IP) */

	SOCKET exampleSocket = INVALID_SOCKET;
	exampleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (exampleSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else {
		printf("Socket created successfully, descriptor: %d\n", exampleSocket);
	}

	/* 3. bind to an arbitrary return address */
	/* in this case, assume it's the client side, so we won't
	/* care about the port number as no application will connect here */
	/* INADDR_ANY is the IP address and 0 is the socket */
	/* htonl converts a long integer (e.g. address) to a network */
	/* representation (IP-standard byte ordering) */
	/* htons function converts a u_short from host to TCP/IP network byte order (which is big-endian) */
	struct sockaddr_in saServer;

	// Set up the sockaddr structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = htonl(INADDR_ANY);
	saServer.sin_port = htons(DEFAULT_PORT);

	// Bind the listening socket using the
	// information in the sockaddr structure
	if ((bind(exampleSocket, (SOCKADDR*)&saServer, sizeof(saServer))) == SOCKET_ERROR){
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(exampleSocket);
		WSACleanup();
		return 1;
	}

	printf("bind complete. Port number = %d\n", ntohs(saServer.sin_port));
	getchar();
	return 0;
}