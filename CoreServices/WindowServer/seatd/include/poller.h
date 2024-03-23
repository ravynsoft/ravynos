#ifndef _SEATD_POLLER_H
#define _SEATD_POLLER_H

#include <stdbool.h>
#include <stdint.h>

#include "linked_list.h"

/*
 * These are the event types available from the poller.
 */
#define EVENT_READABLE 0x1
#define EVENT_WRITABLE 0x4
#define EVENT_ERROR    0x8
#define EVENT_HANGUP   0x10

/**
 * The fd poller class. This must be created by poller_add_fd.
 */
struct event_source_fd;

/*
 * The signal poller class. This must be created by poller_add_signal.
 */
struct event_source_signal;

/**
 * The callback type used by event_source_fd, passed to poller_add_fd.
 */
typedef int (*event_source_fd_func_t)(int fd, uint32_t mask, void *data);

/**
 * The interface that an event_source_fd must implement.
 */
struct event_source_fd_impl {
	int (*update)(struct event_source_fd *event_source, uint32_t mask);
	int (*destroy)(struct event_source_fd *event_source);
};

/**
 * Removes the event_source_fd from the poller and frees the structure.
 */
int event_source_fd_destroy(struct event_source_fd *event_source);

/**
 * Updates the poll mask applied to this fd, effective on the next poll.
 */
int event_source_fd_update(struct event_source_fd *event_source, uint32_t mask);

/**
 * The callback type used by event_source_signal, passed to poller_add_signal.
 */
typedef int (*event_source_signal_func_t)(int signal, void *data);

/**
 * The interface that an event_source_signal must implement.
 */
struct event_source_signal_impl {
	int (*destroy)(struct event_source_signal *event_source);
};

/**
 * Removes the event_source_siganl from the poller and frees the structure.
 */
int event_source_signal_destroy(struct event_source_signal *event_source);

/**
 * The poller base class. This must be created by poller_create.
 */
struct poller {
	struct linked_list signals;
	struct linked_list fds;

	int signal_fds[2];
	struct pollfd *pollfds;
	size_t pollfds_len;
	size_t fd_event_sources;
	bool dirty;
};

/**
 * The interface that a poll backend must implement.
 */
struct poll_impl {
	struct poller *(*create)(void);
	int (*destroy)(struct poller *);

	struct event_source_fd *(*add_fd)(struct poller *, int fd, uint32_t mask,
					  event_source_fd_func_t func, void *data);
	struct event_source_signal *(*add_signal)(struct poller *, int signal,
						  event_source_signal_func_t func, void *data);

	int (*poll)(struct poller *);
};

/**
 * Initializes the poller. The poller must be torn down with poller_finish when
 * it is no longer needed.
 */
int poller_init(struct poller *poller);

/**
 * De-initializes the poller. This destroys all remaining event sources and
 * tears down the poller.
 */
int poller_finish(struct poller *poller);

/**
 * Create an fd event source with the provided initial parameters. This event
 * source must be torn down with event_source_fd_destroy when it is no longer
 * needed.
 */
struct event_source_fd *poller_add_fd(struct poller *poller, int fd, uint32_t mask,
				      event_source_fd_func_t func, void *data);

/**
 * Create  signal event source with the provided initial parameters. This event
 * source must be torn down with event_source_signal_destroy when it is no
 * longer needed.
 */
struct event_source_signal *poller_add_signal(struct poller *poller, int signal,
					      event_source_signal_func_t func, void *data);

/**
 * Poll the poller. I don't know what you were expecting.
 */
int poller_poll(struct poller *poller);

#endif
