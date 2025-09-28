#ifndef EPOLL_SHIM_DETAIL_POLL_H_
#define EPOLL_SHIM_DETAIL_POLL_H_

#include <poll.h>
#include <signal.h>

extern int epoll_shim_poll(struct pollfd *, nfds_t, int);
#ifdef EPOLL_SHIM_NO_VARIADICS
#define poll(fds, nfds, timeout) epoll_shim_poll((fds), (nfds), (timeout))
#else
#define poll(...) epoll_shim_poll(__VA_ARGS__)
#endif

extern int epoll_shim_ppoll(struct pollfd *, nfds_t, struct timespec const *,
    sigset_t const *);
#ifdef EPOLL_SHIM_NO_VARIADICS
#define ppoll(fds, nfds, tmo_p, sigmask) \
	epoll_shim_ppoll((fds), (nfds), (tmo_p), (sigmask))
#else
#define ppoll(...) epoll_shim_ppoll(__VA_ARGS__)
#endif

#endif
