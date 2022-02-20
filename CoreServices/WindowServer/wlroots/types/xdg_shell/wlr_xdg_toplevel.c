#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/util/log.h>
#include <wlr/util/edges.h>
#include "types/wlr_xdg_shell.h"
#include "util/signal.h"

void handle_xdg_toplevel_ack_configure(
		struct wlr_xdg_toplevel *toplevel,
		struct wlr_xdg_toplevel_configure *configure) {
	toplevel->pending.maximized = configure->maximized;
	toplevel->pending.fullscreen = configure->fullscreen;
	toplevel->pending.resizing = configure->resizing;
	toplevel->pending.activated = configure->activated;
	toplevel->pending.tiled = configure->tiled;

	toplevel->pending.width = configure->width;
	toplevel->pending.height = configure->height;
}

struct wlr_xdg_toplevel_configure *send_xdg_toplevel_configure(
		struct wlr_xdg_toplevel *toplevel) {
	struct wlr_xdg_toplevel_configure *configure =
		calloc(1, sizeof(*configure));
	if (configure == NULL) {
		wlr_log(WLR_ERROR, "Allocation failed");
		wl_resource_post_no_memory(toplevel->resource);
		return NULL;
	}
	*configure = toplevel->scheduled;

	struct wl_array states;
	wl_array_init(&states);
	if (configure->maximized) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for maximized xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_MAXIMIZED;
	}
	if (configure->fullscreen) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for fullscreen xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_FULLSCREEN;
	}
	if (configure->resizing) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for resizing xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_RESIZING;
	}
	if (configure->activated) {
		uint32_t *s = wl_array_add(&states, sizeof(uint32_t));
		if (!s) {
			wlr_log(WLR_ERROR, "Could not allocate state for activated xdg_toplevel");
			goto error_out;
		}
		*s = XDG_TOPLEVEL_STATE_ACTIVATED;
	}
	if (configure->tiled) {
		if (wl_resource_get_version(toplevel->resource) >=
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
				if ((configure->tiled & tiled[i].edge) == 0) {
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
		} else if (!configure->maximized) {
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

	uint32_t width = configure->width;
	uint32_t height = configure->height;
	xdg_toplevel_send_configure(toplevel->resource, width, height, &states);

	wl_array_release(&states);
	return configure;

error_out:
	wl_array_release(&states);
	free(configure);
	wl_resource_post_no_memory(toplevel->resource);
	return NULL;
}

void handle_xdg_toplevel_committed(struct wlr_xdg_toplevel *toplevel) {
	if (!toplevel->added) {
		// on the first commit, send a configure request to tell the client it
		// is added
		wlr_xdg_surface_schedule_configure(toplevel->base);
		toplevel->added = true;
		return;
	}

	toplevel->current = toplevel->pending;
}

static const struct xdg_toplevel_interface xdg_toplevel_implementation;

struct wlr_xdg_toplevel *wlr_xdg_toplevel_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &xdg_toplevel_interface,
		&xdg_toplevel_implementation));
	return wl_resource_get_user_data(resource);
}

static void handle_parent_unmap(struct wl_listener *listener, void *data) {
	struct wlr_xdg_toplevel *toplevel =
		wl_container_of(listener, toplevel, parent_unmap);
	wlr_xdg_toplevel_set_parent(toplevel, toplevel->parent->parent);
}

void wlr_xdg_toplevel_set_parent(struct wlr_xdg_toplevel *toplevel,
		struct wlr_xdg_toplevel *parent) {
	if (toplevel->parent) {
		wl_list_remove(&toplevel->parent_unmap.link);
	}
	
	toplevel->parent = parent;
	if (parent) {
		toplevel->parent_unmap.notify = handle_parent_unmap;
		wl_signal_add(&toplevel->parent->base->events.unmap,
				&toplevel->parent_unmap);
	}

	wlr_signal_emit_safe(&toplevel->events.set_parent, NULL);
}

static void xdg_toplevel_handle_set_parent(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *parent_resource) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	struct wlr_xdg_toplevel *parent = NULL;

	if (parent_resource != NULL) {
		parent = wlr_xdg_toplevel_from_resource(parent_resource);
	}

	wlr_xdg_toplevel_set_parent(toplevel, parent);
}

static void xdg_toplevel_handle_set_title(struct wl_client *client,
		struct wl_resource *resource, const char *title) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	char *tmp;

	tmp = strdup(title);
	if (tmp == NULL) {
		return;
	}

	free(toplevel->title);
	toplevel->title = tmp;
	wlr_signal_emit_safe(&toplevel->events.set_title, NULL);
}

static void xdg_toplevel_handle_set_app_id(struct wl_client *client,
		struct wl_resource *resource, const char *app_id) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	char *tmp;

	tmp = strdup(app_id);
	if (tmp == NULL) {
		return;
	}

	free(toplevel->app_id);
	toplevel->app_id = tmp;
	wlr_signal_emit_safe(&toplevel->events.set_app_id, NULL);
}

static void xdg_toplevel_handle_show_window_menu(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial, int32_t x, int32_t y) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	struct wlr_seat_client *seat =
		wlr_seat_client_from_resource(seat_resource);

	if (!toplevel->base->configured) {
		wl_resource_post_error(toplevel->base->resource,
			XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
			"surface has not been configured yet");
		return;
	}

	if (!wlr_seat_validate_grab_serial(seat->seat, serial)) {
		wlr_log(WLR_DEBUG, "invalid serial for grab");
		return;
	}

	struct wlr_xdg_toplevel_show_window_menu_event event = {
		.toplevel = toplevel,
		.seat = seat,
		.serial = serial,
		.x = x,
		.y = y,
	};

	wlr_signal_emit_safe(&toplevel->events.request_show_window_menu, &event);
}

static void xdg_toplevel_handle_move(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	struct wlr_seat_client *seat =
		wlr_seat_client_from_resource(seat_resource);

	if (!toplevel->base->configured) {
		wl_resource_post_error(toplevel->base->resource,
			XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
			"surface has not been configured yet");
		return;
	}

	if (!wlr_seat_validate_grab_serial(seat->seat, serial)) {
		wlr_log(WLR_DEBUG, "invalid serial for grab");
		return;
	}

	struct wlr_xdg_toplevel_move_event event = {
		.toplevel = toplevel,
		.seat = seat,
		.serial = serial,
	};

	wlr_signal_emit_safe(&toplevel->events.request_move, &event);
}

static void xdg_toplevel_handle_resize(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial, uint32_t edges) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	struct wlr_seat_client *seat =
		wlr_seat_client_from_resource(seat_resource);

	if (!toplevel->base->configured) {
		wl_resource_post_error(toplevel->base->resource,
			XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
			"surface has not been configured yet");
		return;
	}

	if (!wlr_seat_validate_grab_serial(seat->seat, serial)) {
		wlr_log(WLR_DEBUG, "invalid serial for grab");
		return;
	}

	struct wlr_xdg_toplevel_resize_event event = {
		.toplevel = toplevel,
		.seat = seat,
		.serial = serial,
		.edges = edges,
	};

	wlr_signal_emit_safe(&toplevel->events.request_resize, &event);
}

static void xdg_toplevel_handle_set_max_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	toplevel->pending.max_width = width;
	toplevel->pending.max_height = height;
}

static void xdg_toplevel_handle_set_min_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	toplevel->pending.min_width = width;
	toplevel->pending.min_height = height;
}

static void xdg_toplevel_handle_set_maximized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	toplevel->requested.maximized = true;
	wlr_signal_emit_safe(&toplevel->events.request_maximize, NULL);
	wlr_xdg_surface_schedule_configure(toplevel->base);
}

static void xdg_toplevel_handle_unset_maximized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	toplevel->requested.maximized = false;
	wlr_signal_emit_safe(&toplevel->events.request_maximize, NULL);
	wlr_xdg_surface_schedule_configure(toplevel->base);
}

static void handle_fullscreen_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_toplevel_requested *req =
		wl_container_of(listener, req, fullscreen_output_destroy);
	req->fullscreen_output = NULL;
	wl_list_remove(&req->fullscreen_output_destroy.link);
}

static void store_fullscreen_requested(struct wlr_xdg_toplevel *toplevel,
		bool fullscreen, struct wlr_output *output) {
	struct wlr_xdg_toplevel_requested *req = &toplevel->requested;
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
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);

	struct wlr_output *output = NULL;
	if (output_resource != NULL) {
		output = wlr_output_from_resource(output_resource);
	}

	store_fullscreen_requested(toplevel, true, output);

	wlr_signal_emit_safe(&toplevel->events.request_fullscreen, NULL);
	wlr_xdg_surface_schedule_configure(toplevel->base);
}

static void xdg_toplevel_handle_unset_fullscreen(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);

	store_fullscreen_requested(toplevel, false, NULL);

	wlr_signal_emit_safe(&toplevel->events.request_fullscreen, NULL);
	wlr_xdg_surface_schedule_configure(toplevel->base);
}

static void xdg_toplevel_handle_set_minimized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	toplevel->requested.minimized = true;
	wlr_signal_emit_safe(&toplevel->events.request_minimize, NULL);
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
	struct wlr_xdg_toplevel *toplevel =
		wlr_xdg_toplevel_from_resource(resource);
	if (toplevel == NULL) {
		return;
	}
	reset_xdg_surface(toplevel->base);
}

const struct wlr_surface_role xdg_toplevel_surface_role = {
	.name = "xdg_toplevel",
	.commit = xdg_surface_role_commit,
	.precommit = xdg_surface_role_precommit,
};

void create_xdg_toplevel(struct wlr_xdg_surface *surface,
		uint32_t id) {
	if (!wlr_surface_set_role(surface->surface, &xdg_toplevel_surface_role,
			surface, surface->resource, XDG_WM_BASE_ERROR_ROLE)) {
		return;
	}

	if (surface->role != WLR_XDG_SURFACE_ROLE_NONE) {
		wl_resource_post_error(surface->resource,
			XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
			"xdg-surface has already been constructed");
		return;
	}

	assert(surface->toplevel == NULL);
	surface->toplevel = calloc(1, sizeof(struct wlr_xdg_toplevel));
	if (surface->toplevel == NULL) {
		wl_resource_post_no_memory(surface->resource);
		return;
	}
	surface->toplevel->base = surface;

	wl_signal_init(&surface->toplevel->events.request_maximize);
	wl_signal_init(&surface->toplevel->events.request_fullscreen);
	wl_signal_init(&surface->toplevel->events.request_minimize);
	wl_signal_init(&surface->toplevel->events.request_move);
	wl_signal_init(&surface->toplevel->events.request_resize);
	wl_signal_init(&surface->toplevel->events.request_show_window_menu);
	wl_signal_init(&surface->toplevel->events.set_parent);
	wl_signal_init(&surface->toplevel->events.set_title);
	wl_signal_init(&surface->toplevel->events.set_app_id);

	surface->toplevel->resource = wl_resource_create(
		surface->client->client, &xdg_toplevel_interface,
		wl_resource_get_version(surface->resource), id);
	if (surface->toplevel->resource == NULL) {
		free(surface->toplevel);
		surface->toplevel = NULL;
		wl_resource_post_no_memory(surface->resource);
		return;
	}
	wl_resource_set_implementation(surface->toplevel->resource,
		&xdg_toplevel_implementation, surface->toplevel,
		xdg_toplevel_handle_resource_destroy);

	surface->role = WLR_XDG_SURFACE_ROLE_TOPLEVEL;
}

void unmap_xdg_toplevel(struct wlr_xdg_toplevel *toplevel) {
	if (toplevel->parent) {
		wl_list_remove(&toplevel->parent_unmap.link);
		toplevel->parent = NULL;
	}
	free(toplevel->title);
	toplevel->title = NULL;
	free(toplevel->app_id);
	toplevel->app_id = NULL;

	if (toplevel->requested.fullscreen_output) {
		wl_list_remove(&toplevel->requested.fullscreen_output_destroy.link);
		toplevel->requested.fullscreen_output = NULL;
	}
	toplevel->requested.fullscreen = false;
	toplevel->requested.maximized = false;
	toplevel->requested.minimized = false;
}

void wlr_xdg_toplevel_send_close(struct wlr_xdg_toplevel *toplevel) {
	xdg_toplevel_send_close(toplevel->resource);
}

uint32_t wlr_xdg_toplevel_set_size(struct wlr_xdg_toplevel *toplevel,
		uint32_t width, uint32_t height) {
	toplevel->scheduled.width = width;
	toplevel->scheduled.height = height;
	return wlr_xdg_surface_schedule_configure(toplevel->base);
}

uint32_t wlr_xdg_toplevel_set_activated(struct wlr_xdg_toplevel *toplevel,
		bool activated) {
	toplevel->scheduled.activated = activated;
	return wlr_xdg_surface_schedule_configure(toplevel->base);
}

uint32_t wlr_xdg_toplevel_set_maximized(struct wlr_xdg_toplevel *toplevel,
		bool maximized) {
	toplevel->scheduled.maximized = maximized;
	return wlr_xdg_surface_schedule_configure(toplevel->base);
}

uint32_t wlr_xdg_toplevel_set_fullscreen(struct wlr_xdg_toplevel *toplevel,
		bool fullscreen) {
	toplevel->scheduled.fullscreen = fullscreen;
	return wlr_xdg_surface_schedule_configure(toplevel->base);
}

uint32_t wlr_xdg_toplevel_set_resizing(struct wlr_xdg_toplevel *toplevel,
		bool resizing) {
	toplevel->scheduled.resizing = resizing;
	return wlr_xdg_surface_schedule_configure(toplevel->base);
}

uint32_t wlr_xdg_toplevel_set_tiled(struct wlr_xdg_toplevel *toplevel,
		uint32_t tiled) {
	toplevel->scheduled.tiled = tiled;
	return wlr_xdg_surface_schedule_configure(toplevel->base);
}
