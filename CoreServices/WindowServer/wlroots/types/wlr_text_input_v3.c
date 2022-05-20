#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <wlr/types/wlr_text_input_v3.h>
#include <wlr/util/log.h>
#include "text-input-unstable-v3-protocol.h"
#include "util/signal.h"

static void text_input_clear_focused_surface(struct wlr_text_input_v3 *text_input) {
	wl_list_remove(&text_input->surface_destroy.link);
	wl_list_init(&text_input->surface_destroy.link);
	text_input->focused_surface = NULL;
}

static const struct zwp_text_input_v3_interface text_input_impl;

static struct wlr_text_input_v3 *text_input_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwp_text_input_v3_interface,
		&text_input_impl));
	return wl_resource_get_user_data(resource);
}

void wlr_text_input_v3_send_enter(struct wlr_text_input_v3 *text_input,
		struct wlr_surface *surface) {
	assert(wl_resource_get_client(text_input->resource)
		== wl_resource_get_client(surface->resource));
	text_input->focused_surface = surface;
	wl_signal_add(&text_input->focused_surface->events.destroy,
		&text_input->surface_destroy);
	zwp_text_input_v3_send_enter(text_input->resource,
		text_input->focused_surface->resource);
}

void wlr_text_input_v3_send_leave(struct wlr_text_input_v3 *text_input) {
	zwp_text_input_v3_send_leave(text_input->resource,
		text_input->focused_surface->resource);
	text_input_clear_focused_surface(text_input);
}

void wlr_text_input_v3_send_preedit_string(struct wlr_text_input_v3 *text_input,
		const char *text, int32_t cursor_begin, int32_t cursor_end) {
	zwp_text_input_v3_send_preedit_string(text_input->resource, text,
		cursor_begin, cursor_end);
}

void wlr_text_input_v3_send_commit_string(struct wlr_text_input_v3 *text_input,
		const char *text) {
	zwp_text_input_v3_send_commit_string(text_input->resource, text);
}

void wlr_text_input_v3_send_delete_surrounding_text(
		struct wlr_text_input_v3 *text_input, uint32_t before_length,
		uint32_t after_length) {
	zwp_text_input_v3_send_delete_surrounding_text(text_input->resource,
		before_length, after_length);
}

void wlr_text_input_v3_send_done(struct wlr_text_input_v3 *text_input) {
	zwp_text_input_v3_send_done(text_input->resource,
		text_input->current_serial);
}

static void wlr_text_input_destroy(struct wlr_text_input_v3 *text_input) {
	wlr_signal_emit_safe(&text_input->events.destroy, text_input);
	text_input_clear_focused_surface(text_input);
	wl_list_remove(&text_input->seat_destroy.link);
	// remove from manager::text_inputs
	wl_list_remove(&text_input->link);
	free(text_input->current.surrounding.text);
	free(text_input->pending.surrounding.text);
	free(text_input);
}

static void text_input_resource_destroy(struct wl_resource *resource) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	wlr_text_input_destroy(text_input);
}

static void text_input_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void text_input_enable(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	struct wlr_text_input_v3_state defaults = {0};
	free(text_input->pending.surrounding.text);
	text_input->pending = defaults;
	text_input->pending_enabled = true;
}

static void text_input_disable(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	text_input->pending_enabled = false;
}

static void text_input_set_surrounding_text(struct wl_client *client,
		struct wl_resource *resource, const char *text, int32_t cursor,
		int32_t anchor) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	free(text_input->pending.surrounding.text);
	text_input->pending.surrounding.text = strdup(text);
	if (!text_input->pending.surrounding.text) {
		wl_client_post_no_memory(client);
	}
	text_input->pending.features |= WLR_TEXT_INPUT_V3_FEATURE_SURROUNDING_TEXT;
	text_input->pending.surrounding.cursor = cursor;
	text_input->pending.surrounding.anchor = anchor;
}

static void text_input_set_text_change_cause(struct wl_client *client,
		struct wl_resource *resource, uint32_t cause) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	text_input->pending.text_change_cause = cause;
}

static void text_input_set_content_type(struct wl_client *client,
		struct wl_resource *resource, uint32_t hint, uint32_t purpose) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	text_input->pending.features |= WLR_TEXT_INPUT_V3_FEATURE_CONTENT_TYPE;
	text_input->pending.content_type.hint = hint;
	text_input->pending.content_type.purpose = purpose;
}

static void text_input_set_cursor_rectangle(struct wl_client *client,
		struct wl_resource *resource, int32_t x, int32_t y, int32_t width,
		int32_t height) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	text_input->pending.features |= WLR_TEXT_INPUT_V3_FEATURE_CURSOR_RECTANGLE;
	text_input->pending.cursor_rectangle.x = x;
	text_input->pending.cursor_rectangle.y = y;
	text_input->pending.cursor_rectangle.width = width;
	text_input->pending.cursor_rectangle.height = height;
}

static void text_input_commit(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_text_input_v3 *text_input = text_input_from_resource(resource);
	if (!text_input) {
		return;
	}
	free(text_input->current.surrounding.text);
	text_input->current = text_input->pending;
	if (text_input->pending.surrounding.text) {
		text_input->current.surrounding.text =
			strdup(text_input->pending.surrounding.text);
		if (text_input->current.surrounding.text == NULL) {
			wl_client_post_no_memory(client);
			return;
		}
	}

	bool old_enabled = text_input->current_enabled;
	text_input->current_enabled = text_input->pending_enabled;
	text_input->current_serial++;

	if (text_input->focused_surface == NULL) {
		wlr_log(WLR_DEBUG, "Text input commit received without focus");
	}

	if (!old_enabled && text_input->current_enabled) {
		text_input->active_features	= text_input->current.features;
		wlr_signal_emit_safe(&text_input->events.enable, text_input);
	} else if (old_enabled && !text_input->current_enabled) {
		text_input->active_features	= 0;
		wlr_signal_emit_safe(&text_input->events.disable, text_input);
	} else { // including never enabled
		wlr_signal_emit_safe(&text_input->events.commit, text_input);
	}
}

static const struct zwp_text_input_v3_interface text_input_impl = {
	.destroy = text_input_destroy,
	.enable = text_input_enable,
	.disable = text_input_disable,
	.set_surrounding_text = text_input_set_surrounding_text,
	.set_text_change_cause = text_input_set_text_change_cause,
	.set_content_type = text_input_set_content_type,
	.set_cursor_rectangle = text_input_set_cursor_rectangle,
	.commit = text_input_commit,
};

static const struct zwp_text_input_manager_v3_interface text_input_manager_impl;

static struct wlr_text_input_manager_v3 *text_input_manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwp_text_input_manager_v3_interface, &text_input_manager_impl));
	return wl_resource_get_user_data(resource);
}

static void text_input_manager_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void text_input_handle_seat_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_text_input_v3 *text_input = wl_container_of(listener, text_input,
		seat_destroy);
	struct wl_resource *resource = text_input->resource;
	wlr_text_input_destroy(text_input);
	wl_resource_set_user_data(resource, NULL);
}

static void text_input_handle_focused_surface_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_text_input_v3 *text_input = wl_container_of(listener, text_input,
		surface_destroy);
	text_input_clear_focused_surface(text_input);
}

static void text_input_manager_get_text_input(struct wl_client *client,
		struct wl_resource *resource, uint32_t id, struct wl_resource *seat) {
	struct wlr_text_input_v3 *text_input =
		calloc(1, sizeof(struct wlr_text_input_v3));
	if (text_input == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_signal_init(&text_input->events.enable);
	wl_signal_init(&text_input->events.commit);
	wl_signal_init(&text_input->events.disable);
	wl_signal_init(&text_input->events.destroy);

	int version = wl_resource_get_version(resource);
	struct wl_resource *text_input_resource = wl_resource_create(client,
		&zwp_text_input_v3_interface, version, id);
	if (text_input_resource == NULL) {
		free(text_input);
		wl_client_post_no_memory(client);
		return;
	}
	text_input->resource = text_input_resource;

	wl_resource_set_implementation(text_input->resource, &text_input_impl,
		text_input, text_input_resource_destroy);

	struct wlr_seat_client *seat_client = wlr_seat_client_from_resource(seat);
	struct wlr_seat *wlr_seat = seat_client->seat;
	text_input->seat = wlr_seat;
	wl_signal_add(&seat_client->events.destroy,
		&text_input->seat_destroy);
	text_input->seat_destroy.notify =
		text_input_handle_seat_destroy;
	text_input->surface_destroy.notify =
		text_input_handle_focused_surface_destroy;
	wl_list_init(&text_input->surface_destroy.link);

	struct wlr_text_input_manager_v3 *manager =
		text_input_manager_from_resource(resource);
	wl_list_insert(&manager->text_inputs, &text_input->link);

	wlr_signal_emit_safe(&manager->events.text_input, text_input);
}

static const struct zwp_text_input_manager_v3_interface
		text_input_manager_impl = {
	.destroy = text_input_manager_destroy,
	.get_text_input = text_input_manager_get_text_input,
};

static void text_input_manager_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_text_input_manager_v3 *manager = data;
	assert(wl_client && manager);

	struct wl_resource *resource = wl_resource_create(wl_client,
		&zwp_text_input_manager_v3_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(resource, &text_input_manager_impl,
		manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_text_input_manager_v3 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_text_input_manager_v3 *wlr_text_input_manager_v3_create(
		struct wl_display *display) {
	struct wlr_text_input_manager_v3 *manager =
		calloc(1, sizeof(struct wlr_text_input_manager_v3));
	if (!manager) {
		return NULL;
	}

	wl_list_init(&manager->text_inputs);
	wl_signal_init(&manager->events.text_input);
	wl_signal_init(&manager->events.destroy);

	manager->global = wl_global_create(display,
		&zwp_text_input_manager_v3_interface, 1, manager,
		text_input_manager_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
