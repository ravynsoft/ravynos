#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend.h"
#include "libseat.h"
#include "log.h"

extern const struct seat_impl seatd_impl;
extern const struct seat_impl logind_impl;
extern const struct seat_impl builtin_impl;
extern const struct seat_impl noop_impl;

static const struct named_backend impls[] = {
#ifdef SEATD_ENABLED
	{"seatd", &seatd_impl},
#endif
#ifdef LOGIND_ENABLED
	{"logind", &logind_impl},
#endif
#ifdef BUILTIN_ENABLED
	{"builtin", &builtin_impl},
#endif
	{"noop", &noop_impl},

	{NULL, NULL},
};

#if !defined(SEATD_ENABLED) && !defined(LOGIND_ENABLED) && !defined(BUILTIN_ENABLED)
#error At least one backend must be enabled
#endif

struct libseat *libseat_open_seat(const struct libseat_seat_listener *listener, void *data) {
	if (listener == NULL || listener->enable_seat == NULL || listener->disable_seat == NULL) {
		errno = EINVAL;
		return NULL;
	}

	log_init();

	char *backend_type = getenv("LIBSEAT_BACKEND");
	if (backend_type != NULL) {
		const struct named_backend *iter = impls;
		while (iter->backend != NULL && strcmp(backend_type, iter->name) != 0) {
			iter++;
		}
		if (iter == NULL || iter->backend == NULL) {
			log_errorf("No backend matched name '%s'", backend_type);
			errno = EINVAL;
			return NULL;
		}
		struct libseat *backend = iter->backend->open_seat(listener, data);
		if (backend == NULL) {
			log_errorf("Backend '%s' failed to open seat: %s", iter->name,
				   strerror(errno));
			return NULL;
		}
		log_infof("Seat opened with backend '%s'", iter->name);
		return backend;
	}

	struct libseat *backend = NULL;
	for (const struct named_backend *iter = impls; iter->backend != NULL; iter++) {
		if (iter->backend == &noop_impl) {
			continue;
		}
		backend = iter->backend->open_seat(listener, data);
		if (backend != NULL) {
			log_infof("Seat opened with backend '%s'", iter->name);
			return backend;
		}
		log_infof("Backend '%s' failed to open seat, skipping", iter->name);
	}

	log_error("No backend was able to open a seat");
	errno = ENOSYS;
	return NULL;
}

int libseat_disable_seat(struct libseat *seat) {
	assert(seat && seat->impl);
	return seat->impl->disable_seat(seat);
}

int libseat_close_seat(struct libseat *seat) {
	assert(seat && seat->impl);
	return seat->impl->close_seat(seat);
}

const char *libseat_seat_name(struct libseat *seat) {
	assert(seat && seat->impl);
	return seat->impl->seat_name(seat);
}

int libseat_open_device(struct libseat *seat, const char *path, int *fd) {
	assert(seat && seat->impl);
	return seat->impl->open_device(seat, path, fd);
}

int libseat_close_device(struct libseat *seat, int device_id) {
	assert(seat && seat->impl);
	return seat->impl->close_device(seat, device_id);
}

int libseat_get_fd(struct libseat *seat) {
	assert(seat && seat->impl);
	return seat->impl->get_fd(seat);
}

int libseat_dispatch(struct libseat *seat, int timeout) {
	assert(seat && seat->impl);
	return seat->impl->dispatch(seat, timeout);
}

int libseat_switch_session(struct libseat *seat, int session) {
	assert(seat && seat->impl);
	return seat->impl->switch_session(seat, session);
}
