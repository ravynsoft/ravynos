#ifndef WLR_TYPES_WLR_OUTPUT_POWER_MANAGEMENT_V1_H
#define WLR_TYPES_WLR_OUTPUT_POWER_MANAGEMENT_V1_H

#include <wayland-server-core.h>
#include "wlr-output-power-management-unstable-v1-protocol.h"

struct wlr_output_power_manager_v1 {
	struct wl_global *global;
	struct wl_list output_powers; // wlr_output_power_v1::link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal set_mode; // wlr_output_power_v1_set_mode_event
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_output_power_v1 {
	struct wl_resource *resource;
	struct wlr_output *output;
	struct wlr_output_power_manager_v1 *manager;
	struct wl_list link;

	struct wl_listener output_destroy_listener;
	struct wl_listener output_commit_listener;

	void *data;
};

struct wlr_output_power_v1_set_mode_event {
	struct wlr_output *output;
	enum zwlr_output_power_v1_mode mode;
};

struct wlr_output_power_manager_v1 *wlr_output_power_manager_v1_create(
	struct wl_display *display);

#endif
