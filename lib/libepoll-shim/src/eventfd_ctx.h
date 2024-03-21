#ifndef EVENTFD_CTX_H_
#define EVENTFD_CTX_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>

#include "kqueue_event.h"

#define EVENTFD_CTX_FLAG_SEMAPHORE (1 << 0)

typedef struct {
	int flags_;

	KQueueEvent kqueue_event_;
	uint_least64_t counter_;
} EventFDCtx;

errno_t eventfd_ctx_init(EventFDCtx *eventfd, int kq, unsigned int counter,
    int flags);
errno_t eventfd_ctx_terminate(EventFDCtx *eventfd);

errno_t eventfd_ctx_write(EventFDCtx *eventfd, int kq, uint64_t value);
errno_t eventfd_ctx_read(EventFDCtx *eventfd, int kq, uint64_t *value);

#endif
