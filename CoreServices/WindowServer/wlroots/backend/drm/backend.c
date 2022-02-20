#include <assert.h>
#include <errno.h>
#include <drm_fourcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend/interface.h>
#include <wlr/backend/session.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/util/log.h>
#include <xf86drm.h>
#include "backend/drm/drm.h"
#include "util/signal.h"

struct wlr_drm_backend *get_drm_backend_from_backend(
		struct wlr_backend *wlr_backend) {
	assert(wlr_backend_is_drm(wlr_backend));
	return (struct wlr_drm_backend *)wlr_backend;
}

static bool backend_start(struct wlr_backend *backend) {
	struct wlr_drm_backend *drm = get_drm_backend_from_backend(backend);
	scan_drm_connectors(drm, NULL);
	return true;
}

static void backend_destroy(struct wlr_backend *backend) {
	if (!backend) {
		return;
	}

	struct wlr_drm_backend *drm = get_drm_backend_from_backend(backend);

	struct wlr_drm_connector *conn, *next;
	wl_list_for_each_safe(conn, next, &drm->outputs, link) {
		destroy_drm_connector(conn);
	}

	wlr_backend_finish(backend);

	struct wlr_drm_fb *fb, *fb_tmp;
	wl_list_for_each_safe(fb, fb_tmp, &drm->fbs, link) {
		drm_fb_destroy(fb);
	}

	wl_list_remove(&drm->display_destroy.link);
	wl_list_remove(&drm->session_destroy.link);
	wl_list_remove(&drm->session_active.link);
	wl_list_remove(&drm->parent_destroy.link);
	wl_list_remove(&drm->dev_change.link);
	wl_list_remove(&drm->dev_remove.link);

	if (drm->parent) {
		finish_drm_renderer(&drm->mgpu_renderer);
	}

	finish_drm_resources(drm);

	free(drm->name);
	wlr_session_close_file(drm->session, drm->dev);
	wl_event_source_remove(drm->drm_event);
	free(drm);
}

static clockid_t backend_get_presentation_clock(struct wlr_backend *backend) {
	struct wlr_drm_backend *drm = get_drm_backend_from_backend(backend);
	return drm->clock;
}

static int backend_get_drm_fd(struct wlr_backend *backend) {
	struct wlr_drm_backend *drm = get_drm_backend_from_backend(backend);

	if (drm->parent) {
		return drm->parent->fd;
	} else {
		return drm->fd;
	}
}

static uint32_t drm_backend_get_buffer_caps(struct wlr_backend *backend) {
	return WLR_BUFFER_CAP_DMABUF;
}

static const struct wlr_backend_impl backend_impl = {
	.start = backend_start,
	.destroy = backend_destroy,
	.get_presentation_clock = backend_get_presentation_clock,
	.get_drm_fd = backend_get_drm_fd,
	.get_buffer_caps = drm_backend_get_buffer_caps,
};

bool wlr_backend_is_drm(struct wlr_backend *b) {
	return b->impl == &backend_impl;
}

static void handle_session_active(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend *drm =
		wl_container_of(listener, drm, session_active);
	struct wlr_session *session = drm->session;

	if (session->active) {
		wlr_log(WLR_INFO, "DRM fd resumed");
		scan_drm_connectors(drm, NULL);

		struct wlr_drm_connector *conn;
		wl_list_for_each(conn, &drm->outputs, link) {
			struct wlr_output_mode *mode = NULL;
			uint32_t committed = WLR_OUTPUT_STATE_ENABLED;
			if (conn->output.enabled && conn->output.current_mode != NULL) {
				committed |= WLR_OUTPUT_STATE_MODE;
				mode = conn->output.current_mode;
			}
			struct wlr_output_state state = {
				.committed = committed,
				.enabled = mode != NULL,
				.mode_type = WLR_OUTPUT_STATE_MODE_FIXED,
				.mode = mode,
			};
			drm_connector_commit_state(conn, &state);
		}
	} else {
		wlr_log(WLR_INFO, "DRM fd paused");
	}
}

static void handle_dev_change(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend *drm = wl_container_of(listener, drm, dev_change);
	struct wlr_device_change_event *change = data;

	if (!drm->session->active) {
		return;
	}

	switch (change->type) {
	case WLR_DEVICE_HOTPLUG:
		wlr_log(WLR_DEBUG, "Received hotplug event for %s", drm->name);
		scan_drm_connectors(drm, &change->hotplug);
		break;
	case WLR_DEVICE_LEASE:
		wlr_log(WLR_DEBUG, "Received lease event for %s", drm->name);
		scan_drm_leases(drm);
		break;
	default:
		wlr_log(WLR_DEBUG, "Received unknown change event for %s", drm->name);
	}
}

static void handle_dev_remove(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend *drm = wl_container_of(listener, drm, dev_remove);

	wlr_log(WLR_INFO, "Destroying DRM backend for %s", drm->name);
	backend_destroy(&drm->backend);
}

static void handle_session_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend *drm =
		wl_container_of(listener, drm, session_destroy);
	backend_destroy(&drm->backend);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend *drm =
		wl_container_of(listener, drm, display_destroy);
	backend_destroy(&drm->backend);
}

static void handle_parent_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend *drm =
		wl_container_of(listener, drm, parent_destroy);
	backend_destroy(&drm->backend);
}

struct wlr_backend *wlr_drm_backend_create(struct wl_display *display,
		struct wlr_session *session, struct wlr_device *dev,
		struct wlr_backend *parent) {
	assert(display && session && dev);
	assert(!parent || wlr_backend_is_drm(parent));

	char *name = drmGetDeviceNameFromFd2(dev->fd);
	drmVersion *version = drmGetVersion(dev->fd);
	wlr_log(WLR_INFO, "Initializing DRM backend for %s (%s)", name, version->name);
	drmFreeVersion(version);

	struct wlr_drm_backend *drm = calloc(1, sizeof(struct wlr_drm_backend));
	if (!drm) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	wlr_backend_init(&drm->backend, &backend_impl);

	drm->session = session;
	wl_list_init(&drm->fbs);
	wl_list_init(&drm->outputs);

	drm->dev = dev;
	drm->fd = dev->fd;
	drm->name = name;

	if (parent != NULL) {
		drm->parent = get_drm_backend_from_backend(parent);

		drm->parent_destroy.notify = handle_parent_destroy;
		wl_signal_add(&parent->events.destroy, &drm->parent_destroy);
	} else {
		wl_list_init(&drm->parent_destroy.link);
	}

	drm->dev_change.notify = handle_dev_change;
	wl_signal_add(&dev->events.change, &drm->dev_change);

	drm->dev_remove.notify = handle_dev_remove;
	wl_signal_add(&dev->events.remove, &drm->dev_remove);

	drm->display = display;

	struct wl_event_loop *event_loop = wl_display_get_event_loop(display);
	drm->drm_event = wl_event_loop_add_fd(event_loop, drm->fd,
		WL_EVENT_READABLE, handle_drm_event, drm);
	if (!drm->drm_event) {
		wlr_log(WLR_ERROR, "Failed to create DRM event source");
		goto error_fd;
	}

	drm->session_active.notify = handle_session_active;
	wl_signal_add(&session->events.active, &drm->session_active);

	if (!check_drm_features(drm)) {
		goto error_event;
	}

	if (!init_drm_resources(drm)) {
		goto error_event;
	}

	if (drm->parent) {
		if (!init_drm_renderer(drm, &drm->mgpu_renderer)) {
			wlr_log(WLR_ERROR, "Failed to initialize renderer");
			goto error_resources;
		}

		// We'll perform a multi-GPU copy for all submitted buffers, we need
		// to be able to texture from them
		struct wlr_renderer *renderer = drm->mgpu_renderer.wlr_rend;
		const struct wlr_drm_format_set *texture_formats =
			wlr_renderer_get_dmabuf_texture_formats(renderer);
		if (texture_formats == NULL) {
			wlr_log(WLR_ERROR, "Failed to query renderer texture formats");
			goto error_mgpu_renderer;
		}

		// Force a linear layout. In case explicit modifiers aren't supported,
		// the meaning of implicit modifiers changes from one GPU to the other.
		// In case explicit modifiers are supported, we still have no guarantee
		// that the buffer producer will support these, so they might fallback
		// to implicit modifiers.
		for (size_t i = 0; i < texture_formats->len; i++) {
			const struct wlr_drm_format *fmt = texture_formats->formats[i];
			wlr_drm_format_set_add(&drm->mgpu_formats, fmt->format,
				DRM_FORMAT_MOD_LINEAR);
		}
	}

	drm->session_destroy.notify = handle_session_destroy;
	wl_signal_add(&session->events.destroy, &drm->session_destroy);

	drm->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &drm->display_destroy);

	return &drm->backend;

error_mgpu_renderer:
	finish_drm_renderer(&drm->mgpu_renderer);
error_resources:
	finish_drm_resources(drm);
error_event:
	wl_list_remove(&drm->session_active.link);
	wl_event_source_remove(drm->drm_event);
error_fd:
	wl_list_remove(&drm->dev_remove.link);
	wl_list_remove(&drm->dev_change.link);
	wl_list_remove(&drm->parent_destroy.link);
	wlr_session_close_file(drm->session, dev);
	free(drm);
	return NULL;
}
