/*
server: TCP/IP sockets example
get a connect: keep reading data from the socket, echoing
back the received data.

usage:	server [-d] [-p port]
*/

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "../util/getopt.h"

#include "../util/port.h"

#ifndef ERESTART
#define ERESTART EINTR
#endif

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

void serve(int port);	/* main server function */

int main(int argc,      // Number of strings in array argv  
	char *argv[],   // Array of command-line argument strings  
	char *envp[])  // Array of environment variable strings  )
{
	extern char *optarg;
	extern int optind;
	int c, err = 0;
	int port = SERVICE_PORT; /* default: whatever is in ../util/port.h */
	static char usage[] = "usage: %s [-d] [-p port]\n";
	
	while ((c = getopt(argc, argv, "dp:")) != -1)
	{
		switch (c) {
		case 'p':
			port = atoi(optarg);
			if (port < 1024 || port > 65535) {
				fprintf(stderr, "invalid port number: %s\n", optarg);
				err = 1;
			}
			break;
		case '?':
			err = 1;
			break;
		}
		if (err == 1)
		{
			break;
		}
	}
	if (err || (optind < argc)) {
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}
	serve(port);
	return 0;
}

/* serve: set up the service */

void serve(int port)
{
	WSADATA wsaData;
	SOCKET svc = INVALID_SOCKET;
	SOCKET rqst = INVALID_SOCKET;

	/* initial Winsock*/
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		perror("cannot initialize Winsock");
		exit(1);
	}

	/* get a tcp/ip socket */
	/*   AF_INET is the Internet address (protocol) family  */
	/*   with SOCK_STREAM we ask for a sequenced, reliable, two-way */
	/*   conenction based on byte streams.  With IP, this means that */
	/*   TCP will be used */

	svc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (svc == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}

	/* we use setsockopt to set SO_REUSEADDR. This allows us */
	/* to reuse the port immediately as soon as the service exits. */
	/* Some operating systems will not allow immediate reuse */
	/* on the chance that some packets may still be en route */
	/* to the port. */

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	if ((setsockopt(svc, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, bOptLen)) == INVALID_SOCKET) {
		printf("setsockopt for SO_REUSEADDR failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}

	/* set up our address */
	/* htons converts a short integer into the network representation */
	/* htonl converts a long integer into the network representation */
	/* INADDR_ANY is the special IP address 0.0.0.0 which binds the */
	/* transport endpoint to all IP addresses on the machine. */

	struct sockaddr_in saServer;
	char saServer_addr_str[INET_ADDRSTRLEN];

	// Set up the sockaddr structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = htonl(INADDR_ANY);
	saServer.sin_port = htons(port);

	if ((bind(svc, (SOCKADDR*)&saServer, sizeof(saServer))) == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(svc);
		WSACleanup();
		exit(1);
	}

	/* set up the socket for listening with a queue length of 5 */
	if ((listen(svc, 5)) == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(svc);
		WSACleanup();
		exit(1);
	}

	printf("server started on %s, listening on port %d\n", inet_ntop(AF_INET, &(saServer.sin_addr), saServer_addr_str, INET_ADDRSTRLEN), port);

	/* loop forever - wait for connection requests and perform the service */
	struct sockaddr_in client_addr;
	socklen_t alen;
	alen = sizeof(client_addr);     /* length of address */
	char client_addr_str[INET_ADDRSTRLEN];

	for (;;) {
		while ((rqst = accept(svc, (struct sockaddr *)&client_addr, &alen)) < 0) {
			/* we may break out of accept if the system call */
			/* was interrupted. In this case, loop back and */
			/* try again */
			if ((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR)) {
				perror("accept failed");
				exit(1);
			}
		}

		printf("received a connection from: %s port %d\n",
			inet_ntop(AF_INET, &(client_addr.sin_addr), client_addr_str, INET_ADDRSTRLEN), ntohs(client_addr.sin_port));
		shutdown(rqst, 2);    /* close the connection */
	}
}