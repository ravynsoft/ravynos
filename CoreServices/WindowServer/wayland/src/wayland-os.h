/*
 * Copyright © 2012 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WAYLAND_OS_H
#define WAYLAND_OS_H

#include <sys/types.h>
#include <sys/socket.h>

int
wl_os_socket_cloexec(int domain, int type, int protocol);

int
wl_os_socket_peercred(int sockfd, uid_t *uid, gid_t *gid, pid_t *pid);

int
wl_os_dupfd_cloexec(int fd, int minfd);

ssize_t
wl_os_recvmsg_cloexec(int sockfd, struct msghdr *msg, int flags);

int
wl_os_epoll_create_cloexec(void);

int
wl_os_accept_cloexec(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

void *
wl_os_mremap_maymove(int fd, void *old_data, ssize_t *old_size,
		     ssize_t new_size, int prot, int flags);


/*
 * The following are for wayland-os.c and the unit tests.
 * Do not use them elsewhere.
 */

#ifdef __linux__

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 02000000
#endif

#ifndef F_DUPFD_CLOEXEC
#define F_DUPFD_CLOEXEC 1030
#endif

#ifndef MSG_CMSG_CLOEXEC
#define MSG_CMSG_CLOEXEC 0x40000000
#endif

#endif /* __linux__ */

#endif
