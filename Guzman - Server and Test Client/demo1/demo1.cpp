/*
demo1: create a TCP/IP socket in Windows

usage:	demo1
*/

#include <stdio.h>	/* defines printf */

/*
Use the Winsock API by including the Winsock 2 header files. 
The Winsock2.h header file contains most of the Winsock functions, structures, and definitions
*/
#include <winsock2.h>

/*
Ensure that the build environment links to the Winsock Library file Ws2_32.lib. 
Applications that use Winsock must be linked with the Ws2_32.lib library file. 
The #pragma comment indicates to the linker that the Ws2_32.lib file is needed.
*/
#pragma comment(lib, "Ws2_32.lib")

int main()
{
	/* 1. Initializing Winsock */
	/* Reference: */
	/* https://msdn.microsoft.com/en-us/library/windows/desktop/ms738566(v=vs.85).aspx */
	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		perror("cannot initialize Winsock\n");
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

	getchar();
	return 0;
}