#include <assert.h>
#include <stdlib.h>
#include <util/signal.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_keyboard_shortcuts_inhibit_v1.h>
#include "keyboard-shortcuts-inhibit-unstable-v1-protocol.h"

static const struct zwp_keyboard_shortcuts_inhibit_manager_v1_interface
	keyboard_shortcuts_inhibit_impl;

static const struct zwp_keyboard_shortcuts_inhibitor_v1_interface
	keyboard_shortcuts_inhibitor_impl;

static struct wlr_keyboard_shortcuts_inhibit_manager_v1 *
wlr_keyboard_shortcuts_inhibit_manager_v1_from_resource(
		struct wl_resource *manager_resource) {
	assert(wl_resource_instance_of(manager_resource,
		&zwp_keyboard_shortcuts_inhibit_manager_v1_interface,
		&keyboard_shortcuts_inhibit_impl));
	return wl_resource_get_user_data(manager_resource);
}

static struct wlr_keyboard_shortcuts_inhibitor_v1 *
wlr_keyboard_shortcuts_inhibitor_v1_from_resource(
		struct wl_resource *inhibitor_resource) {
	assert(wl_resource_instance_of(inhibitor_resource,
		&zwp_keyboard_shortcuts_inhibitor_v1_interface,
		&keyboard_shortcuts_inhibitor_impl));
	return wl_resource_get_user_data(inhibitor_resource);
}

static void keyboard_shortcuts_inhibitor_v1_destroy(
		struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor) {
	if (!inhibitor) {
		return;
	}

	wlr_signal_emit_safe(&inhibitor->events.destroy, inhibitor);

	wl_resource_set_user_data(inhibitor->resource, NULL);
	wl_list_remove(&inhibitor->link);
	wl_list_remove(&inhibitor->surface_destroy.link);
	wl_list_remove(&inhibitor->seat_destroy.link);
	free(inhibitor);
}

static void keyboard_shortcuts_inhibitor_v1_handle_resource_destroy(
		struct wl_resource *inhibitor_resource) {
	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor =
		wlr_keyboard_shortcuts_inhibitor_v1_from_resource(
				inhibitor_resource);
	keyboard_shortcuts_inhibitor_v1_destroy(inhibitor);
}

static void keyboard_shortcuts_inhibitor_handle_surface_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor =
		wl_container_of(listener, inhibitor, surface_destroy);

	// be gracious and notify client that destruction of a referenced
	// resource makes inhibitor moot
	wlr_keyboard_shortcuts_inhibitor_v1_deactivate(inhibitor);
	keyboard_shortcuts_inhibitor_v1_destroy(inhibitor);
}

static void keyboard_shortcuts_inhibitor_handle_seat_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor =
		wl_container_of(listener, inhibitor, seat_destroy);
	wlr_keyboard_shortcuts_inhibitor_v1_deactivate(inhibitor);
	keyboard_shortcuts_inhibitor_v1_destroy(inhibitor);
}

static void keyboard_shortcuts_inhibitor_v1_handle_destroy(
		struct wl_client *client,
		struct wl_resource *inhibitor_resource) {
	wl_resource_destroy(inhibitor_resource);
}

static const struct zwp_keyboard_shortcuts_inhibitor_v1_interface
keyboard_shortcuts_inhibitor_impl = {
	.destroy = keyboard_shortcuts_inhibitor_v1_handle_destroy,
};

static void manager_handle_inhibit_shortcuts(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *surface_resource,
		struct wl_resource *seat_resource) {
	struct wlr_surface *surface =
		wlr_surface_from_resource(surface_resource);
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);
	struct wlr_keyboard_shortcuts_inhibit_manager_v1 *manager =
		wlr_keyboard_shortcuts_inhibit_manager_v1_from_resource(
				manager_resource);

	struct wlr_seat *seat = seat_client->seat;
	struct wlr_keyboard_shortcuts_inhibitor_v1 *existing_inhibitor;
	wl_list_for_each(existing_inhibitor, &manager->inhibitors, link) {
		if (existing_inhibitor->surface != surface ||
				existing_inhibitor->seat != seat) {
			continue;
		}

		wl_resource_post_error(manager_resource,
			ZWP_KEYBOARD_SHORTCUTS_INHIBIT_MANAGER_V1_ERROR_ALREADY_INHIBITED,
			"this surface already has keyboard shortcuts "
			"inhibited on this seat");
		return;
	}

	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor =
		calloc(1, sizeof(struct wlr_keyboard_shortcuts_inhibitor_v1));
	if (!inhibitor) {
		wl_client_post_no_memory(client);
		return;
	}

	uint32_t version = wl_resource_get_version(manager_resource);
	struct wl_resource *inhibitor_resource = wl_resource_create(client,
		&zwp_keyboard_shortcuts_inhibitor_v1_interface, version, id);
	if (!inhibitor_resource) {
		wl_client_post_no_memory(client);
		free(inhibitor);
		return;
	}

	inhibitor->resource = inhibitor_resource;
	inhibitor->surface = surface;
	inhibitor->seat = seat;
	inhibitor->active = false;
	wl_signal_init(&inhibitor->events.destroy);

	inhibitor->surface_destroy.notify =
		keyboard_shortcuts_inhibitor_handle_surface_destroy;
	wl_signal_add(&surface->events.destroy, &inhibitor->surface_destroy);

	inhibitor->seat_destroy.notify =
		keyboard_shortcuts_inhibitor_handle_seat_destroy;
	wl_signal_add(&seat->events.destroy, &inhibitor->seat_destroy);

	wl_resource_set_implementation(inhibitor_resource,
		&keyboard_shortcuts_inhibitor_impl, inhibitor,
		keyboard_shortcuts_inhibitor_v1_handle_resource_destroy);

	wl_list_insert(&manager->inhibitors, &inhibitor->link);
	wlr_signal_emit_safe(&manager->events.new_inhibitor, inhibitor);
}

static void manager_handle_destroy(struct wl_client *client,
		struct wl_resource *manager_resource) {
	wl_resource_destroy(manager_resource);
}

static const struct zwp_keyboard_shortcuts_inhibit_manager_v1_interface
keyboard_shortcuts_inhibit_impl = {
	.destroy = manager_handle_destroy,
	.inhibit_shortcuts = manager_handle_inhibit_shortcuts,
};

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_keyboard_shortcuts_inhibit_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

static void keyboard_shortcuts_inhibit_bind(struct wl_client *wl_client,
		void *data, uint32_t version, uint32_t id) {
	struct wlr_keyboard_shortcuts_inhibit_manager_v1 *manager = data;

	struct wl_resource *manager_resource = wl_resource_create(wl_client,
		&zwp_keyboard_shortcuts_inhibit_manager_v1_interface,
		version, id);
	if (!manager_resource) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_resource_set_implementation(manager_resource,
		&keyboard_shortcuts_inhibit_impl, manager, NULL);
}

struct wlr_keyboard_shortcuts_inhibit_manager_v1 *
wlr_keyboard_shortcuts_inhibit_v1_create(struct wl_display *display) {
	struct wlr_keyboard_shortcuts_inhibit_manager_v1 *manager =
		calloc(1, sizeof(struct wlr_keyboard_shortcuts_inhibit_manager_v1));
	if (!manager) {
		return NULL;
	}

	wl_list_init(&manager->inhibitors);
	wl_signal_init(&manager->events.new_inhibitor);
	wl_signal_init(&manager->events.destroy);

	manager->global = wl_global_create(display,
		&zwp_keyboard_shortcuts_inhibit_manager_v1_interface, 1,
		manager, keyboard_shortcuts_inhibit_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}

void wlr_keyboard_shortcuts_inhibitor_v1_activate(
		struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor) {
	if (!inhibitor->active) {
		zwp_keyboard_shortcuts_inhibitor_v1_send_active(
				inhibitor->resource);
		inhibitor->active = true;
	}
}

void wlr_keyboard_shortcuts_inhibitor_v1_deactivate(
		struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor) {
	if (inhibitor->active) {
		zwp_keyboard_shortcuts_inhibitor_v1_send_inactive(
				inhibitor->resource);
		inhibitor->active = false;
	}
}
