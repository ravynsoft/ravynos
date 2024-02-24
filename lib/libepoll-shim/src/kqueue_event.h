#ifndef KQUEUE_EVENT_H
#define KQUEUE_EVENT_H

#include <sys/types.h>

#include <sys/event.h>

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
	bool is_triggered_;
	int self_pipe_[2]; /* only used if EVFILT_USER is not available */
} KQueueEvent;

errno_t kqueue_event_init(KQueueEvent *kqueue_event, struct kevent *kevs,
    int *kevs_length, bool should_trigger);
errno_t kqueue_event_terminate(KQueueEvent *kqueue_event);

bool kqueue_event_is_triggered(KQueueEvent *kqueue_event);

errno_t kqueue_event_trigger(KQueueEvent *kqueue_event, int kq);
void kqueue_event_clear(KQueueEvent *kqueue_event, int kq);

#endif
