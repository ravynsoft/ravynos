/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_DATA_CONTROL_V1_H
#define WLR_TYPES_WLR_DATA_CONTROL_V1_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

struct wlr_data_control_manager_v1 {
	struct wl_global *global;
	struct wl_list devices; // wlr_data_control_device_v1::link

	struct {
		struct wl_signal destroy;
		struct wl_signal new_device; // wlr_data_control_device_v1
	} events;

	struct wl_listener display_destroy;
};

struct wlr_data_control_device_v1 {
	struct wl_resource *resource;
	struct wlr_data_control_manager_v1 *manager;
	struct wl_list link; // wlr_data_control_manager_v1::devices

	struct wlr_seat *seat;
	struct wl_resource *selection_offer_resource; // current selection offer
	struct wl_resource *primary_selection_offer_resource; // current primary selection offer

	struct wl_listener seat_destroy;
	struct wl_listener seat_set_selection;
	struct wl_listener seat_set_primary_selection;
};

struct wlr_data_control_manager_v1 *wlr_data_control_manager_v1_create(
	struct wl_display *display);

void wlr_data_control_device_v1_destroy(
	struct wlr_data_control_device_v1 *device);

#endif
