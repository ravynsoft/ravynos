#include "compat_ppoll.h"

#include <sys/types.h>

#include <sys/event.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "wrap.h"

static errno_t
compat_ppoll_impl(struct pollfd fds[], nfds_t nfds,
    struct timespec const *restrict timeout, sigset_t const *restrict sigmask,
    int *n)
{
	errno_t ec;

	int timeout_ms = -1;
	if (timeout != NULL) {
		if (timeout->tv_sec < 0 || timeout->tv_nsec < 0 ||
		    timeout->tv_nsec >= 1000000000) {
			return EINVAL;
		}

		timeout_ms = timeout->tv_nsec / 1000000;
		if (timeout->tv_nsec % 1000000) {
			++timeout_ms;
		}

		int sec_ms;
		if (__builtin_mul_overflow(timeout->tv_sec, 1000, &sec_ms) ||
		    __builtin_add_overflow(sec_ms, timeout_ms, &timeout_ms)) {
			timeout_ms = -1;
		}
	}

	struct kevent kevs[NSIG];
	int kevs_length = 0;

	sigset_t origmask;

	if (timeout_ms != 0 && sigmask != NULL) {
		if ((ec = pthread_sigmask(SIG_SETMASK, /**/
			 NULL, &origmask)) != 0) {
			return ec;
		}

		for (int i = 1; i < NSIG; ++i) {
			if (i == SIGKILL || i == SIGSTOP) {
				continue;
			}

			if (!sigismember(&origmask, i) ||
			    sigismember(sigmask, i)) {
				continue;
			}

			struct sigaction act;
			if (sigaction(i, NULL, &act) < 0) {
				return errno;
			}

			if (!(act.sa_flags & SA_SIGINFO) &&
			    (act.sa_handler == SIG_DFL ||
				act.sa_handler == SIG_IGN)) {
				continue;
			}

			if (act.sa_flags & SA_RESTART) {
				continue;
			}

			EV_SET(&kevs[kevs_length++], i, EVFILT_SIGNAL,
			    EV_ADD | EV_ONESHOT, 0, 0, 0);
		}
	}

	int kq = -1;
	struct pollfd *fds2 = NULL;

	if (kevs_length > 0) {
		kq = kqueue();
		if (kq < 0) {
			return errno;
		}

		if (kevent(kq, kevs, kevs_length, NULL, 0, NULL) < 0) {
			ec = errno;
			goto out;
		}

#ifdef EVFILT_USER
		sigset_t pending;
		if (sigpending(&pending) < 0) {
			ec = errno;
			goto out;
		}
		for (int i = 0; i < kevs_length; ++i) {
			if (sigismember(&pending, (int)kevs[i].ident)) {
				EV_SET(&kevs[0], 0, EVFILT_USER, /**/
				    EV_ADD | EV_ONESHOT, 0, 0, 0);
				EV_SET(&kevs[1], 0, EVFILT_USER, /**/
				    0, NOTE_TRIGGER, 0, 0);
				if (kevent(kq, kevs, 2, NULL, 0, NULL) < 0) {
					ec = errno;
					goto out;
				}
				break;
			}
		}
#endif

		size_t pfds2_bytes;
		if (__builtin_add_overflow(nfds, 1, &pfds2_bytes) ||
		    __builtin_mul_overflow(pfds2_bytes, sizeof(struct pollfd),
			&pfds2_bytes)) {
			ec = EINVAL;
			goto out;
		}

		fds2 = malloc(pfds2_bytes);
		if (!fds2) {
			ec = errno;
			goto out;
		}
		memcpy(fds2, fds, nfds * sizeof(struct pollfd));
		fds2[nfds] = (struct pollfd) {
			.fd = kq,
			.events = POLLIN,
		};
	}

	if (sigmask != NULL) {
		(void)pthread_sigmask(SIG_SETMASK, sigmask, &origmask);
	}

	int ready = real_poll(fds2 ? fds2 : fds, nfds + !!(fds2), timeout_ms);
	ec = ready < 0 ? errno : 0;

	if (sigmask != NULL) {
		(void)pthread_sigmask(SIG_SETMASK, &origmask, NULL);
	}

	if (ec == 0 && fds2 && fds2[nfds].revents) {
		--ready;
		if (ready == 0) {
			ec = EINTR;
		}
	}

	if (ec == 0) {
		*n = ready;
		if (fds2) {
			memcpy(fds, fds2, nfds * sizeof(struct pollfd));
		}
	}

out:
	free(fds2);
	if (kq >= 0) {
		real_close(kq);
	}
	return ec;
}

int
compat_ppoll(struct pollfd fds[], nfds_t nfds,
    struct timespec const *restrict timeout, sigset_t const *restrict sigmask)
{
	int n;
	errno_t ec = compat_ppoll_impl(fds, nfds, timeout, sigmask, &n);
	if (ec != 0) {
		errno = ec;
		return -1;
	}
	return n;
}
