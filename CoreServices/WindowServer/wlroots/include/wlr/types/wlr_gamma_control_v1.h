#ifndef WLR_TYPES_WLR_GAMMA_CONTROL_V1_H
#define WLR_TYPES_WLR_GAMMA_CONTROL_V1_H

#include <wayland-server-core.h>

struct wlr_gamma_control_manager_v1 {
	struct wl_global *global;
	struct wl_list controls; // wlr_gamma_control_v1::link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_gamma_control_v1 {
	struct wl_resource *resource;
	struct wlr_output *output;
	struct wl_list link;

	uint16_t *table;
	size_t ramp_size;

	struct wl_listener output_commit_listener;
	struct wl_listener output_destroy_listener;

	void *data;
};

struct wlr_gamma_control_manager_v1 *wlr_gamma_control_manager_v1_create(
	struct wl_display *display);

#endif
