#ifndef WRAP_H_
#define WRAP_H_

#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

ssize_t real_read(int fd, void *buf, size_t nbytes);
ssize_t real_write(int fd, void const *buf, size_t nbytes);
int real_close(int fd);
int real_poll(struct pollfd fds[], nfds_t nfds, int timeout);
int real_ppoll(struct pollfd fds[], nfds_t nfds,
    struct timespec const *restrict timeout,
    sigset_t const *restrict newsigmask);
int real_fcntl(int fd, int cmd, ...);

#endif
