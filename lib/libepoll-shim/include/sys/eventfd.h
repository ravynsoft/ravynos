#ifndef EPOLL_SHIM_SYS_EVENTFD_H_
#define EPOLL_SHIM_SYS_EVENTFD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <fcntl.h>

typedef uint64_t eventfd_t;

#define EFD_SEMAPHORE 1
#define EFD_CLOEXEC O_CLOEXEC
#define EFD_NONBLOCK O_NONBLOCK

int eventfd(unsigned int, int);
int eventfd_read(int, eventfd_t *);
int eventfd_write(int, eventfd_t);


#ifndef EPOLL_SHIM_DISABLE_WRAPPER_MACROS
#include <epoll-shim/detail/common.h>
#include <epoll-shim/detail/read.h>
#include <epoll-shim/detail/write.h>
#endif


#ifdef __cplusplus
}
#endif

#endif /* sys/eventfd.h */
