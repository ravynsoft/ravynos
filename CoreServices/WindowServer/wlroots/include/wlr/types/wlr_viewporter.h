/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_VIEWPORTER_H
#define WLR_TYPES_WLR_VIEWPORTER_H

#include <wayland-server-core.h>

/**
 * Implementation for the viewporter protocol.
 *
 * When enabling viewporter, compositors need to update their rendering logic:
 *
 * - The size of the surface texture may not match the surface size anymore.
 *   Compositors must use the surface size only.
 * - Compositors must call wlr_render_subtexture_with_matrix when rendering a
 *   surface texture with the source box returned by
 *   wlr_surface_get_buffer_source_box.
 */
struct wlr_viewporter {
	struct wl_global *global;

	struct {
		struct wl_signal destroy;
	} events;

	struct wl_listener display_destroy;
};

struct wlr_viewporter *wlr_viewporter_create(struct wl_display *display);

#endif
