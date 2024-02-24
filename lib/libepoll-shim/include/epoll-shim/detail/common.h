#ifndef EPOLL_SHIM_DETAIL_COMMON_H_
#define EPOLL_SHIM_DETAIL_COMMON_H_

#include <fcntl.h>
#include <unistd.h>

#if defined(__STRICT_ANSI__) && /**/                    \
    !defined(__GXX_EXPERIMENTAL_CXX0X__) &&             \
    (!defined(__cplusplus) || __cplusplus < 201103L) && \
    (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L)
#define EPOLL_SHIM_NO_VARIADICS
#endif

extern int epoll_shim_close(int);
#ifdef EPOLL_SHIM_NO_VARIADICS
#define close(fd) epoll_shim_close((fd))
#else
#define close(...) epoll_shim_close(__VA_ARGS__)
#endif

extern int epoll_shim_fcntl(int, int, ...);
#ifdef EPOLL_SHIM_NO_VARIADICS
#define fcntl epoll_shim_fcntl
#else
#define fcntl(...) epoll_shim_fcntl(__VA_ARGS__)
#endif

#endif
