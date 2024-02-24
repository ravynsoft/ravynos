#include "compat_socket.h"

#include <errno.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "wrap.h"

static errno_t
compat_socket_impl(int domain, int type, int protocol, int *s)
{
	errno_t ec;

	int sock = socket(domain,
	    type & ~(COMPAT_SOCK_CLOEXEC | COMPAT_SOCK_NONBLOCK), protocol);
	if (sock < 0) {
		return errno;
	}

	{
		int r;

		if (type & COMPAT_SOCK_NONBLOCK) {
			if ((r = fcntl(sock, F_GETFL, 0)) < 0 ||
			    fcntl(sock, F_SETFL, r | O_NONBLOCK) < 0) {
				ec = errno;
				goto out;
			}
		}

		if (type & COMPAT_SOCK_CLOEXEC) {
			if ((r = fcntl(sock, F_GETFD, 0)) < 0 ||
			    fcntl(sock, F_SETFD, r | FD_CLOEXEC) < 0) {
				ec = errno;
				goto out;
			}
		}
	}

	*s = sock;
	return 0;

out:
	(void)real_close(sock);
	return ec;
}

int
compat_socket(int domain, int type, int protocol)
{
	int s;
	errno_t ec = compat_socket_impl(domain, type, protocol, &s);
	if (ec != 0) {
		errno = ec;
		return -1;
	}
	return s;
}
