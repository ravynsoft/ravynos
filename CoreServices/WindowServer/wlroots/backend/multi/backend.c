#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <wlr/backend/interface.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/util/log.h>
#include "backend/backend.h"
#include "backend/multi.h"
#include "util/signal.h"

struct subbackend_state {
	struct wlr_backend *backend;
	struct wlr_backend *container;
	struct wl_listener new_input;
	struct wl_listener new_output;
	struct wl_listener destroy;
	struct wl_list link;
};

static struct wlr_multi_backend *multi_backend_from_backend(
		struct wlr_backend *wlr_backend) {
	assert(wlr_backend_is_multi(wlr_backend));
	return (struct wlr_multi_backend *)wlr_backend;
}

static bool multi_backend_start(struct wlr_backend *wlr_backend) {
	struct wlr_multi_backend *backend = multi_backend_from_backend(wlr_backend);
	struct subbackend_state *sub;
	wl_list_for_each(sub, &backend->backends, link) {
		if (!wlr_backend_start(sub->backend)) {
			wlr_log(WLR_ERROR, "Failed to initialize backend.");
			return false;
		}
	}
	return true;
}

static void subbackend_state_destroy(struct subbackend_state *sub) {
	wl_list_remove(&sub->new_input.link);
	wl_list_remove(&sub->new_output.link);
	wl_list_remove(&sub->destroy.link);
	wl_list_remove(&sub->link);
	free(sub);
}

static void multi_backend_destroy(struct wlr_backend *wlr_backend) {
	struct wlr_multi_backend *backend = multi_backend_from_backend(wlr_backend);

	wl_list_remove(&backend->display_destroy.link);

	// Some backends may depend on other backends, ie. destroying a backend may
	// also destroy other backends
	while (!wl_list_empty(&backend->backends)) {
		struct subbackend_state *sub =
			wl_container_of(backend->backends.next, sub, link);
		wlr_backend_destroy(sub->backend);
	}

	// Destroy this backend only after removing all sub-backends
	wlr_backend_finish(wlr_backend);
	free(backend);
}

static struct wlr_session *multi_backend_get_session(
		struct wlr_backend *_backend) {
	struct wlr_multi_backend *backend = multi_backend_from_backend(_backend);
	return backend->session;
}

static clockid_t multi_backend_get_presentation_clock(
		struct wlr_backend *backend) {
	struct wlr_multi_backend *multi = multi_backend_from_backend(backend);

	struct subbackend_state *sub;
	wl_list_for_each(sub, &multi->backends, link) {
		if (sub->backend->impl->get_presentation_clock) {
			return wlr_backend_get_presentation_clock(sub->backend);
		}
	}

	return CLOCK_MONOTONIC;
}

static int multi_backend_get_drm_fd(struct wlr_backend *backend) {
	struct wlr_multi_backend *multi = multi_backend_from_backend(backend);

	struct subbackend_state *sub;
	wl_list_for_each(sub, &multi->backends, link) {
		if (sub->backend->impl->get_drm_fd) {
			return wlr_backend_get_drm_fd(sub->backend);
		}
	}

	return -1;
}

static uint32_t multi_backend_get_buffer_caps(struct wlr_backend *backend) {
	struct wlr_multi_backend *multi = multi_backend_from_backend(backend);

	if (wl_list_empty(&multi->backends)) {
		return 0;
	}

	uint32_t caps = WLR_BUFFER_CAP_DATA_PTR | WLR_BUFFER_CAP_DMABUF
			| WLR_BUFFER_CAP_SHM;

	struct subbackend_state *sub;
	wl_list_for_each(sub, &multi->backends, link) {
		uint32_t backend_caps = backend_get_buffer_caps(sub->backend);
		if (backend_caps != 0) {
			// only count backend capable of presenting a buffer
			caps = caps & backend_caps;
		}
	}

	return caps;
}

static const struct wlr_backend_impl backend_impl = {
	.start = multi_backend_start,
	.destroy = multi_backend_destroy,
	.get_session = multi_backend_get_session,
	.get_presentation_clock = multi_backend_get_presentation_clock,
	.get_drm_fd = multi_backend_get_drm_fd,
	.get_buffer_caps = multi_backend_get_buffer_caps,
};

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_multi_backend *backend =
		wl_container_of(listener, backend, display_destroy);
	multi_backend_destroy((struct wlr_backend*)backend);
}

struct wlr_backend *wlr_multi_backend_create(struct wl_display *display) {
	struct wlr_multi_backend *backend =
		calloc(1, sizeof(struct wlr_multi_backend));
	if (!backend) {
		wlr_log(WLR_ERROR, "Backend allocation failed");
		return NULL;
	}

	wl_list_init(&backend->backends);
	wlr_backend_init(&backend->backend, &backend_impl);

	wl_signal_init(&backend->events.backend_add);
	wl_signal_init(&backend->events.backend_remove);

	backend->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &backend->display_destroy);

	return &backend->backend;
}

bool wlr_backend_is_multi(struct wlr_backend *b) {
	return b->impl == &backend_impl;
}

static void new_input_reemit(struct wl_listener *listener, void *data) {
	struct subbackend_state *state = wl_container_of(listener,
			state, new_input);
	wlr_signal_emit_safe(&state->container->events.new_input, data);
}

static void new_output_reemit(struct wl_listener *listener, void *data) {
	struct subbackend_state *state = wl_container_of(listener,
			state, new_output);
	wlr_signal_emit_safe(&state->container->events.new_output, data);
}

static void handle_subbackend_destroy(struct wl_listener *listener,
		void *data) {
	struct subbackend_state *state = wl_container_of(listener, state, destroy);
	subbackend_state_destroy(state);
}

static struct subbackend_state *multi_backend_get_subbackend(struct wlr_multi_backend *multi,
		struct wlr_backend *backend) {
	struct subbackend_state *sub = NULL;
	wl_list_for_each(sub, &multi->backends, link) {
		if (sub->backend == backend) {
			return sub;
		}
	}
	return NULL;
}

bool wlr_multi_backend_add(struct wlr_backend *_multi,
		struct wlr_backend *backend) {
	assert(_multi && backend);
	assert(_multi != backend);

	struct wlr_multi_backend *multi = multi_backend_from_backend(_multi);

	if (multi_backend_get_subbackend(multi, backend)) {
		// already added
		return true;
	}

	struct subbackend_state *sub = calloc(1, sizeof(struct subbackend_state));
	if (sub == NULL) {
		wlr_log(WLR_ERROR, "Could not add backend: allocation failed");
		return false;
	}
	wl_list_insert(multi->backends.prev, &sub->link);

	sub->backend = backend;
	sub->container = &multi->backend;

	wl_signal_add(&backend->events.destroy, &sub->destroy);
	sub->destroy.notify = handle_subbackend_destroy;

	wl_signal_add(&backend->events.new_input, &sub->new_input);
	sub->new_input.notify = new_input_reemit;

	wl_signal_add(&backend->events.new_output, &sub->new_output);
	sub->new_output.notify = new_output_reemit;

	wlr_signal_emit_safe(&multi->events.backend_add, backend);
	return true;
}

void wlr_multi_backend_remove(struct wlr_backend *_multi,
		struct wlr_backend *backend) {
	struct wlr_multi_backend *multi = multi_backend_from_backend(_multi);

	struct subbackend_state *sub =
		multi_backend_get_subbackend(multi, backend);

	if (sub) {
		wlr_signal_emit_safe(&multi->events.backend_remove, backend);
		subbackend_state_destroy(sub);
	}
}

bool wlr_multi_is_empty(struct wlr_backend *_backend) {
	assert(wlr_backend_is_multi(_backend));
	struct wlr_multi_backend *backend = (struct wlr_multi_backend *)_backend;
	return wl_list_length(&backend->backends) < 1;
}

void wlr_multi_for_each_backend(struct wlr_backend *_backend,
		void (*callback)(struct wlr_backend *backend, void *data), void *data) {
	assert(wlr_backend_is_multi(_backend));
	struct wlr_multi_backend *backend = (struct wlr_multi_backend *)_backend;
	struct subbackend_state *sub;
	wl_list_for_each(sub, &backend->backends, link) {
		callback(sub->backend, data);
	}
}
