#ifndef COMPAT_PPOLL_H
#define COMPAT_PPOLL_H

#include <poll.h>
#include <signal.h>
#include <time.h>

int compat_ppoll(struct pollfd fds[], nfds_t nfds,
    struct timespec const *restrict timeout, sigset_t const *restrict sigmask);
#ifdef COMPAT_ENABLE_PPOLL
#define ppoll compat_ppoll
#endif

#endif
