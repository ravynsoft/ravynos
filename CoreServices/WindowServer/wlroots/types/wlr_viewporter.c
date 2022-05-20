#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "viewporter-protocol.h"

#define VIEWPORTER_VERSION 1

struct wlr_viewport {
	struct wl_resource *resource;
	struct wlr_surface *surface;

	struct wl_listener surface_destroy;
	struct wl_listener surface_commit;
};

static const struct wp_viewport_interface viewport_impl;

// Returns NULL if the viewport is inert
static struct wlr_viewport *viewport_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wp_viewport_interface,
		&viewport_impl));
	return wl_resource_get_user_data(resource);
}

static void viewport_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void viewport_handle_set_source(struct wl_client *client,
		struct wl_resource *resource, wl_fixed_t x_fixed, wl_fixed_t y_fixed,
		wl_fixed_t width_fixed, wl_fixed_t height_fixed) {
	struct wlr_viewport *viewport = viewport_from_resource(resource);
	if (viewport == NULL) {
		wl_resource_post_error(resource, WP_VIEWPORT_ERROR_NO_SURFACE,
			"wp_viewport.set_source sent after wl_surface has been destroyed");
		return;
	}

	struct wlr_surface_state *pending = &viewport->surface->pending;

	double x = wl_fixed_to_double(x_fixed);
	double y = wl_fixed_to_double(y_fixed);
	double width = wl_fixed_to_double(width_fixed);
	double height = wl_fixed_to_double(height_fixed);

	if (x == -1.0 && y == -1.0 && width == -1.0 && height == -1.0) {
		pending->viewport.has_src = false;
	} else if (x < 0 || y < 0 || width <= 0 || height <= 0) {
		wl_resource_post_error(resource, WP_VIEWPORT_ERROR_BAD_VALUE,
			"wl_viewport.set_source sent with invalid values");
		return;
	} else {
		pending->viewport.has_src = true;
	}

	pending->viewport.src.x = x;
	pending->viewport.src.y = y;
	pending->viewport.src.width = width;
	pending->viewport.src.height = height;

	pending->committed |= WLR_SURFACE_STATE_VIEWPORT;
}

static void viewport_handle_set_destination(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_viewport *viewport = viewport_from_resource(resource);
	if (viewport == NULL) {
		wl_resource_post_error(resource, WP_VIEWPORT_ERROR_NO_SURFACE,
			"wp_viewport.set_destination sent after wl_surface has been destroyed");
		return;
	}

	struct wlr_surface_state *pending = &viewport->surface->pending;

	if (width == -1 && height == -1) {
		pending->viewport.has_dst = false;
	} else if (width <= 0 || height <= 0) {
		wl_resource_post_error(resource, WP_VIEWPORT_ERROR_BAD_VALUE,
			"wl_viewport.set_destination sent with invalid values");
		return;
	} else {
		pending->viewport.has_dst = true;
	}

	pending->viewport.dst_width = width;
	pending->viewport.dst_height = height;

	pending->committed |= WLR_SURFACE_STATE_VIEWPORT;
}

static const struct wp_viewport_interface viewport_impl = {
	.destroy = viewport_handle_destroy,
	.set_source = viewport_handle_set_source,
	.set_destination = viewport_handle_set_destination,
};

static void viewport_destroy(struct wlr_viewport *viewport) {
	if (viewport == NULL) {
		return;
	}

	struct wlr_surface_state *pending = &viewport->surface->pending;
	pending->viewport.has_src = false;
	pending->viewport.has_dst = false;
	pending->committed |= WLR_SURFACE_STATE_VIEWPORT;

	wl_resource_set_user_data(viewport->resource, NULL);
	wl_list_remove(&viewport->surface_destroy.link);
	wl_list_remove(&viewport->surface_commit.link);
	free(viewport);
}

static void viewport_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_viewport *viewport = viewport_from_resource(resource);
	viewport_destroy(viewport);
}

static void viewport_handle_surface_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_viewport *viewport =
		wl_container_of(listener, viewport, surface_destroy);
	viewport_destroy(viewport);
}

static void viewport_handle_surface_commit(struct wl_listener *listener,
		void *data) {
	struct wlr_viewport *viewport =
		wl_container_of(listener, viewport, surface_commit);

	struct wlr_surface_state *current = &viewport->surface->pending;

	if (!current->viewport.has_dst &&
			(floor(current->viewport.src.width) != current->viewport.src.width ||
			floor(current->viewport.src.height) != current->viewport.src.height)) {
		wl_resource_post_error(viewport->resource, WP_VIEWPORT_ERROR_BAD_SIZE,
			"wl_viewport.set_source width and height must be integers "
			"when the destination rectangle is unset");
		return;
	}

	if (current->viewport.has_src && current->buffer != NULL &&
			(current->viewport.src.x + current->viewport.src.width >
				current->buffer_width ||
			current->viewport.src.y + current->viewport.src.height >
				current->buffer_height)) {
		wl_resource_post_error(viewport->resource, WP_VIEWPORT_ERROR_OUT_OF_BUFFER,
			"source rectangle out of buffer bounds");
		return;
	}
}

static void viewporter_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void viewporter_handle_get_viewport(struct wl_client *client,
		struct wl_resource *resource, uint32_t id,
		struct wl_resource *surface_resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);

	struct wlr_viewport *viewport = calloc(1, sizeof(*viewport));
	if (viewport == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	uint32_t version = wl_resource_get_version(resource);
	viewport->resource = wl_resource_create(client, &wp_viewport_interface,
		version, id);
	if (viewport->resource == NULL) {
		wl_client_post_no_memory(client);
		free(viewport);
		return;
	}
	wl_resource_set_implementation(viewport->resource, &viewport_impl,
		viewport, viewport_handle_resource_destroy);

	viewport->surface = surface;

	viewport->surface_destroy.notify = viewport_handle_surface_destroy;
	wl_signal_add(&surface->events.destroy, &viewport->surface_destroy);

	viewport->surface_commit.notify = viewport_handle_surface_commit;
	wl_signal_add(&surface->events.commit, &viewport->surface_commit);
}

static const struct wp_viewporter_interface viewporter_impl = {
	.destroy = viewporter_handle_destroy,
	.get_viewport = viewporter_handle_get_viewport,
};

static void viewporter_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_viewporter *viewporter = data;

	struct wl_resource *resource = wl_resource_create(client,
		&wp_viewporter_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &viewporter_impl, viewporter, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_viewporter *viewporter =
		wl_container_of(listener, viewporter, display_destroy);
	wlr_signal_emit_safe(&viewporter->events.destroy, NULL);
	wl_global_destroy(viewporter->global);
	free(viewporter);
}

struct wlr_viewporter *wlr_viewporter_create(struct wl_display *display) {
	struct wlr_viewporter *viewporter = calloc(1, sizeof(*viewporter));
	if (viewporter == NULL) {
		return NULL;
	}

	viewporter->global = wl_global_create(display, &wp_viewporter_interface,
		VIEWPORTER_VERSION, viewporter, viewporter_bind);
	if (viewporter->global == NULL) {
		free(viewporter);
		return NULL;
	}

	wl_signal_init(&viewporter->events.destroy);

	viewporter->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &viewporter->display_destroy);

	return viewporter;
}
