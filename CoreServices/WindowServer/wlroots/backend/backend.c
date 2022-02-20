#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-server-core.h>

#include <wlr/backend/headless.h>
#include <wlr/backend/interface.h>
#include <wlr/backend/multi.h>
#include <wlr/backend/session.h>
#include <wlr/backend/wayland.h>
#include <wlr/config.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/util/log.h>
#include "backend/backend.h"
#include "backend/multi.h"
#include "render/allocator/allocator.h"
#include "util/signal.h"

#if WLR_HAS_DRM_BACKEND
#include <wlr/backend/drm.h>
#include "backend/drm/monitor.h"
#endif

#if WLR_HAS_LIBINPUT_BACKEND
#include <wlr/backend/libinput.h>
#endif

#if WLR_HAS_X11_BACKEND
#include <wlr/backend/x11.h>
#endif

#define WAIT_SESSION_TIMEOUT 10000 // ms

void wlr_backend_init(struct wlr_backend *backend,
		const struct wlr_backend_impl *impl) {
	assert(backend);
	backend->impl = impl;
	wl_signal_init(&backend->events.destroy);
	wl_signal_init(&backend->events.new_input);
	wl_signal_init(&backend->events.new_output);
}

void wlr_backend_finish(struct wlr_backend *backend) {
	wlr_signal_emit_safe(&backend->events.destroy, backend);
}

bool wlr_backend_start(struct wlr_backend *backend) {
	if (backend->impl->start) {
		return backend->impl->start(backend);
	}
	return true;
}

void wlr_backend_destroy(struct wlr_backend *backend) {
	if (!backend) {
		return;
	}

	if (backend->impl && backend->impl->destroy) {
		backend->impl->destroy(backend);
	} else {
		free(backend);
	}
}

struct wlr_session *wlr_backend_get_session(struct wlr_backend *backend) {
	if (backend->impl->get_session) {
		return backend->impl->get_session(backend);
	}
	return NULL;
}

static uint64_t get_current_time_ms(void) {
	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static struct wlr_session *session_create_and_wait(struct wl_display *disp) {
	struct wlr_session *session = wlr_session_create(disp);

	if (!session) {
		wlr_log(WLR_ERROR, "Failed to start a session");
		return NULL;
	}

	if (!session->active) {
		wlr_log(WLR_INFO, "Waiting for a session to become active");

		uint64_t started_at = get_current_time_ms();
		uint64_t timeout = WAIT_SESSION_TIMEOUT;
		struct wl_event_loop *event_loop =
			wl_display_get_event_loop(session->display);

		while (!session->active) {
			int ret = wl_event_loop_dispatch(event_loop, (int)timeout);
			if (ret < 0) {
				wlr_log_errno(WLR_ERROR, "Failed to wait for session active: "
					"wl_event_loop_dispatch failed");
				return NULL;
			}

			uint64_t now = get_current_time_ms();
			if (now >= started_at + WAIT_SESSION_TIMEOUT) {
				break;
			}
			timeout = started_at + WAIT_SESSION_TIMEOUT - now;
		}

		if (!session->active) {
			wlr_log(WLR_ERROR, "Timeout waiting session to become active");
			return NULL;
		}
	}

	return session;
}

clockid_t wlr_backend_get_presentation_clock(struct wlr_backend *backend) {
	if (backend->impl->get_presentation_clock) {
		return backend->impl->get_presentation_clock(backend);
	}
	return CLOCK_MONOTONIC;
}

int wlr_backend_get_drm_fd(struct wlr_backend *backend) {
	if (!backend->impl->get_drm_fd) {
		return -1;
	}
	return backend->impl->get_drm_fd(backend);
}

uint32_t backend_get_buffer_caps(struct wlr_backend *backend) {
	if (!backend->impl->get_buffer_caps) {
		return 0;
	}

	return backend->impl->get_buffer_caps(backend);
}

static size_t parse_outputs_env(const char *name) {
	const char *outputs_str = getenv(name);
	if (outputs_str == NULL) {
		return 1;
	}

	char *end;
	int outputs = (int)strtol(outputs_str, &end, 10);
	if (*end || outputs < 0) {
		wlr_log(WLR_ERROR, "%s specified with invalid integer, ignoring", name);
		return 1;
	}

	return outputs;
}

static struct wlr_backend *attempt_wl_backend(struct wl_display *display) {
	struct wlr_backend *backend = wlr_wl_backend_create(display, NULL);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_WL_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_wl_output_create(backend);
	}

	return backend;
}

#if WLR_HAS_X11_BACKEND
static struct wlr_backend *attempt_x11_backend(struct wl_display *display,
		const char *x11_display) {
	struct wlr_backend *backend = wlr_x11_backend_create(display, x11_display);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_X11_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_x11_output_create(backend);
	}

	return backend;
}
#endif

static struct wlr_backend *attempt_headless_backend(
		struct wl_display *display) {
	struct wlr_backend *backend = wlr_headless_backend_create(display);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_HEADLESS_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_headless_add_output(backend, 1280, 720);
	}

	return backend;
}

#if WLR_HAS_DRM_BACKEND
static struct wlr_backend *attempt_drm_backend(struct wl_display *display,
		struct wlr_backend *backend, struct wlr_session *session) {
	struct wlr_device *gpus[8];
	ssize_t num_gpus = wlr_session_find_gpus(session, 8, gpus);
	if (num_gpus < 0) {
		wlr_log(WLR_ERROR, "Failed to find GPUs");
		return NULL;
	}

	if (num_gpus == 0) {
		wlr_log(WLR_ERROR, "Found 0 GPUs, cannot create backend");
		return NULL;
	} else {
		wlr_log(WLR_INFO, "Found %zu GPUs", num_gpus);
	}

	struct wlr_backend *primary_drm = NULL;
	for (size_t i = 0; i < (size_t)num_gpus; ++i) {
		struct wlr_backend *drm = wlr_drm_backend_create(display, session,
			gpus[i], primary_drm);
		if (!drm) {
			wlr_log(WLR_ERROR, "Failed to create DRM backend");
			continue;
		}

		if (!primary_drm) {
			primary_drm = drm;
		}

		wlr_multi_backend_add(backend, drm);
	}
	if (!primary_drm) {
		wlr_log(WLR_ERROR, "Could not successfully create backend on any GPU");
		return NULL;
	}

	return primary_drm;
}
#endif

static bool attempt_backend_by_name(struct wl_display *display,
		struct wlr_multi_backend *multi, char *name) {
	struct wlr_backend *backend = NULL;
	if (strcmp(name, "wayland") == 0) {
		backend = attempt_wl_backend(display);
#if WLR_HAS_X11_BACKEND
	} else if (strcmp(name, "x11") == 0) {
		backend = attempt_x11_backend(display, NULL);
#endif
	} else if (strcmp(name, "headless") == 0) {
		backend = attempt_headless_backend(display);
	} else if (strcmp(name, "drm") == 0 || strcmp(name, "libinput") == 0) {
		// DRM and libinput need a session
		if (multi->session == NULL) {
			multi->session = session_create_and_wait(display);
			if (multi->session == NULL) {
				wlr_log(WLR_ERROR, "failed to start a session");
				return false;
			}
		}

		if (strcmp(name, "libinput") == 0) {
#if WLR_HAS_LIBINPUT_BACKEND
			backend = wlr_libinput_backend_create(display, multi->session);
#endif
		} else {
#if WLR_HAS_DRM_BACKEND
			// attempt_drm_backend adds the multi drm backends itself
			return attempt_drm_backend(display, &multi->backend,
					multi->session) != NULL;
#endif
		}
	} else {
		wlr_log(WLR_ERROR, "unrecognized backend '%s'", name);
		return false;
	}

	return wlr_multi_backend_add(&multi->backend, backend);
}

struct wlr_backend *wlr_backend_autocreate(struct wl_display *display) {
	struct wlr_backend *backend = wlr_multi_backend_create(display);
	struct wlr_multi_backend *multi = (struct wlr_multi_backend *)backend;
	if (!backend) {
		wlr_log(WLR_ERROR, "could not allocate multibackend");
		return NULL;
	}

	char *names = getenv("WLR_BACKENDS");
	if (names) {
		wlr_log(WLR_INFO, "Loading user-specified backends due to WLR_BACKENDS: %s",
			names);

		names = strdup(names);
		if (names == NULL) {
			wlr_log(WLR_ERROR, "allocation failed");
			wlr_backend_destroy(backend);
			return NULL;
		}

		char *saveptr;
		char *name = strtok_r(names, ",", &saveptr);
		while (name != NULL) {
			if (!attempt_backend_by_name(display, multi, name)) {
				wlr_log(WLR_ERROR, "failed to add backend '%s'", name);
				wlr_session_destroy(multi->session);
				wlr_backend_destroy(backend);
				free(names);
				return NULL;
			}

			name = strtok_r(NULL, ",", &saveptr);
		}

		free(names);
		return backend;
	}

	if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET")) {
		struct wlr_backend *wl_backend = attempt_wl_backend(display);
		if (!wl_backend) {
			goto error;
		}

		wlr_multi_backend_add(backend, wl_backend);
		return backend;
	}

#if WLR_HAS_X11_BACKEND
	const char *x11_display = getenv("DISPLAY");
	if (x11_display) {
		struct wlr_backend *x11_backend =
			attempt_x11_backend(display, x11_display);
		if (!x11_backend) {
			goto error;
		}

		wlr_multi_backend_add(backend, x11_backend);
		return backend;
	}
#endif

	// Attempt DRM+libinput
	multi->session = session_create_and_wait(display);
	if (!multi->session) {
		wlr_log(WLR_ERROR, "Failed to start a DRM session");
		wlr_backend_destroy(backend);
		return NULL;
	}

#if WLR_HAS_LIBINPUT_BACKEND
	struct wlr_backend *libinput = wlr_libinput_backend_create(display,
		multi->session);
	if (!libinput) {
		wlr_log(WLR_ERROR, "Failed to start libinput backend");
		wlr_session_destroy(multi->session);
		wlr_backend_destroy(backend);
		return NULL;
	}
	wlr_multi_backend_add(backend, libinput);
#else
	const char *no_devs = getenv("WLR_LIBINPUT_NO_DEVICES");
	if (no_devs && strcmp(no_devs, "1") == 0) {
		wlr_log(WLR_INFO, "WLR_LIBINPUT_NO_DEVICES is set, "
			"starting without libinput backend");
	} else {
		wlr_log(WLR_ERROR, "libinput support is not compiled in, "
			"refusing to start");
		wlr_log(WLR_ERROR, "Set WLR_LIBINPUT_NO_DEVICES=1 to suppress this check");
		wlr_session_destroy(multi->session);
		wlr_backend_destroy(backend);
		return NULL;
	}
#endif

#if WLR_HAS_DRM_BACKEND
	struct wlr_backend *primary_drm =
		attempt_drm_backend(display, backend, multi->session);
	if (!primary_drm) {
		wlr_log(WLR_ERROR, "Failed to open any DRM device");
		wlr_session_destroy(multi->session);
		wlr_backend_destroy(backend);
		return NULL;
	}

	drm_backend_monitor_create(backend, primary_drm, multi->session);

	return backend;
#endif

error:
	wlr_backend_destroy(backend);
	return NULL;
}
