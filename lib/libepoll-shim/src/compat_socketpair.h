#ifndef COMPAT_SOCKETPAIR_H
#define COMPAT_SOCKETPAIR_H

#include <sys/types.h>

#include <sys/socket.h>

#define COMPAT_SOCK_NONBLOCK (O_NONBLOCK)
#define COMPAT_SOCK_CLOEXEC (O_CLOEXEC)
int compat_socketpair(int domain, int type, int protocol, int sv[2]);
#ifdef COMPAT_ENABLE_SOCKETPAIR
#define socketpair compat_socketpair
#define SOCK_NONBLOCK COMPAT_SOCK_NONBLOCK
#define SOCK_CLOEXEC COMPAT_SOCK_CLOEXEC
#endif

#endif
