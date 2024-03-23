#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "backend.h"
#include "log.h"

struct backend_noop {
	struct libseat base;
	const struct libseat_seat_listener *seat_listener;
	void *seat_listener_data;

	bool initial_setup;
	int sockets[2];
};

extern const struct seat_impl noop_impl;

static struct backend_noop *backend_noop_from_libseat_backend(struct libseat *base) {
	assert(base->impl == &noop_impl);
	return (struct backend_noop *)base;
}

static void destroy(struct backend_noop *backend) {
	close(backend->sockets[0]);
	close(backend->sockets[1]);
	free(backend);
}

static int close_seat(struct libseat *base) {
	struct backend_noop *backend = backend_noop_from_libseat_backend(base);
	destroy(backend);
	return 0;
}

static int disable_seat(struct libseat *base) {
	(void)base;
	return 0;
}

static const char *seat_name(struct libseat *base) {
	(void)base;
	return "seat0";
}

static int open_device(struct libseat *base, const char *path, int *fd) {
	(void)base;

	int tmpfd = open(path, O_RDWR | O_NOCTTY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (tmpfd < 0) {
		log_errorf("Failed to open device: %s", strerror(errno));
		return -1;
	}

	*fd = tmpfd;
	return tmpfd;
}

static int close_device(struct libseat *base, int device_id) {
	(void)base;
	(void)device_id;
	return 0;
}

static int switch_session(struct libseat *base, int s) {
	(void)base;
	(void)s;
	log_errorf("No-op backend cannot switch to session %d", s);
	return -1;
}

static int get_fd(struct libseat *base) {
	struct backend_noop *backend = backend_noop_from_libseat_backend(base);
	return backend->sockets[0];
}

static int dispatch_background(struct libseat *base, int timeout) {
	struct backend_noop *backend = backend_noop_from_libseat_backend(base);

	if (backend->initial_setup) {
		backend->initial_setup = false;
		backend->seat_listener->enable_seat(&backend->base, backend->seat_listener_data);
	}

	struct pollfd fd = {
		.fd = backend->sockets[0],
		.events = POLLIN,
	};
	if (poll(&fd, 1, timeout) < 0) {
		if (errno == EAGAIN || errno == EINTR) {
			return 0;
		} else {
			return -1;
		}
	}

	return 0;
}

static struct libseat *noop_open_seat(const struct libseat_seat_listener *listener, void *data) {
	struct backend_noop *backend = calloc(1, sizeof(struct backend_noop));
	if (backend == NULL) {
		return NULL;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, backend->sockets) != 0) {
		log_errorf("socketpair() failed: %s", strerror(errno));
		free(backend);
		return NULL;
	}

	backend->initial_setup = true;
	backend->seat_listener = listener;
	backend->seat_listener_data = data;
	backend->base.impl = &noop_impl;

	return &backend->base;
}

const struct seat_impl noop_impl = {
	.open_seat = noop_open_seat,
	.disable_seat = disable_seat,
	.close_seat = close_seat,
	.seat_name = seat_name,
	.open_device = open_device,
	.close_device = close_device,
	.switch_session = switch_session,
	.get_fd = get_fd,
	.dispatch = dispatch_background,
};
