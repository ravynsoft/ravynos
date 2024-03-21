#include "compat_socketpair.h"

#include <errno.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "wrap.h"

static errno_t
compat_socketpair_impl(int domain, int type, int protocol, int sv[2])
{
	errno_t ec;

	int p[2];
	if (socketpair(domain,
		type & ~(COMPAT_SOCK_CLOEXEC | COMPAT_SOCK_NONBLOCK), protocol,
		p) < 0) {
		return errno;
	}

	{
		int r;

		if (type & COMPAT_SOCK_NONBLOCK) {
			if ((r = fcntl(p[0], F_GETFL, 0)) < 0 ||
			    fcntl(p[0], F_SETFL, r | O_NONBLOCK) < 0 ||
			    (r = fcntl(p[1], F_GETFL, 0)) < 0 ||
			    fcntl(p[1], F_SETFL, r | O_NONBLOCK) < 0) {
				ec = errno;
				goto out;
			}
		}

		if (type & COMPAT_SOCK_CLOEXEC) {
			if ((r = fcntl(p[0], F_GETFD, 0)) < 0 ||
			    fcntl(p[0], F_SETFD, r | FD_CLOEXEC) < 0 ||
			    (r = fcntl(p[1], F_GETFD, 0)) < 0 ||
			    fcntl(p[1], F_SETFD, r | FD_CLOEXEC) < 0) {
				ec = errno;
				goto out;
			}
		}
	}

	sv[0] = p[0];
	sv[1] = p[1];

	return 0;

out:
	(void)real_close(p[0]);
	(void)real_close(p[1]);
	return ec;
}

int
compat_socketpair(int domain, int type, int protocol, int sv[2])
{
	errno_t ec = compat_socketpair_impl(domain, type, protocol, sv);
	if (ec != 0) {
		errno = ec;
		return -1;
	}
	return 0;
}
