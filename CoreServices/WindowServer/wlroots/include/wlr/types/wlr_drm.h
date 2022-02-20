/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_DRM_H
#define WLR_TYPES_WLR_DRM_H

#include <wayland-server-protocol.h>

struct wlr_renderer;

struct wlr_drm_buffer {
	struct wlr_buffer base;

	struct wl_resource *resource; // can be NULL if the client destroyed it
	struct wlr_dmabuf_attributes dmabuf;

	struct wl_listener release;
};

/**
 * A stub implementation of Mesa's wl_drm protocol.
 *
 * It only implements the minimum necessary for modern clients to behave
 * properly. In particular, flink handles are left unimplemented.
 */
struct wlr_drm {
	struct wl_global *global;
	struct wlr_renderer *renderer;
	char *node_name;

	struct {
		struct wl_signal destroy;
	} events;

	struct wl_listener display_destroy;
	struct wl_listener renderer_destroy;
};

bool wlr_drm_buffer_is_resource(struct wl_resource *resource);

struct wlr_drm_buffer *wlr_drm_buffer_from_resource(
	struct wl_resource *resource);

struct wlr_drm *wlr_drm_create(struct wl_display *display,
	struct wlr_renderer *renderer);

#endif
