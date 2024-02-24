#ifndef EPOLL_SHIM_DETAIL_READ_H_
#define EPOLL_SHIM_DETAIL_READ_H_

#include <unistd.h>

extern ssize_t epoll_shim_read(int, void *, size_t);
#ifdef EPOLL_SHIM_NO_VARIADICS
#define read(fd, buf, count) epoll_shim_read((fd), (buf), (count))
#else
#define read(...) epoll_shim_read(__VA_ARGS__)
#endif

#endif
