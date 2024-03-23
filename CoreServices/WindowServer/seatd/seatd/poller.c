#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linked_list.h"
#include "poller.h"

struct event_source_fd {
	struct linked_list link; // poller::fds
	const struct event_source_fd_impl *impl;
	event_source_fd_func_t func;

	int fd;
	uint32_t mask;
	void *data;

	struct poller *poller;
	bool killed;
	ssize_t pollfd_idx;
};

struct event_source_signal {
	struct linked_list link; // poller::signals
	const struct event_source_signal_impl *impl;
	event_source_signal_func_t func;

	int signal;
	void *data;

	struct poller *poller;
	bool raised;
	bool killed;
};

/* Used for signal handling */
struct poller *global_poller = NULL;

static void signal_handler(int sig) {
	if (global_poller == NULL) {
		return;
	}
	for (struct linked_list *elem = global_poller->signals.next;
	     elem != &global_poller->signals; elem = elem->next) {
		struct event_source_signal *bps = (struct event_source_signal *)elem;
		if (bps->signal == sig) {
			bps->raised = true;
		}
	}
	int saved_errno = errno;
	if (write(global_poller->signal_fds[1], "\0", 1) == -1 && errno != EAGAIN) {
		// This is unfortunate.
	}
	errno = saved_errno;
}

static int set_nonblock(int fd) {
	int flags;
	if ((flags = fcntl(fd, F_GETFL)) == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}

int poller_init(struct poller *poller) {
	assert(global_poller == NULL);

	if (pipe(poller->signal_fds) == -1) {
		return -1;
	}
	if (set_nonblock(poller->signal_fds[0]) == -1) {
		return -1;
	}
	if (set_nonblock(poller->signal_fds[1]) == -1) {
		return -1;
	}

	linked_list_init(&poller->fds);
	linked_list_init(&poller->signals);
	poller->pollfds = NULL;
	poller->pollfds_len = 0;
	poller->dirty = true;
	poller->fd_event_sources = 1;
	global_poller = poller;

	return 0;
}

int poller_finish(struct poller *poller) {
	while (!linked_list_empty(&poller->fds)) {
		struct event_source_fd *bpfd = (struct event_source_fd *)poller->fds.next;
		linked_list_remove(&bpfd->link);
		free(bpfd);
	}
	while (!linked_list_empty(&poller->signals)) {
		struct event_source_signal *bps = (struct event_source_signal *)poller->signals.next;

		struct sigaction sa;
		sa.sa_handler = SIG_DFL;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(bps->signal, &sa, NULL);

		linked_list_remove(&bps->link);
		free(bps);
	}
	free(poller->pollfds);
	global_poller = NULL;
	return 0;
}

struct event_source_fd *poller_add_fd(struct poller *poller, int fd, uint32_t mask,
				      event_source_fd_func_t func, void *data) {
	struct event_source_fd *bpfd = calloc(1, sizeof(struct event_source_fd));
	if (bpfd == NULL) {
		return NULL;
	}
	bpfd->fd = fd;
	bpfd->mask = mask;
	bpfd->data = data;
	bpfd->func = func;
	bpfd->poller = poller;
	bpfd->pollfd_idx = -1;
	poller->fd_event_sources += 1;
	poller->dirty = true;
	linked_list_insert(&poller->fds, &bpfd->link);
	return (struct event_source_fd *)bpfd;
}

int event_source_fd_destroy(struct event_source_fd *event_source) {
	struct event_source_fd *bpfd = (struct event_source_fd *)event_source;
	struct poller *poller = bpfd->poller;
	poller->fd_event_sources -= 1;
	poller->dirty = true;
	bpfd->killed = true;
	return 0;
}

int event_source_fd_update(struct event_source_fd *event_source, uint32_t mask) {
	struct event_source_fd *bpfd = (struct event_source_fd *)event_source;
	struct poller *poller = bpfd->poller;
	event_source->mask = mask;
	poller->dirty = true;
	return 0;
}

struct event_source_signal *poller_add_signal(struct poller *poller, int signal,
					      event_source_signal_func_t func, void *data) {

	struct event_source_signal *bps = calloc(1, sizeof(struct event_source_signal));
	if (bps == NULL) {
		return NULL;
	}

	bps->signal = signal;
	bps->data = data;
	bps->func = func;
	bps->poller = poller;

	struct sigaction sa;
	sa.sa_handler = &signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(signal, &sa, NULL);

	linked_list_insert(&poller->signals, &bps->link);

	return (struct event_source_signal *)bps;
}

int event_source_signal_destroy(struct event_source_signal *event_source) {
	struct event_source_signal *bps = (struct event_source_signal *)event_source;
	struct poller *poller = bps->poller;

	int refcnt = 0;
	for (struct linked_list *elem = poller->signals.next; elem != &poller->signals;
	     elem = elem->next) {
		struct event_source_signal *b = (struct event_source_signal *)elem;
		if (b->signal == bps->signal && !b->killed) {
			refcnt++;
		}
	}

	if (refcnt == 0) {
		struct sigaction sa;
		sa.sa_handler = SIG_DFL;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(bps->signal, &sa, NULL);
	}

	poller->dirty = true;
	bps->killed = true;
	return 0;
}

static int event_mask_to_poll_mask(uint32_t event_mask) {
	int poll_mask = 0;
	if (event_mask & EVENT_READABLE) {
		poll_mask |= POLLIN;
	}
	if (event_mask & EVENT_WRITABLE) {
		poll_mask |= POLLOUT;
	}
	return poll_mask;
}

static uint32_t poll_mask_to_event_mask(int poll_mask) {
	uint32_t event_mask = 0;
	if (poll_mask & POLLIN) {
		event_mask |= EVENT_READABLE;
	}
	if (poll_mask & POLLOUT) {
		event_mask |= EVENT_WRITABLE;
	}
	if (poll_mask & POLLERR) {
		event_mask |= EVENT_ERROR;
	}
	if (poll_mask & POLLHUP) {
		event_mask |= EVENT_HANGUP;
	}
	return event_mask;
}

static int regenerate(struct poller *poller) {
	if (poller->fd_event_sources > poller->pollfds_len) {
		struct pollfd *fds =
			realloc(poller->pollfds, poller->fd_event_sources * sizeof(struct pollfd));
		if (fds == NULL) {
			return -1;
		}
		poller->pollfds = fds;
		poller->pollfds_len = poller->fd_event_sources;
	}

	size_t idx = 0;
	poller->pollfds[idx++] = (struct pollfd){
		.fd = poller->signal_fds[0],
		.events = POLLIN,
	};

	for (struct linked_list *elem = poller->fds.next; elem != &poller->fds; elem = elem->next) {
		struct event_source_fd *bpfd = (struct event_source_fd *)elem;
		if (bpfd->killed) {
			elem = elem->prev;
			linked_list_remove(&bpfd->link);
			free(bpfd);
		} else {
			bpfd->pollfd_idx = idx++;
			assert(bpfd->pollfd_idx < (ssize_t)poller->pollfds_len);
			poller->pollfds[bpfd->pollfd_idx] = (struct pollfd){
				.fd = bpfd->fd,
				.events = event_mask_to_poll_mask(bpfd->mask),
			};
		}
	}
	assert(idx == poller->fd_event_sources);

	for (struct linked_list *elem = poller->signals.next; elem != &poller->signals;
	     elem = elem->next) {
		struct event_source_signal *bps = (struct event_source_signal *)elem;
		if (bps->killed) {
			elem = elem->prev;
			linked_list_remove(&bps->link);
			free(bps);
		}
	}

	return 0;
}

static void dispatch(struct poller *poller) {
	if ((poller->pollfds[0].revents & POLLIN) != 0) {
		char garbage[8];
		while (read(poller->signal_fds[0], &garbage, sizeof garbage) != -1) {
		}

		for (struct linked_list *elem = poller->signals.next; elem != &poller->signals;
		     elem = elem->next) {
			struct event_source_signal *bps = (struct event_source_signal *)elem;
			if (!bps->raised || bps->killed) {
				continue;
			}
			bps->func(bps->signal, bps->data);
			bps->raised = false;
		}
	}

	for (struct linked_list *elem = poller->fds.next; elem != &poller->fds; elem = elem->next) {
		struct event_source_fd *bpfd = (struct event_source_fd *)elem;
		if (bpfd->pollfd_idx == -1 || bpfd->killed) {
			continue;
		}
		assert(bpfd->pollfd_idx < (ssize_t)poller->pollfds_len);
		short revents = poller->pollfds[bpfd->pollfd_idx].revents;
		if (revents == 0) {
			continue;
		}
		bpfd->func(poller->pollfds[bpfd->pollfd_idx].fd, poll_mask_to_event_mask(revents),
			   bpfd->data);
	}
}

int poller_poll(struct poller *poller) {
	if (poller->dirty) {
		if (regenerate(poller) == -1) {
			return -1;
		}
		poller->dirty = false;
	}

	while (poll(poller->pollfds, poller->fd_event_sources, -1) == -1) {
		if (errno != EINTR) {
			return -1;
		}
	}

	dispatch(poller);

	return 0;
}
