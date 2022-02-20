/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_XDG_OUTPUT_V1_H
#define WLR_TYPES_WLR_XDG_OUTPUT_V1_H
#include <wayland-server-core.h>
#include <wlr/types/wlr_output_layout.h>

struct wlr_xdg_output_v1 {
	struct wlr_xdg_output_manager_v1 *manager;
	struct wl_list resources;
	struct wl_list link;

	struct wlr_output_layout_output *layout_output;

	int32_t x, y;
	int32_t width, height;

	struct wl_listener destroy;
	struct wl_listener description;
};

struct wlr_xdg_output_manager_v1 {
	struct wl_global *global;
	struct wlr_output_layout *layout;

	struct wl_list outputs;

	struct {
		struct wl_signal destroy;
	} events;

	struct wl_listener display_destroy;
	struct wl_listener layout_add;
	struct wl_listener layout_change;
	struct wl_listener layout_destroy;
};

struct wlr_xdg_output_manager_v1 *wlr_xdg_output_manager_v1_create(
	struct wl_display *display, struct wlr_output_layout *layout);

#endif
