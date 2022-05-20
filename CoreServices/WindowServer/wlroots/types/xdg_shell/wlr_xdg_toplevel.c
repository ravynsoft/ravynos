#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/util/log.h>
#include <wlr/util/edges.h>
#include "types/wlr_xdg_shell.h"
#include "util/signal.h"

void handle_xdg_toplevel_ack_configure(
		struct wlr_xdg_surface *surface,
		struct wlr_xdg_surface_configure *configure) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	struct wlr_xdg_toplevel_configure *acked = configure->toplevel_configure;
	assert(acked != NULL);

	surface->toplevel->pending.maximized = acked->maximized;
	surface->toplevel->pending.fullscreen = acked->fullscreen;
	surface->toplevel->pending.resizing = acked->resizing;
	surface->toplevel->pending.activated = acked->activated;
	surface->toplevel->pending.tiled = acked->tiled;

	surface->toplevel->pending.width = acked->width;
	surface->toplevel->pending.height = acked->height;
}

void send_xdg_toplevel_configure(struct wlr_xdg_surface *surface,
		struct wlr_xdg_surface_configure *configure) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	configure->toplevel_configure = malloc(sizeof(*configure->toplevel_configure));
	if (configure->toplevel_configure == NULL) {
		wlr_log(WLR_ERROR, "Allocation failed");
		wl_resource_post_no_memory(surface->toplevel->resource);
		return;
	}
	*configure->toplevel_configure = surface->toplevel->scheduled;

	struct wl_array states;
	wl_array_init(&states);
	if (surface->toplevel->scheduled.maximized) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for maximized xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_MAXIMIZED;
	}
	if (surface->toplevel->scheduled.fullscreen) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for fullscreen xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_FULLSCREEN;
	}
	if (surface->toplevel->scheduled.resizing) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for resizing xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_RESIZING;
	}
	if (surface->toplevel->scheduled.activated) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for activated xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_ACTIVATED;
	}
	if (surface->toplevel->scheduled.tiled) {
		if (wl_resource_get_version(surface->resource) >=
				XDG_TOPLEVEL_STATE_TILED_LEFT_SINCE_VERSION) {
			const struct {
				enum wlr_edges edge;
				enum xdg_toplevel_state state;
			} tiled[] = {
				{ WLR_EDGE_LEFT, XDG_TOPLEVEL_STATE_TILED_LEFT },
				{ WLR_EDGE_RIGHT, XDG_TOPLEVEL_STATE_TILED_RIGHT },
				{ WLR_EDGE_TOP, XDG_TOPLEVEL_STATE_TILED_TOP },
				{ WLR_EDGE_BOTTOM, XDG_TOPLEVEL_STATE_TILED_BOTTOM },
			};

			for (size_t i = 0; i < sizeof(tiled)/sizeof(tiled[0]); ++i) {
				if ((surface->toplevel->scheduled.tiled &
						tiled[i].edge) == 0) {
					continue;
				}

				uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
				if (!s) {
					wlr_log(WLR_ERROR,
						"Could not allocate state for tiled xdg_toplevel");
					goto error_out;
				}
				*s = tiled[i].state;
			}
		} else if (!surface->toplevel->scheduled.maximized) {
			// This version doesn't support tiling, best we can do is make the
			// toplevel maximized
			uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
			if (!s) {
				wlr_log(WLR_ERROR,
					"Could not allocate state for maximized xdg_toplevel");
				goto error_out;
			}
			*s = XDG_TOPLEVEL_STATE_MAXIMIZED;
		}
	}

	uint32_t width = surface->toplevel->scheduled.width;
	uint32_t height = surface->toplevel->scheduled.height;
	xdg_toplevel_send_configure(surface->toplevel->resource, width, height,
		&states);

	wl_array_release(&states);
	return;

error_out:
	wl_array_release(&states);
	wl_resource_post_no_memory(surface->toplevel->resource);
}

void handle_xdg_surface_toplevel_committed(struct wlr_xdg_surface *surface) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	if (!surface->toplevel->added) {
		// on the first commit, send a configure request to tell the client it
		// is added
		wlr_xdg_surface_schedule_configure(surface);
		surface->toplevel->added = true;
		return;
	}

	surface->toplevel->current = surface->toplevel->pending;
}

static const struct xdg_toplevel_interface xdg_toplevel_implementation;

struct wlr_xdg_surface *wlr_xdg_surface_from_toplevel_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &xdg_toplevel_interface,
		&xdg_toplevel_implementation));
	return wl_resource_get_user_data(resource);
}

static void handle_parent_unmap(struct wl_listener *listener, void *data) {
	struct wlr_xdg_toplevel *toplevel =
		wl_container_of(listener, toplevel, parent_unmap);
	wlr_xdg_toplevel_set_parent(toplevel->base,
			toplevel->parent->toplevel->parent);
}

void wlr_xdg_toplevel_set_parent(struct wlr_xdg_surface *surface,
		struct wlr_xdg_surface *parent) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	assert(!parent || parent->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	if (surface->toplevel->parent) {
		wl_list_remove(&surface->toplevel->parent_unmap.link);
	}

	surface->toplevel->parent = parent;
	if (surface->toplevel->parent) {
		surface->toplevel->parent_unmap.notify = handle_parent_unmap;
		wl_signal_add(&surface->toplevel->parent->events.unmap,
				&surface->toplevel->parent_unmap);
	}

	wlr_signal_emit_safe(&surface->toplevel->events.set_parent, surface);
}

static void xdg_toplevel_handle_set_parent(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *parent_resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	struct wlr_xdg_surface *parent = NULL;

	if (parent_resource != NULL) {
		parent = wlr_xdg_surface_from_toplevel_resource(parent_resource);
	}

	wlr_xdg_toplevel_set_parent(surface, parent);
}

static void xdg_toplevel_handle_set_title(struct wl_client *client,
		struct wl_resource *resource, const char *title) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	char *tmp;

	tmp = strdup(title);
	if (tmp == NULL) {
		return;
	}

	free(surface->toplevel->title);
	surface->toplevel->title = tmp;
	wlr_signal_emit_safe(&surface->toplevel->events.set_title, surface);
}

static void xdg_toplevel_handle_set_app_id(struct wl_client *client,
		struct wl_resource *resource, const char *app_id) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	char *tmp;

	tmp = strdup(app_id);
	if (tmp == NULL) {
		return;
	}

	free(surface->toplevel->app_id);
	surface->toplevel->app_id = tmp;
	wlr_signal_emit_safe(&surface->toplevel->events.set_app_id, surface);
}

static void xdg_toplevel_handle_show_window_menu(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial, int32_t x, int32_t y) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	struct wlr_seat_client *seat =
		wlr_seat_client_from_resource(seat_resource);

	if (!surface->configured) {
		wl_resource_post_error(surface->toplevel->resource,
			XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
			"surface has not been configured yet");
		return;
	}

	if (!wlr_seat_validate_grab_serial(seat->seat, serial)) {
		wlr_log(WLR_DEBUG, "invalid serial for grab");
		return;
	}

	struct wlr_xdg_toplevel_show_window_menu_event event = {
		.surface = surface,
		.seat = seat,
		.serial = serial,
		.x = x,
		.y = y,
	};

	wlr_signal_emit_safe(&surface->toplevel->events.request_show_window_menu, &event);
}

static void xdg_toplevel_handle_move(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	struct wlr_seat_client *seat =
		wlr_seat_client_from_resource(seat_resource);

	if (!surface->configured) {
		wl_resource_post_error(surface->toplevel->resource,
			XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
			"surface has not been configured yet");
		return;
	}

	if (!wlr_seat_validate_grab_serial(seat->seat, serial)) {
		wlr_log(WLR_DEBUG, "invalid serial for grab");
		return;
	}

	struct wlr_xdg_toplevel_move_event event = {
		.surface = surface,
		.seat = seat,
		.serial = serial,
	};

	wlr_signal_emit_safe(&surface->toplevel->events.request_move, &event);
}

static void xdg_toplevel_handle_resize(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial, uint32_t edges) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	struct wlr_seat_client *seat =
		wlr_seat_client_from_resource(seat_resource);

	if (!surface->configured) {
		wl_resource_post_error(surface->toplevel->resource,
			XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
			"surface has not been configured yet");
		return;
	}

	if (!wlr_seat_validate_grab_serial(seat->seat, serial)) {
		wlr_log(WLR_DEBUG, "invalid serial for grab");
		return;
	}

	struct wlr_xdg_toplevel_resize_event event = {
		.surface = surface,
		.seat = seat,
		.serial = serial,
		.edges = edges,
	};

	wlr_signal_emit_safe(&surface->toplevel->events.request_resize, &event);
}

static void xdg_toplevel_handle_set_max_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	surface->toplevel->pending.max_width = width;
	surface->toplevel->pending.max_height = height;
}

static void xdg_toplevel_handle_set_min_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	surface->toplevel->pending.min_width = width;
	surface->toplevel->pending.min_height = height;
}

static void xdg_toplevel_handle_set_maximized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	surface->toplevel->requested.maximized = true;
	wlr_signal_emit_safe(&surface->toplevel->events.request_maximize, surface);
	wlr_xdg_surface_schedule_configure(surface);
}

static void xdg_toplevel_handle_unset_maximized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	surface->toplevel->requested.maximized = false;
	wlr_signal_emit_safe(&surface->toplevel->events.request_maximize, surface);
	wlr_xdg_surface_schedule_configure(surface);
}

static void handle_fullscreen_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_toplevel_requested *req =
		wl_container_of(listener, req, fullscreen_output_destroy);
	req->fullscreen_output = NULL;
	wl_list_remove(&req->fullscreen_output_destroy.link);
}

static void store_fullscreen_requested(struct wlr_xdg_surface *surface,
		bool fullscreen, struct wlr_output *output) {
	struct wlr_xdg_toplevel_requested *req = &surface->toplevel->requested;
	req->fullscreen = fullscreen;
	if (req->fullscreen_output) {
		wl_list_remove(&req->fullscreen_output_destroy.link);
	}
	req->fullscreen_output = output;
	if (req->fullscreen_output) {
		req->fullscreen_output_destroy.notify =
			handle_fullscreen_output_destroy;
		wl_signal_add(&req->fullscreen_output->events.destroy,
				&req->fullscreen_output_destroy);
	}
}

static void xdg_toplevel_handle_set_fullscreen(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *output_resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);

	struct wlr_output *output = NULL;
	if (output_resource != NULL) {
		output = wlr_output_from_resource(output_resource);
	}

	store_fullscreen_requested(surface, true, output);

	struct wlr_xdg_toplevel_set_fullscreen_event event = {
		.surface = surface,
		.fullscreen = true,
		.output = output,
	};

	wlr_signal_emit_safe(&surface->toplevel->events.request_fullscreen, &event);
	wlr_xdg_surface_schedule_configure(surface);
}

static void xdg_toplevel_handle_unset_fullscreen(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);

	store_fullscreen_requested(surface, false, NULL);

	struct wlr_xdg_toplevel_set_fullscreen_event event = {
		.surface = surface,
		.fullscreen = false,
		.output = NULL,
	};

	wlr_signal_emit_safe(&surface->toplevel->events.request_fullscreen, &event);
	wlr_xdg_surface_schedule_configure(surface);
}

static void xdg_toplevel_handle_set_minimized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	surface->toplevel->requested.minimized = true;
	wlr_signal_emit_safe(&surface->toplevel->events.request_minimize, surface);
}

static void xdg_toplevel_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct xdg_toplevel_interface xdg_toplevel_implementation = {
	.destroy = xdg_toplevel_handle_destroy,
	.set_parent = xdg_toplevel_handle_set_parent,
	.set_title = xdg_toplevel_handle_set_title,
	.set_app_id = xdg_toplevel_handle_set_app_id,
	.show_window_menu = xdg_toplevel_handle_show_window_menu,
	.move = xdg_toplevel_handle_move,
	.resize = xdg_toplevel_handle_resize,
	.set_max_size = xdg_toplevel_handle_set_max_size,
	.set_min_size = xdg_toplevel_handle_set_min_size,
	.set_maximized = xdg_toplevel_handle_set_maximized,
	.unset_maximized = xdg_toplevel_handle_unset_maximized,
	.set_fullscreen = xdg_toplevel_handle_set_fullscreen,
	.unset_fullscreen = xdg_toplevel_handle_unset_fullscreen,
	.set_minimized = xdg_toplevel_handle_set_minimized,
};

static void xdg_toplevel_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_toplevel_resource(resource);
	destroy_xdg_toplevel(surface);
}

const struct wlr_surface_role xdg_toplevel_surface_role = {
	.name = "xdg_toplevel",
	.commit = handle_xdg_surface_commit,
	.precommit = handle_xdg_surface_precommit,
};

void create_xdg_toplevel(struct wlr_xdg_surface *xdg_surface,
		uint32_t id) {
	if (!wlr_surface_set_role(xdg_surface->surface, &xdg_toplevel_surface_role,
			xdg_surface, xdg_surface->resource, XDG_WM_BASE_ERROR_ROLE)) {
		return;
	}

	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_NONE) {
		wl_resource_post_error(xdg_surface->resource,
			XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
			"xdg-surface has already been constructed");
		return;
	}

	assert(xdg_surface->toplevel == NULL);
	xdg_surface->toplevel = calloc(1, sizeof(struct wlr_xdg_toplevel));
	if (xdg_surface->toplevel == NULL) {
		wl_resource_post_no_memory(xdg_surface->resource);
		return;
	}
	xdg_surface->toplevel->base = xdg_surface;

	wl_signal_init(&xdg_surface->toplevel->events.request_maximize);
	wl_signal_init(&xdg_surface->toplevel->events.request_fullscreen);
	wl_signal_init(&xdg_surface->toplevel->events.request_minimize);
	wl_signal_init(&xdg_surface->toplevel->events.request_move);
	wl_signal_init(&xdg_surface->toplevel->events.request_resize);
	wl_signal_init(&xdg_surface->toplevel->events.request_show_window_menu);
	wl_signal_init(&xdg_surface->toplevel->events.set_parent);
	wl_signal_init(&xdg_surface->toplevel->events.set_title);
	wl_signal_init(&xdg_surface->toplevel->events.set_app_id);

	xdg_surface->toplevel->resource = wl_resource_create(
		xdg_surface->client->client, &xdg_toplevel_interface,
		wl_resource_get_version(xdg_surface->resource), id);
	if (xdg_surface->toplevel->resource == NULL) {
		free(xdg_surface->toplevel);
		wl_resource_post_no_memory(xdg_surface->resource);
		return;
	}
	wl_resource_set_implementation(xdg_surface->toplevel->resource,
		&xdg_toplevel_implementation, xdg_surface,
		xdg_toplevel_handle_resource_destroy);

	xdg_surface->role = WLR_XDG_SURFACE_ROLE_TOPLEVEL;
}

void destroy_xdg_toplevel(struct wlr_xdg_surface *xdg_surface) {
	if (xdg_surface == NULL) {
		return;
	}
	assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	reset_xdg_surface(xdg_surface);
}

uint32_t wlr_xdg_toplevel_set_size(struct wlr_xdg_surface *surface,
		uint32_t width, uint32_t height) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	surface->toplevel->scheduled.width = width;
	surface->toplevel->scheduled.height = height;

	return wlr_xdg_surface_schedule_configure(surface);
}

uint32_t wlr_xdg_toplevel_set_activated(struct wlr_xdg_surface *surface,
		bool activated) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	surface->toplevel->scheduled.activated = activated;

	return wlr_xdg_surface_schedule_configure(surface);
}

uint32_t wlr_xdg_toplevel_set_maximized(struct wlr_xdg_surface *surface,
		bool maximized) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	surface->toplevel->scheduled.maximized = maximized;

	return wlr_xdg_surface_schedule_configure(surface);
}

uint32_t wlr_xdg_toplevel_set_fullscreen(struct wlr_xdg_surface *surface,
		bool fullscreen) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	surface->toplevel->scheduled.fullscreen = fullscreen;

	return wlr_xdg_surface_schedule_configure(surface);
}

uint32_t wlr_xdg_toplevel_set_resizing(struct wlr_xdg_surface *surface,
		bool resizing) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	surface->toplevel->scheduled.resizing = resizing;

	return wlr_xdg_surface_schedule_configure(surface);
}

uint32_t wlr_xdg_toplevel_set_tiled(struct wlr_xdg_surface *surface,
		uint32_t tiled) {
	assert(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
	surface->toplevel->scheduled.tiled = tiled;

	return wlr_xdg_surface_schedule_configure(surface);
}
