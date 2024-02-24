#ifndef COMPAT_SOCKET_H
#define COMPAT_SOCKET_H

#include <sys/types.h>

#include <sys/socket.h>

#define COMPAT_SOCK_NONBLOCK (O_NONBLOCK)
#define COMPAT_SOCK_CLOEXEC (O_CLOEXEC)
int compat_socket(int domain, int type, int protocol);
#ifdef COMPAT_ENABLE_SOCKET
#define socket compat_socket
#define SOCK_NONBLOCK COMPAT_SOCK_NONBLOCK
#define SOCK_CLOEXEC COMPAT_SOCK_CLOEXEC
#endif

#endif
