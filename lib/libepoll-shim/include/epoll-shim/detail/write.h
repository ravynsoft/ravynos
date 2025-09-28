#ifndef EPOLL_SHIM_DETAIL_WRITE_H_
#define EPOLL_SHIM_DETAIL_WRITE_H_

#include <unistd.h>

extern ssize_t epoll_shim_write(int, void const *, size_t);
#ifdef EPOLL_SHIM_NO_VARIADICS
#define write(fd, buf, count) epoll_shim_write((fd), (buf), (count))
#else
#define write(...) epoll_shim_write(__VA_ARGS__)
#endif

#endif
