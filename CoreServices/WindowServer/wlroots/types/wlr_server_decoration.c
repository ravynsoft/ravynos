#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/log.h>
#include "server-decoration-protocol.h"
#include "util/signal.h"

static const struct org_kde_kwin_server_decoration_interface
	server_decoration_impl;

static struct wlr_server_decoration *decoration_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&org_kde_kwin_server_decoration_interface, &server_decoration_impl));
	return wl_resource_get_user_data(resource);
}

static void server_decoration_handle_release(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void server_decoration_handle_request_mode(struct wl_client *client,
		struct wl_resource *resource, uint32_t mode) {
	struct wlr_server_decoration *decoration =
		decoration_from_resource(resource);
	if (decoration->mode == mode) {
		return;
	}
	decoration->mode = mode;
	wlr_signal_emit_safe(&decoration->events.mode, decoration);
	org_kde_kwin_server_decoration_send_mode(decoration->resource,
		decoration->mode);
}

static void server_decoration_destroy(
		struct wlr_server_decoration *decoration) {
	wlr_signal_emit_safe(&decoration->events.destroy, decoration);
	wl_list_remove(&decoration->surface_destroy_listener.link);
	wl_resource_set_user_data(decoration->resource, NULL);
	wl_list_remove(&decoration->link);
	free(decoration);
}

static void server_decoration_destroy_resource(struct wl_resource *resource) {
	struct wlr_server_decoration *decoration =
		decoration_from_resource(resource);
	if (decoration != NULL) {
		server_decoration_destroy(decoration);
	}
}

static void server_decoration_handle_surface_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_server_decoration *decoration =
		wl_container_of(listener, decoration, surface_destroy_listener);
	server_decoration_destroy(decoration);
}

static const struct org_kde_kwin_server_decoration_interface
		server_decoration_impl = {
	.release = server_decoration_handle_release,
	.request_mode = server_decoration_handle_request_mode,
};

static const struct org_kde_kwin_server_decoration_manager_interface
	server_decoration_manager_impl;

static struct wlr_server_decoration_manager *manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&org_kde_kwin_server_decoration_manager_interface,
		&server_decoration_manager_impl));
	return wl_resource_get_user_data(resource);
}

static void server_decoration_manager_handle_create(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *surface_resource) {
	struct wlr_server_decoration_manager *manager =
		manager_from_resource(manager_resource);
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);

	struct wlr_server_decoration *decoration =
		calloc(1, sizeof(struct wlr_server_decoration));
	if (decoration == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	decoration->surface = surface;
	decoration->mode = manager->default_mode;

	int version = wl_resource_get_version(manager_resource);
	decoration->resource = wl_resource_create(client,
		&org_kde_kwin_server_decoration_interface, version, id);
	if (decoration->resource == NULL) {
		free(decoration);
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(decoration->resource,
		&server_decoration_impl, decoration,
		server_decoration_destroy_resource);

	wlr_log(WLR_DEBUG, "new server_decoration %p (res %p)", decoration,
		decoration->resource);

	wl_signal_init(&decoration->events.destroy);
	wl_signal_init(&decoration->events.mode);

	wl_signal_add(&surface->events.destroy,
		&decoration->surface_destroy_listener);
	decoration->surface_destroy_listener.notify =
		server_decoration_handle_surface_destroy;

	wl_list_insert(&manager->decorations, &decoration->link);

	org_kde_kwin_server_decoration_send_mode(decoration->resource,
		decoration->mode);

	wlr_signal_emit_safe(&manager->events.new_decoration, decoration);
}

static const struct org_kde_kwin_server_decoration_manager_interface
		server_decoration_manager_impl = {
	.create = server_decoration_manager_handle_create,
};

void wlr_server_decoration_manager_set_default_mode(
		struct wlr_server_decoration_manager *manager, uint32_t default_mode) {
	manager->default_mode = default_mode;

	struct wl_resource *resource;
	wl_resource_for_each(resource, &manager->resources) {
		org_kde_kwin_server_decoration_manager_send_default_mode(resource,
			manager->default_mode);
	}
}

static void server_decoration_manager_destroy_resource(
		struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void server_decoration_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_server_decoration_manager *manager = data;
	assert(client && manager);

	struct wl_resource *resource = wl_resource_create(client,
		&org_kde_kwin_server_decoration_manager_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &server_decoration_manager_impl,
		manager, server_decoration_manager_destroy_resource);

	wl_list_insert(&manager->resources, wl_resource_get_link(resource));

	org_kde_kwin_server_decoration_manager_send_default_mode(resource,
		manager->default_mode);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_server_decoration_manager *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_server_decoration_manager *wlr_server_decoration_manager_create(
		struct wl_display *display) {
	struct wlr_server_decoration_manager *manager =
		calloc(1, sizeof(struct wlr_server_decoration_manager));
	if (manager == NULL) {
		return NULL;
	}
	manager->global = wl_global_create(display,
		&org_kde_kwin_server_decoration_manager_interface, 1, manager,
		server_decoration_manager_bind);
	if (manager->global == NULL) {
		free(manager);
		return NULL;
	}
	manager->default_mode = ORG_KDE_KWIN_SERVER_DECORATION_MANAGER_MODE_NONE;
	wl_list_init(&manager->resources);
	wl_list_init(&manager->decorations);
	wl_signal_init(&manager->events.new_decoration);
	wl_signal_init(&manager->events.destroy);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
