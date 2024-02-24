#include "compat_pipe2.h"

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>

#include "wrap.h"

static errno_t
compat_pipe2_impl(int pipefd[2], int flags)
{
	errno_t ec;

	if (flags & ~(O_CLOEXEC | O_NONBLOCK)) {
		return EINVAL;
	}

	int p[2];
	if (pipe(p) < 0) {
		return errno;
	}

	{
		int r;

		if (flags & O_NONBLOCK) {
			if ((r = fcntl(p[0], F_GETFL, 0)) < 0 ||
			    fcntl(p[0], F_SETFL, r | O_NONBLOCK) < 0 ||
			    (r = fcntl(p[1], F_GETFL, 0)) < 0 ||
			    fcntl(p[1], F_SETFL, r | O_NONBLOCK) < 0) {
				ec = errno;
				goto out;
			}
		}

		if (flags & O_CLOEXEC) {
			if ((r = fcntl(p[0], F_GETFD, 0)) < 0 ||
			    fcntl(p[0], F_SETFD, r | FD_CLOEXEC) < 0 ||
			    (r = fcntl(p[1], F_GETFD, 0)) < 0 ||
			    fcntl(p[1], F_SETFD, r | FD_CLOEXEC) < 0) {
				ec = errno;
				goto out;
			}
		}
	}

	pipefd[0] = p[0];
	pipefd[1] = p[1];

	return 0;

out:
	(void)real_close(p[0]);
	(void)real_close(p[1]);
	return ec;
}

int
compat_pipe2(int pipefd[2], int flags)
{
	errno_t ec = compat_pipe2_impl(pipefd, flags);
	if (ec != 0) {
		errno = ec;
		return -1;
	}
	return 0;
}
