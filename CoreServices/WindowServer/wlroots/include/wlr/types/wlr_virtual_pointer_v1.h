/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_VIRTUAL_POINTER_V1_H
#define WLR_TYPES_WLR_VIRTUAL_POINTER_V1_H

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_output.h>

struct wlr_virtual_pointer_manager_v1 {
	struct wl_global *global;
	struct wl_list virtual_pointers; // struct wlr_virtual_pointer_v1*

	struct wl_listener display_destroy;

	struct {
		struct wl_signal new_virtual_pointer; // struct wlr_virtual_pointer_v1_new_pointer_event*
		struct wl_signal destroy;
	} events;
};

struct wlr_virtual_pointer_v1 {
	struct wlr_input_device input_device;
	struct wl_resource *resource;
	/* Vertical and horizontal */
	struct wlr_event_pointer_axis axis_event[2];
	enum wl_pointer_axis axis;
	bool axis_valid[2];

	struct wl_list link;

	struct {
		struct wl_signal destroy; // struct wlr_virtual_pointer_v1*
	} events;
};

struct wlr_virtual_pointer_v1_new_pointer_event {
	struct wlr_virtual_pointer_v1 *new_pointer;
	/** Suggested by client; may be NULL. */
	struct wlr_seat *suggested_seat;
	struct wlr_output *suggested_output;
};

struct wlr_virtual_pointer_manager_v1* wlr_virtual_pointer_manager_v1_create(
	struct wl_display *display);

#endif
