#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"

#define LAYER_SHELL_VERSION 4

static void resource_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwlr_layer_shell_v1_interface layer_shell_implementation;
static const struct zwlr_layer_surface_v1_interface layer_surface_implementation;

static struct wlr_layer_shell_v1 *layer_shell_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwlr_layer_shell_v1_interface,
		&layer_shell_implementation));
	return wl_resource_get_user_data(resource);
}

static struct wlr_layer_surface_v1 *layer_surface_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwlr_layer_surface_v1_interface,
		&layer_surface_implementation));
	return wl_resource_get_user_data(resource);
}

static const struct wlr_surface_role layer_surface_role;

bool wlr_surface_is_layer_surface(struct wlr_surface *surface) {
	return surface->role == &layer_surface_role;
}

struct wlr_layer_surface_v1 *wlr_layer_surface_v1_from_wlr_surface(
		struct wlr_surface *surface) {
	assert(wlr_surface_is_layer_surface(surface));
	return (struct wlr_layer_surface_v1 *)surface->role_data;
}

static void layer_surface_configure_destroy(
		struct wlr_layer_surface_v1_configure *configure) {
	if (configure == NULL) {
		return;
	}
	wl_list_remove(&configure->link);
	free(configure);
}

static void layer_surface_handle_ack_configure(struct wl_client *client,
		struct wl_resource *resource, uint32_t serial) {
	struct wlr_layer_surface_v1 *surface = layer_surface_from_resource(resource);

	if (!surface) {
		return;
	}

	// First find the ack'ed configure
	bool found = false;
	struct wlr_layer_surface_v1_configure *configure, *tmp;
	wl_list_for_each(configure, &surface->configure_list, link) {
		if (configure->serial == serial) {
			found = true;
			break;
		}
	}
	if (!found) {
		wl_resource_post_error(resource,
			ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SURFACE_STATE,
			"wrong configure serial: %" PRIu32, serial);
		return;
	}
	// Then remove old configures from the list
	wl_list_for_each_safe(configure, tmp, &surface->configure_list, link) {
		if (configure->serial == serial) {
			break;
		}
		layer_surface_configure_destroy(configure);
	}

	surface->pending.configure_serial = configure->serial;
	surface->pending.actual_width = configure->width;
	surface->pending.actual_height = configure->height;

	surface->configured = true;

	layer_surface_configure_destroy(configure);
}

static void layer_surface_handle_set_size(struct wl_client *client,
		struct wl_resource *resource, uint32_t width, uint32_t height) {
	struct wlr_layer_surface_v1 *surface = layer_surface_from_resource(resource);

	if (!surface) {
		return;
	}

	if (surface->current.desired_width == width
			&& surface->current.desired_height == height) {
		surface->pending.committed &= ~WLR_LAYER_SURFACE_V1_STATE_DESIRED_SIZE;
	} else {
		surface->pending.committed |= WLR_LAYER_SURFACE_V1_STATE_DESIRED_SIZE;
	}

	surface->pending.desired_width = width;
	surface->pending.desired_height = height;
}

static void layer_surface_handle_set_anchor(struct wl_client *client,
		struct wl_resource *resource, uint32_t anchor) {
	const uint32_t max_anchor =
		ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	if (anchor > max_anchor) {
		wl_resource_post_error(resource,
			ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_ANCHOR,
			"invalid anchor %" PRIu32, anchor);
	}
	struct wlr_layer_surface_v1 *surface = layer_surface_from_resource(resource);

	if (!surface) {
		return;
	}

	if (surface->current.anchor == anchor) {
		surface->pending.committed &= ~WLR_LAYER_SURFACE_V1_STATE_ANCHOR;
	} else {
		surface->pending.committed |= WLR_LAYER_SURFACE_V1_STATE_ANCHOR;
	}

	surface->pending.anchor = anchor;
}

static void layer_surface_handle_set_exclusive_zone(struct wl_client *client,
		struct wl_resource *resource, int32_t zone) {
	struct wlr_layer_surface_v1 *surface = layer_surface_from_resource(resource);

	if (!surface) {
		return;
	}

	if (surface->current.exclusive_zone == zone) {
		surface->pending.committed &= ~WLR_LAYER_SURFACE_V1_STATE_EXCLUSIVE_ZONE;
	} else {
		surface->pending.committed |= WLR_LAYER_SURFACE_V1_STATE_EXCLUSIVE_ZONE;
	}

	surface->pending.exclusive_zone = zone;
}

static void layer_surface_handle_set_margin(
		struct wl_client *client, struct wl_resource *resource,
		int32_t top, int32_t right, int32_t bottom, int32_t left) {
	struct wlr_layer_surface_v1 *surface = layer_surface_from_resource(resource);

	if (!surface) {
		return;
	}

	if (surface->current.margin.top == top
			&& surface->current.margin.right == right
			&& surface->current.margin.bottom == bottom
			&& surface->current.margin.left == left) {
		surface->pending.committed &= ~WLR_LAYER_SURFACE_V1_STATE_MARGIN;
	} else {
		surface->pending.committed |= WLR_LAYER_SURFACE_V1_STATE_MARGIN;
	}

	surface->pending.margin.top = top;
	surface->pending.margin.right = right;
	surface->pending.margin.bottom = bottom;
	surface->pending.margin.left = left;
}

static void layer_surface_handle_set_keyboard_interactivity(
		struct wl_client *client, struct wl_resource *resource,
		uint32_t interactive) {
	struct wlr_layer_surface_v1 *surface = layer_surface_from_resource(resource);

	if (!surface) {
		return;
	}

	surface->pending.committed |= WLR_LAYER_SURFACE_V1_STATE_KEYBOARD_INTERACTIVITY;
	if (wl_resource_get_version(resource) < ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND_SINCE_VERSION) {
		surface->pending.keyboard_interactive = !!interactive;
	} else {
		if (interactive > ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND) {
			wl_resource_post_error(resource,
				ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_KEYBOARD_INTERACTIVITY,
				"wrong keyboard interactivity value: %" PRIu32, interactive);
		} else {
			surface->pending.keyboard_interactive = interactive;
		}
	}
}

static void layer_surface_handle_get_popup(struct wl_client *client,
		struct wl_resource *layer_resource,
		struct wl_resource *popup_resource) {
	struct wlr_layer_surface_v1 *parent =
		layer_surface_from_resource(layer_resource);
	struct wlr_xdg_popup *popup =
		wlr_xdg_popup_from_resource(popup_resource);

	if (!parent) {
		return;
	}
	popup->parent = parent->surface;
	wl_list_insert(&parent->popups, &popup->link);
	wlr_signal_emit_safe(&parent->events.new_popup, popup);
}

static void layer_surface_set_layer(struct wl_client *client,
		struct wl_resource *surface_resource, uint32_t layer) {
	struct wlr_layer_surface_v1 *surface =
			layer_surface_from_resource(surface_resource);
	if (!surface) {
		return;
	}
	if (layer > ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY) {
		wl_resource_post_error(surface->resource,
				ZWLR_LAYER_SHELL_V1_ERROR_INVALID_LAYER,
				"Invalid layer %" PRIu32, layer);
		return;
	}

	if (surface->current.layer == layer) {
		surface->pending.committed &= ~WLR_LAYER_SURFACE_V1_STATE_LAYER;
	} else {
		surface->pending.committed |= WLR_LAYER_SURFACE_V1_STATE_LAYER;
	}

	surface->pending.layer = layer;
}

static const struct zwlr_layer_surface_v1_interface layer_surface_implementation = {
	.destroy = resource_handle_destroy,
	.ack_configure = layer_surface_handle_ack_configure,
	.set_size = layer_surface_handle_set_size,
	.set_anchor = layer_surface_handle_set_anchor,
	.set_exclusive_zone = layer_surface_handle_set_exclusive_zone,
	.set_margin = layer_surface_handle_set_margin,
	.set_keyboard_interactivity = layer_surface_handle_set_keyboard_interactivity,
	.get_popup = layer_surface_handle_get_popup,
	.set_layer = layer_surface_set_layer,
};

static void layer_surface_unmap(struct wlr_layer_surface_v1 *surface) {
	// TODO: probably need to ungrab before this event
	wlr_signal_emit_safe(&surface->events.unmap, surface);

	struct wlr_xdg_popup *popup, *popup_tmp;
	wl_list_for_each_safe(popup, popup_tmp, &surface->popups, link) {
		wlr_xdg_popup_destroy(popup);
	}

	struct wlr_layer_surface_v1_configure *configure, *tmp;
	wl_list_for_each_safe(configure, tmp, &surface->configure_list, link) {
		layer_surface_configure_destroy(configure);
	}

	surface->configured = surface->mapped = false;
}

static void layer_surface_destroy(struct wlr_layer_surface_v1 *surface) {
	if (surface->configured && surface->mapped) {
		layer_surface_unmap(surface);
	}
	wlr_signal_emit_safe(&surface->events.destroy, surface);
	wl_resource_set_user_data(surface->resource, NULL);
	surface->surface->role_data = NULL;
	wl_list_remove(&surface->surface_destroy.link);
	free(surface->namespace);
	free(surface);
}

static void layer_surface_resource_destroy(struct wl_resource *resource) {
	struct wlr_layer_surface_v1 *surface =
		layer_surface_from_resource(resource);
	if (surface != NULL) {
		layer_surface_destroy(surface);
	}
}

uint32_t wlr_layer_surface_v1_configure(struct wlr_layer_surface_v1 *surface,
		uint32_t width, uint32_t height) {
	struct wl_display *display =
		wl_client_get_display(wl_resource_get_client(surface->resource));
	struct wlr_layer_surface_v1_configure *configure =
		calloc(1, sizeof(struct wlr_layer_surface_v1_configure));
	if (configure == NULL) {
		wl_client_post_no_memory(wl_resource_get_client(surface->resource));
		return surface->pending.configure_serial;
	}
	wl_list_insert(surface->configure_list.prev, &configure->link);
	configure->width = width;
	configure->height = height;
	configure->serial = wl_display_next_serial(display);
	zwlr_layer_surface_v1_send_configure(surface->resource,
			configure->serial, configure->width,
			configure->height);
	return configure->serial;
}

void wlr_layer_surface_v1_destroy(struct wlr_layer_surface_v1 *surface) {
	zwlr_layer_surface_v1_send_closed(surface->resource);
	layer_surface_destroy(surface);
}

static void layer_surface_role_commit(struct wlr_surface *wlr_surface) {
	struct wlr_layer_surface_v1 *surface =
		wlr_layer_surface_v1_from_wlr_surface(wlr_surface);
	if (surface == NULL) {
		return;
	}

	const uint32_t horiz = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	if (surface->pending.desired_width == 0 &&
		(surface->pending.anchor & horiz) != horiz) {
		wl_resource_post_error(surface->resource,
			ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
			"width 0 requested without setting left and right anchors");
		return;
	}

	const uint32_t vert = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
	if (surface->pending.desired_height == 0 &&
		(surface->pending.anchor & vert) != vert) {
		wl_resource_post_error(surface->resource,
			ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
			"height 0 requested without setting top and bottom anchors");
		return;
	}

	surface->current = surface->pending;
	surface->pending.committed = 0;

	if (wlr_surface_has_buffer(surface->surface) && !surface->configured) {
		wl_resource_post_error(surface->resource,
			ZWLR_LAYER_SHELL_V1_ERROR_ALREADY_CONSTRUCTED,
			"layer_surface has never been configured");
		return;
	}

	if (!surface->added) {
		surface->added = true;
		assert(!surface->configured);
		assert(!surface->mapped);
		wlr_signal_emit_safe(&surface->shell->events.new_surface, surface);
		// Return early here as the compositor may have closed this layer surface
		// in response to the new_surface event.
		return;
	}

	if (surface->configured && wlr_surface_has_buffer(surface->surface) &&
			!surface->mapped) {
		surface->mapped = true;
		wlr_signal_emit_safe(&surface->events.map, surface);
	}
}

static void layer_surface_role_precommit(struct wlr_surface *wlr_surface,
		const struct wlr_surface_state *state) {
	struct wlr_layer_surface_v1 *surface =
		wlr_layer_surface_v1_from_wlr_surface(wlr_surface);
	if (surface == NULL) {
		return;
	}

	if (state->committed & WLR_SURFACE_STATE_BUFFER && state->buffer == NULL) {
		// This is a NULL commit
		if (surface->configured && surface->mapped) {
			layer_surface_unmap(surface);
		}
	}
}

static const struct wlr_surface_role layer_surface_role = {
	.name = "zwlr_layer_surface_v1",
	.commit = layer_surface_role_commit,
	.precommit = layer_surface_role_precommit,
};

static void handle_surface_destroyed(struct wl_listener *listener,
		void *data) {
	struct wlr_layer_surface_v1 *layer_surface =
		wl_container_of(listener, layer_surface, surface_destroy);
	layer_surface_destroy(layer_surface);
}

static void layer_shell_handle_get_layer_surface(struct wl_client *wl_client,
		struct wl_resource *client_resource, uint32_t id,
		struct wl_resource *surface_resource,
		struct wl_resource *output_resource,
		uint32_t layer, const char *namespace) {
	struct wlr_layer_shell_v1 *shell =
		layer_shell_from_resource(client_resource);
	struct wlr_surface *wlr_surface =
		wlr_surface_from_resource(surface_resource);

	struct wlr_layer_surface_v1 *surface =
		calloc(1, sizeof(struct wlr_layer_surface_v1));
	if (surface == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	if (!wlr_surface_set_role(wlr_surface, &layer_surface_role, surface,
			client_resource, ZWLR_LAYER_SHELL_V1_ERROR_ROLE)) {
		free(surface);
		return;
	}

	surface->shell = shell;
	surface->surface = wlr_surface;
	if (output_resource) {
		surface->output = wlr_output_from_resource(output_resource);
	}
	surface->current.layer = surface->pending.layer = layer;
	if (layer > ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY) {
		free(surface);
		wl_resource_post_error(client_resource,
				ZWLR_LAYER_SHELL_V1_ERROR_INVALID_LAYER,
				"Invalid layer %" PRIu32, layer);
		return;
	}
	surface->namespace = strdup(namespace);
	if (surface->namespace == NULL) {
		free(surface);
		wl_client_post_no_memory(wl_client);
		return;
	}
	surface->resource = wl_resource_create(wl_client,
		&zwlr_layer_surface_v1_interface,
		wl_resource_get_version(client_resource),
		id);
	if (surface->resource == NULL) {
		free(surface->namespace);
		free(surface);
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_list_init(&surface->configure_list);
	wl_list_init(&surface->popups);

	wl_signal_init(&surface->events.destroy);
	wl_signal_init(&surface->events.map);
	wl_signal_init(&surface->events.unmap);
	wl_signal_init(&surface->events.new_popup);

	wl_signal_add(&surface->surface->events.destroy,
		&surface->surface_destroy);
	surface->surface_destroy.notify = handle_surface_destroyed;

	wlr_log(WLR_DEBUG, "new layer_surface %p (res %p)",
			surface, surface->resource);
	wl_resource_set_implementation(surface->resource,
		&layer_surface_implementation, surface, layer_surface_resource_destroy);
}

static const struct zwlr_layer_shell_v1_interface layer_shell_implementation = {
	.get_layer_surface = layer_shell_handle_get_layer_surface,
	.destroy = resource_handle_destroy,
};

static void layer_shell_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_layer_shell_v1 *layer_shell = data;
	assert(wl_client && layer_shell);

	struct wl_resource *resource = wl_resource_create(
			wl_client, &zwlr_layer_shell_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(resource,
			&layer_shell_implementation, layer_shell, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_layer_shell_v1 *layer_shell =
		wl_container_of(listener, layer_shell, display_destroy);
	wlr_signal_emit_safe(&layer_shell->events.destroy, layer_shell);
	wl_list_remove(&layer_shell->display_destroy.link);
	wl_global_destroy(layer_shell->global);
	free(layer_shell);
}

struct wlr_layer_shell_v1 *wlr_layer_shell_v1_create(struct wl_display *display) {
	struct wlr_layer_shell_v1 *layer_shell =
		calloc(1, sizeof(struct wlr_layer_shell_v1));
	if (!layer_shell) {
		return NULL;
	}

	struct wl_global *global = wl_global_create(display,
		&zwlr_layer_shell_v1_interface, LAYER_SHELL_VERSION,
		layer_shell, layer_shell_bind);
	if (!global) {
		free(layer_shell);
		return NULL;
	}
	layer_shell->global = global;

	wl_signal_init(&layer_shell->events.new_surface);
	wl_signal_init(&layer_shell->events.destroy);

	layer_shell->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &layer_shell->display_destroy);

	return layer_shell;
}

struct layer_surface_iterator_data {
	wlr_surface_iterator_func_t user_iterator;
	void *user_data;
	int x, y;
};

static void layer_surface_iterator(struct wlr_surface *surface,
		int sx, int sy, void *data) {
	struct layer_surface_iterator_data *iter_data = data;
	iter_data->user_iterator(surface, iter_data->x + sx, iter_data->y + sy,
		iter_data->user_data);
}

void wlr_layer_surface_v1_for_each_surface(struct wlr_layer_surface_v1 *surface,
		wlr_surface_iterator_func_t iterator, void *user_data) {
	wlr_surface_for_each_surface(surface->surface, iterator, user_data);
	wlr_layer_surface_v1_for_each_popup_surface(surface, iterator, user_data);
}

void wlr_layer_surface_v1_for_each_popup_surface(struct wlr_layer_surface_v1 *surface,
		wlr_surface_iterator_func_t iterator, void *user_data){
	struct wlr_xdg_popup *popup_state;
	wl_list_for_each(popup_state, &surface->popups, link) {
		struct wlr_xdg_surface *popup = popup_state->base;
		if (!popup->configured || !popup->mapped) {
			continue;
		}

		double popup_sx, popup_sy;
		popup_sx = popup->popup->geometry.x - popup->current.geometry.x;
		popup_sy = popup->popup->geometry.y - popup->current.geometry.y;

		struct layer_surface_iterator_data data = {
			.user_iterator = iterator,
			.user_data = user_data,
			.x = popup_sx, .y = popup_sy,
		};

		wlr_xdg_surface_for_each_surface(popup, layer_surface_iterator, &data);
	}
}

struct wlr_surface *wlr_layer_surface_v1_surface_at(
		struct wlr_layer_surface_v1 *surface, double sx, double sy,
		double *sub_x, double *sub_y) {
	struct wlr_surface *sub = wlr_layer_surface_v1_popup_surface_at(surface,
			sx, sy, sub_x, sub_y);
	if (sub != NULL) {
		return sub;
	}
	return wlr_surface_surface_at(surface->surface, sx, sy, sub_x, sub_y);
}

struct wlr_surface *wlr_layer_surface_v1_popup_surface_at(
		struct wlr_layer_surface_v1 *surface, double sx, double sy,
		double *sub_x, double *sub_y) {
	struct wlr_xdg_popup *popup_state;
	wl_list_for_each(popup_state, &surface->popups, link) {
		struct wlr_xdg_surface *popup = popup_state->base;
		if (!popup->mapped) {
			continue;
		}

		double popup_sx = popup_state->geometry.x - popup->current.geometry.x;
		double popup_sy = popup_state->geometry.y - popup->current.geometry.y;

		struct wlr_surface *sub = wlr_xdg_surface_surface_at(popup,
			sx - popup_sx,
			sy - popup_sy,
			sub_x, sub_y);
		if (sub != NULL) {
			return sub;
		}
	}

	return NULL;
}
