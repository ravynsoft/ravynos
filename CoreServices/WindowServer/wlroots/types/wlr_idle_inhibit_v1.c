#include <assert.h>
#include <stdlib.h>
#include <util/signal.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/util/log.h>
#include "idle-inhibit-unstable-v1-protocol.h"

static const struct zwp_idle_inhibit_manager_v1_interface idle_inhibit_impl;

static const struct zwp_idle_inhibitor_v1_interface idle_inhibitor_impl;

static struct wlr_idle_inhibit_manager_v1 *
wlr_idle_inhibit_manager_v1_from_resource(struct wl_resource *manager_resource) {
	assert(wl_resource_instance_of(manager_resource,
		&zwp_idle_inhibit_manager_v1_interface,
		&idle_inhibit_impl));
	return wl_resource_get_user_data(manager_resource);
}

static struct wlr_idle_inhibitor_v1 *
wlr_idle_inhibitor_v1_from_resource(struct wl_resource *inhibitor_resource) {
	assert(wl_resource_instance_of(inhibitor_resource,
		&zwp_idle_inhibitor_v1_interface,
		&idle_inhibitor_impl));
	return wl_resource_get_user_data(inhibitor_resource);
}

static void idle_inhibitor_v1_destroy(struct wlr_idle_inhibitor_v1 *inhibitor) {
	if (!inhibitor) {
		return;
	}

	wlr_signal_emit_safe(&inhibitor->events.destroy, inhibitor->surface);

	wl_resource_set_user_data(inhibitor->resource, NULL);
	wl_list_remove(&inhibitor->link);
	wl_list_remove(&inhibitor->surface_destroy.link);
	free(inhibitor);
}

static void idle_inhibitor_v1_handle_resource_destroy(
		struct wl_resource *inhibitor_resource) {
	struct wlr_idle_inhibitor_v1 *inhibitor =
		wlr_idle_inhibitor_v1_from_resource(inhibitor_resource);
	idle_inhibitor_v1_destroy(inhibitor);
}

static void idle_inhibitor_handle_surface_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_idle_inhibitor_v1 *inhibitor =
		wl_container_of(listener, inhibitor, surface_destroy);
	idle_inhibitor_v1_destroy(inhibitor);
}

static void idle_inhibitor_v1_handle_destroy(struct wl_client *client,
		struct wl_resource *inhibitor_resource) {
	wl_resource_destroy(inhibitor_resource);
}

static const struct zwp_idle_inhibitor_v1_interface idle_inhibitor_impl = {
	.destroy = idle_inhibitor_v1_handle_destroy,
};

static void manager_handle_create_inhibitor(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *surface_resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);
	struct wlr_idle_inhibit_manager_v1 *manager =
		wlr_idle_inhibit_manager_v1_from_resource(manager_resource);

	struct wlr_idle_inhibitor_v1 *inhibitor =
		calloc(1, sizeof(struct wlr_idle_inhibitor_v1));
	if (!inhibitor) {
		wl_client_post_no_memory(client);
		return;
	}

	uint32_t version = wl_resource_get_version(manager_resource);
	struct wl_resource *inhibitor_resource = wl_resource_create(client,
		&zwp_idle_inhibitor_v1_interface, version, id);
	if (!inhibitor_resource) {
		wl_client_post_no_memory(client);
		free(inhibitor);
		return;
	}

	inhibitor->resource = inhibitor_resource;
	inhibitor->surface = surface;
	wl_signal_init(&inhibitor->events.destroy);

	inhibitor->surface_destroy.notify = idle_inhibitor_handle_surface_destroy;
	wl_signal_add(&surface->events.destroy, &inhibitor->surface_destroy);

	wl_resource_set_implementation(inhibitor_resource, &idle_inhibitor_impl,
		inhibitor, idle_inhibitor_v1_handle_resource_destroy);

	wl_list_insert(&manager->inhibitors, &inhibitor->link);
	wlr_signal_emit_safe(&manager->events.new_inhibitor, inhibitor);
}

static void manager_handle_destroy(struct wl_client *client,
		struct wl_resource *manager_resource) {
	wl_resource_destroy(manager_resource);
}

static const struct zwp_idle_inhibit_manager_v1_interface idle_inhibit_impl = {
	.destroy = manager_handle_destroy,
	.create_inhibitor = manager_handle_create_inhibitor,
};

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_idle_inhibit_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

static void idle_inhibit_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_idle_inhibit_manager_v1 *manager = data;

	struct wl_resource *manager_resource  = wl_resource_create(wl_client,
		&zwp_idle_inhibit_manager_v1_interface, version, id);
	if (!manager_resource) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_resource_set_implementation(manager_resource, &idle_inhibit_impl,
		manager, NULL);
}

struct wlr_idle_inhibit_manager_v1 *wlr_idle_inhibit_v1_create(struct wl_display *display) {
	struct wlr_idle_inhibit_manager_v1 *manager =
		calloc(1, sizeof(struct wlr_idle_inhibit_manager_v1));
	if (!manager) {
		return NULL;
	}

	wl_list_init(&manager->inhibitors);
	wl_signal_init(&manager->events.new_inhibitor);
	wl_signal_init(&manager->events.destroy);

	manager->global = wl_global_create(display,
		&zwp_idle_inhibit_manager_v1_interface, 1,
		manager, idle_inhibit_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
