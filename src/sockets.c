/*
 * sockets.c
 *
 *  Created on: Mar 24, 2014
 *      Author: jrm
 *
 *  Slightly modified from code in:
 *     Unix Systems Programming: Communication, Concurrency, and Threads
 *     By Kay A. Robbins , Steven Robbins
 */
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAXBACKLOG 50

int u_ignore_sigpipe(void);

/**
 * The combination of socket, bind and listen establishes a handle for the server to monitor
 * communication requests from a well-known port.
 */
int u_open(int port) {
	int error;struct sockaddr_in server;
	int sock;
	int true = 1;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&true,
			sizeof(true)) == -1) {
		error = errno;
		while ((close(sock) == -1) && (errno == EINTR));
		errno = error;
		return -1;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons((short)port);
	if ((bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) ||
			(listen(sock, MAXBACKLOG) == -1)) {
		error = errno;
		while ((close(sock) == -1) && (errno == EINTR));
		errno = error;
		return -1;
	}
	return sock;
}

/**
 * The u_accept function waits for a connection request from a client and returns a file descriptor
 * that can be used to communicate with that client. It also fills in the name of the client host in a
 * user-supplied buffer. The socket accept function returns information about the client in a
 * struct sockaddr_in structure. The client's address is contained in this structure.
 */
int u_accept(int fd, char *hostn, int hostnsize) {
	int len = sizeof(struct sockaddr);
	struct sockaddr_in netclient;
	int retval;
	while (((retval = accept(fd, (struct sockaddr *)(&netclient), &len)) == -1) && (errno == EINTR));
	if ((retval == -1) || (hostn == NULL) || (hostnsize <= 0)) {
		return retval;
	}
	return retval;
}

/*
 * a function that initiates a connection request to a server. The
 * u_connect function has two parameters, a port number (port) and a host name (hostn), which
 * together specify the server to connect to.
 */
int u_connect(int port, char *hostn) {
	int error;
	int retval;
	struct sockaddr_in server;
	int sock;
	fd_set sockset;
	server.sin_port = htons((short)port);
	server.sin_family = AF_INET;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	if (((retval =
			connect(sock, (struct sockaddr *)&server, sizeof(server))) == -1) &&
			((errno == EINTR) || (errno == EALREADY))) {
		/* asynchronous */
		FD_ZERO(&sockset);
		FD_SET(sock, &sockset);
		while (((retval = select(sock+1, NULL, &sockset, NULL, NULL)) == -1)
				&& (errno == EINTR)) {
			FD_ZERO(&sockset);
			FD_SET(sock, &sockset);
		}
	}
	if (retval == -1) {
		error = errno;
		while ((close(sock) == -1) && (errno == EINTR));
		errno = error;
		return -1;
	}
	return sock;
}
