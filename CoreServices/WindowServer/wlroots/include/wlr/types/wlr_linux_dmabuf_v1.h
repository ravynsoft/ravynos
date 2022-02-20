/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_LINUX_DMABUF_H
#define WLR_TYPES_WLR_LINUX_DMABUF_H

#include <stdint.h>
#include <sys/stat.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/render/dmabuf.h>

struct wlr_surface;

struct wlr_dmabuf_v1_buffer {
	struct wlr_buffer base;

	struct wl_resource *resource; // can be NULL if the client destroyed it
	struct wlr_dmabuf_attributes attributes;

	// private state

	struct wl_listener release;
};

/**
 * Returns true if the given resource was created via the linux-dmabuf
 * buffer protocol, false otherwise
 */
bool wlr_dmabuf_v1_resource_is_buffer(struct wl_resource *buffer_resource);

/**
 * Returns the wlr_dmabuf_buffer if the given resource was created
 * via the linux-dmabuf buffer protocol
 */
struct wlr_dmabuf_v1_buffer *wlr_dmabuf_v1_buffer_from_buffer_resource(
	struct wl_resource *buffer_resource);

struct wlr_linux_dmabuf_feedback_v1 {
	dev_t main_device;
	size_t tranches_len;
	const struct wlr_linux_dmabuf_feedback_v1_tranche *tranches;
};

struct wlr_linux_dmabuf_feedback_v1_tranche {
	dev_t target_device;
	uint32_t flags; // bitfield of enum zwp_linux_dmabuf_feedback_v1_tranche_flags
	const struct wlr_drm_format_set *formats;
};

/* the protocol interface */
struct wlr_linux_dmabuf_v1 {
	struct wl_global *global;
	struct wlr_renderer *renderer;

	struct {
		struct wl_signal destroy;
	} events;

	// private state

	struct wlr_linux_dmabuf_feedback_v1_compiled *default_feedback;
	struct wl_list surfaces; // wlr_linux_dmabuf_v1_surface.link

	struct wl_listener display_destroy;
	struct wl_listener renderer_destroy;
};

/**
 * Create linux-dmabuf interface
 */
struct wlr_linux_dmabuf_v1 *wlr_linux_dmabuf_v1_create(struct wl_display *display,
	struct wlr_renderer *renderer);

/**
 * Set a surface's DMA-BUF feedback.
 *
 * Passing a NULL feedback resets it to the default feedback.
 */
bool wlr_linux_dmabuf_v1_set_surface_feedback(
	struct wlr_linux_dmabuf_v1 *linux_dmabuf, struct wlr_surface *surface,
	const struct wlr_linux_dmabuf_feedback_v1 *feedback);

#endif
