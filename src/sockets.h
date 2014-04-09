/*
 * sockets.h
 *
 *  Created on: Mar 24, 2014
 *      Author: jrm
 *
 *  Slightly modified from code in:
 *     Unix Systems Programming: Communication, Concurrency, and Threads
 *     By Kay A. Robbins , Steven Robbins
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

int u_ignore_sigpipe(void);
int u_open(int port);
int u_accept(int fd, char *hostn, int hostnsize);
int u_connect(int port, char *hostn);

#endif /* SOCKETS_H_ */
