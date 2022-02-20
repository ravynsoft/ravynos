/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_SCREENCOPY_V1_H
#define WLR_TYPES_WLR_SCREENCOPY_V1_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/util/box.h>

struct wlr_screencopy_manager_v1 {
	struct wl_global *global;
	struct wl_list frames; // wlr_screencopy_frame_v1::link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_screencopy_v1_client {
	int ref;
	struct wlr_screencopy_manager_v1 *manager;
	struct wl_list damages;
};

struct wlr_screencopy_frame_v1 {
	struct wl_resource *resource;
	struct wlr_screencopy_v1_client *client;
	struct wl_list link;

	enum wl_shm_format format;
	uint32_t fourcc;
	struct wlr_box box;
	int stride;

	bool overlay_cursor, cursor_locked;

	bool with_damage;

	struct wl_shm_buffer *shm_buffer;
	struct wlr_dmabuf_v1_buffer *dma_buffer;

	struct wl_listener buffer_destroy;

	struct wlr_output *output;
	struct wl_listener output_commit;
	struct wl_listener output_destroy;
	struct wl_listener output_enable;

	void *data;
};

struct wlr_screencopy_manager_v1 *wlr_screencopy_manager_v1_create(
	struct wl_display *display);

#endif
