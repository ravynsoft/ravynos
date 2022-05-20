#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/log.h>
#include "types/wlr_region.h"
#include "types/wlr_surface.h"
#include "util/signal.h"

#define COMPOSITOR_VERSION 4
#define SUBCOMPOSITOR_VERSION 1

extern const struct wlr_surface_role subsurface_role;

bool wlr_surface_is_subsurface(struct wlr_surface *surface) {
	return surface->role == &subsurface_role;
}

struct wlr_subsurface *wlr_subsurface_from_wlr_surface(
		struct wlr_surface *surface) {
	assert(wlr_surface_is_subsurface(surface));
	return (struct wlr_subsurface *)surface->role_data;
}

static void subcompositor_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void subcompositor_handle_get_subsurface(struct wl_client *client,
		struct wl_resource *resource, uint32_t id,
		struct wl_resource *surface_resource,
		struct wl_resource *parent_resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);
	struct wlr_surface *parent = wlr_surface_from_resource(parent_resource);

	static const char msg[] = "get_subsurface: wl_subsurface@";

	if (surface == parent) {
		wl_resource_post_error(resource,
			WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
			"%s%" PRIu32 ": wl_surface@%" PRIu32 " cannot be its own parent",
			msg, id, wl_resource_get_id(surface_resource));
		return;
	}

	if (wlr_surface_is_subsurface(surface) &&
			wlr_subsurface_from_wlr_surface(surface) != NULL) {
		wl_resource_post_error(resource,
			WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
			"%s%" PRIu32 ": wl_surface@%" PRIu32 " is already a sub-surface",
			msg, id, wl_resource_get_id(surface_resource));
		return;
	}

	if (wlr_surface_get_root_surface(parent) == surface) {
		wl_resource_post_error(resource,
			WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
			"%s%" PRIu32 ": wl_surface@%" PRIu32 " is an ancestor of parent",
			msg, id, wl_resource_get_id(surface_resource));
		return;
	}

	if (!wlr_surface_set_role(surface, &subsurface_role, NULL,
			resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE)) {
		return;
	}

	subsurface_create(surface, parent, wl_resource_get_version(resource), id);
}

static const struct wl_subcompositor_interface subcompositor_impl = {
	.destroy = subcompositor_handle_destroy,
	.get_subsurface = subcompositor_handle_get_subsurface,
};

static void subcompositor_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_subcompositor *subcompositor = data;
	struct wl_resource *resource =
		wl_resource_create(client, &wl_subcompositor_interface, 1, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &subcompositor_impl,
		subcompositor, NULL);
}

static bool subcompositor_init(struct wlr_subcompositor *subcompositor,
		struct wl_display *display) {
	subcompositor->global = wl_global_create(display,
		&wl_subcompositor_interface, SUBCOMPOSITOR_VERSION, subcompositor,
		subcompositor_bind);
	if (subcompositor->global == NULL) {
		wlr_log_errno(WLR_ERROR, "Could not allocate subcompositor global");
		return false;
	}

	return true;
}

static void subcompositor_finish(struct wlr_subcompositor *subcompositor) {
	wl_global_destroy(subcompositor->global);
}


static const struct wl_compositor_interface compositor_impl;

static struct wlr_compositor *compositor_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_compositor_interface,
		&compositor_impl));
	return wl_resource_get_user_data(resource);
}

static void compositor_create_surface(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	struct wlr_compositor *compositor = compositor_from_resource(resource);

	struct wlr_surface *surface = surface_create(client,
		wl_resource_get_version(resource), id, compositor->renderer);
	if (surface == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wlr_signal_emit_safe(&compositor->events.new_surface, surface);
}

static void compositor_create_region(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	region_create(client, wl_resource_get_version(resource), id);
}

static const struct wl_compositor_interface compositor_impl = {
	.create_surface = compositor_create_surface,
	.create_region = compositor_create_region,
};

static void compositor_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_compositor *compositor = data;

	struct wl_resource *resource =
		wl_resource_create(wl_client, &wl_compositor_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(resource, &compositor_impl, compositor, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_compositor *compositor =
		wl_container_of(listener, compositor, display_destroy);
	wlr_signal_emit_safe(&compositor->events.destroy, compositor);
	subcompositor_finish(&compositor->subcompositor);
	wl_list_remove(&compositor->display_destroy.link);
	wl_global_destroy(compositor->global);
	free(compositor);
}

struct wlr_compositor *wlr_compositor_create(struct wl_display *display,
		struct wlr_renderer *renderer) {
	struct wlr_compositor *compositor =
		calloc(1, sizeof(struct wlr_compositor));
	if (!compositor) {
		wlr_log_errno(WLR_ERROR, "Could not allocate wlr compositor");
		return NULL;
	}

	compositor->global = wl_global_create(display, &wl_compositor_interface,
		COMPOSITOR_VERSION, compositor, compositor_bind);
	if (!compositor->global) {
		free(compositor);
		wlr_log_errno(WLR_ERROR, "Could not allocate compositor global");
		return NULL;
	}
	compositor->renderer = renderer;

	wl_signal_init(&compositor->events.new_surface);
	wl_signal_init(&compositor->events.destroy);

	subcompositor_init(&compositor->subcompositor, display);

	compositor->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &compositor->display_destroy);

	return compositor;
}
