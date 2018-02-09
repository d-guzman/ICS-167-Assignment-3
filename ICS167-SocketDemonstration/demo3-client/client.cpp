/*
echoc: a demo of TCP/IP sockets connect

usage:	client [-h serverhost] [-p port]
*/
using namespace std;

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string>

#include "../util/getopt.h"

#include "../util/port.h"


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

SOCKET conn(char *host, int port);	/* connect to host,port; return socket */
void disconn(SOCKET clt);	/* close a socket connection */
int debug = 1;

int main(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	int c, err = 0;
	char *prompt = 0;
	int port = SERVICE_PORT;	/* default: whatever is in ../util/port.h */
	char default_host[] = "localhost"; /* default: this host */
	char *host = default_host;	
	SOCKET clt = INVALID_SOCKET;
	static char usage[] =
		"usage: %s [-d] [-h serverhost] [-p port]\n";

	while ((c = getopt(argc, argv, "dh:p:")) != -1)
	{
		switch (c) {
		case 'h':  /* hostname */
			host = optarg;
			break;
		case 'p':  /* port number */
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
	if (err || (optind < argc)) {	/* error or extra arguments? */
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}
	

	printf("connecting to %s, port %d\n", host, port);

	if ((clt = conn(host, port)) < 0)    /* connect */
		exit(1);   /* something went wrong */

				   /* in a useful program, we would do something here involving reads and writes on fd */

	disconn(clt);    /* disconnect */
	return 0;
}


/* conn: connect to the service running on host:port */
/* return -1 on failure, file descriptor for the socket on success */
SOCKET conn(char *host, int port)
{
	WSADATA wsaData;
	SOCKET clt = INVALID_SOCKET;

	/* initial Winsock*/
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		perror("cannot initialize Winsock");
		exit(1);
	}

	if (debug) printf("conn(host=\"%s\", port=\"%d\")\n", host, port);

	/* get a tcp/ip socket */
	/* We do this as we did it for the server */
	/* request the Internet address protocol */
	/* and a reliable 2-way byte stream */

	clt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clt == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}

	/* bind to an arbitrary return address */
	/* because this is the client side, we don't care about the */
	/* address since no application will connect here  --- */
	/* INADDR_ANY is the IP address and 0 is the socket */
	/* htonl converts a long integer (e.g. address) to a network */
	/* representation (agreed-upon byte ordering */
	struct sockaddr_in myaddr;	/* our address */
	char myaddr_str[INET_ADDRSTRLEN];

	// Set up the sockaddr structure
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if ((bind(clt, (SOCKADDR*)&myaddr, sizeof(myaddr))) == SOCKET_ERROR) {
		perror("bind failed\n");
		closesocket(clt);
		WSACleanup();
		exit(1);
	}

	/* this part is for debugging only - get the port # that the operating */
	/* system allocated for us. */
	unsigned int alen;	/* address length when we get the port number */
	alen = sizeof(myaddr);
	if (getsockname(clt, (struct sockaddr *)&myaddr, (int *)&alen) < 0) {
		perror("getsockname failed");
		closesocket(clt);
		exit(1);
	}
	if (debug) printf("local port number = %d\n", ntohs(myaddr.sin_port));

	/* fill in the server's address and data */
	struct addrinfo *servaddr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	/* look up the address of the server given its name */
	stringstream portsstr;
	portsstr << port;
	string portstr = portsstr.str();
	const char *portcstr = portstr.c_str();
	if ((getaddrinfo(host, portcstr, &hints, &servaddr)) != 0) {
		perror("getaddrinfo failed with error\n");
		closesocket(clt);
		WSACleanup(); 
		exit(1);
	}

	/* connect to server */
	if (connect(clt, servaddr->ai_addr, (int)servaddr->ai_addrlen) == INVALID_SOCKET) {
		perror("connect failed\n");
		closesocket(clt);
		exit(1);
	}
	if (debug) printf("connected socket = %d\n", clt);
	return clt;
}

/* disconnect from the service */
/* lame: we can just as easily do a shutdown() or close() ourselves */

void disconn(SOCKET clt)
{
	if (debug) printf("disconn(%d)\n", clt);
	shutdown(clt, 2);    /* 2 means future sends & receives are disallowed */
}
