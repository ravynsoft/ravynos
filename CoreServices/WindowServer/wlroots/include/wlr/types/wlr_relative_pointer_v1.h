/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_RELATIVE_POINTER_V1_H
#define WLR_TYPES_WLR_RELATIVE_POINTER_V1_H

#include <wayland-server-core.h>

/**
 * This protocol specifies a set of interfaces used for making clients able to
 * receive relative pointer events not obstructed by barriers (such as the
 * monitor edge or pointer constraints).
 */

/**
 * A global interface used for getting the relative pointer object for a given
 * pointer.
 */
struct wlr_relative_pointer_manager_v1 {
	struct wl_global *global;
	struct wl_list relative_pointers; // wlr_relative_pointer_v1::link

	struct {
		struct wl_signal destroy;
		struct wl_signal new_relative_pointer; // wlr_relative_pointer_v1
	} events;

	struct wl_listener display_destroy_listener;

	void *data;
};

/**
 * A wp_relative_pointer object is an extension to the wl_pointer interface
 * used for emitting relative pointer events. It shares the same focus as
 * wl_pointer objects of the same seat and will only emit events when it has
 * focus.
 */
struct wlr_relative_pointer_v1 {
	struct wl_resource *resource;
	struct wl_resource *pointer_resource;
	struct wlr_seat *seat;
	struct wl_list link; // wlr_relative_pointer_manager_v1::relative_pointers

	struct {
		struct wl_signal destroy;
	} events;

	struct wl_listener seat_destroy;
	struct wl_listener pointer_destroy;

	void *data;
};

struct wlr_relative_pointer_manager_v1 *wlr_relative_pointer_manager_v1_create(
	struct wl_display *display);

/**
 * Send a relative motion event to the seat. Time is given in microseconds
 * (unlike wl_pointer which uses milliseconds).
 */
void wlr_relative_pointer_manager_v1_send_relative_motion(
	struct wlr_relative_pointer_manager_v1 *manager, struct wlr_seat *seat,
	uint64_t time_usec, double dx, double dy,
	double dx_unaccel, double dy_unaccel);

/**
 * Get a relative pointer from its resource. Returns NULL if inert.
 */
struct wlr_relative_pointer_v1 *wlr_relative_pointer_v1_from_resource(
	struct wl_resource *resource);

#endif
