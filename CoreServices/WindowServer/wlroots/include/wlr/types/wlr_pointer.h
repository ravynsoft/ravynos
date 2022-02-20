/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_POINTER_H
#define WLR_TYPES_WLR_POINTER_H

#include <stdint.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_input_device.h>

struct wlr_pointer_impl;

struct wlr_pointer {
	const struct wlr_pointer_impl *impl;

	struct {
		struct wl_signal motion; // struct wlr_event_pointer_motion
		struct wl_signal motion_absolute; // struct wlr_event_pointer_motion_absolute
		struct wl_signal button; // struct wlr_event_pointer_button
		struct wl_signal axis; // struct wlr_event_pointer_axis
		struct wl_signal frame;

		struct wl_signal swipe_begin; // struct wlr_event_pointer_swipe_begin
		struct wl_signal swipe_update; // struct wlr_event_pointer_swipe_update
		struct wl_signal swipe_end; // struct wlr_event_pointer_swipe_end

		struct wl_signal pinch_begin; // struct wlr_event_pointer_pinch_begin
		struct wl_signal pinch_update; // struct wlr_event_pointer_pinch_update
		struct wl_signal pinch_end; // struct wlr_event_pointer_pinch_end

		struct wl_signal hold_begin; // struct wlr_event_pointer_hold_begin
		struct wl_signal hold_end; // struct wlr_event_pointer_hold_end
	} events;

	void *data;
};

struct wlr_event_pointer_motion {
	struct wlr_input_device *device;
	uint32_t time_msec;
	double delta_x, delta_y;
	double unaccel_dx, unaccel_dy;
};

struct wlr_event_pointer_motion_absolute {
	struct wlr_input_device *device;
	uint32_t time_msec;
	// From 0..1
	double x, y;
};

struct wlr_event_pointer_button {
	struct wlr_input_device *device;
	uint32_t time_msec;
	uint32_t button;
	enum wlr_button_state state;
};

enum wlr_axis_source {
	WLR_AXIS_SOURCE_WHEEL,
	WLR_AXIS_SOURCE_FINGER,
	WLR_AXIS_SOURCE_CONTINUOUS,
	WLR_AXIS_SOURCE_WHEEL_TILT,
};

enum wlr_axis_orientation {
	WLR_AXIS_ORIENTATION_VERTICAL,
	WLR_AXIS_ORIENTATION_HORIZONTAL,
};

struct wlr_event_pointer_axis {
	struct wlr_input_device *device;
	uint32_t time_msec;
	enum wlr_axis_source source;
	enum wlr_axis_orientation orientation;
	double delta;
	int32_t delta_discrete;
};

struct wlr_event_pointer_swipe_begin {
	struct wlr_input_device *device;
	uint32_t time_msec;
	uint32_t fingers;
};

struct wlr_event_pointer_swipe_update {
	struct wlr_input_device *device;
	uint32_t time_msec;
	uint32_t fingers;
	// Relative coordinates of the logical center of the gesture
	// compared to the previous event.
	double dx, dy;
};

struct wlr_event_pointer_swipe_end {
	struct wlr_input_device *device;
	uint32_t time_msec;
	bool cancelled;
};

struct wlr_event_pointer_pinch_begin {
	struct wlr_input_device *device;
	uint32_t time_msec;
	uint32_t fingers;
};

struct wlr_event_pointer_pinch_update {
	struct wlr_input_device *device;
	uint32_t time_msec;
	uint32_t fingers;
	// Relative coordinates of the logical center of the gesture
	// compared to the previous event.
	double dx, dy;
	// Absolute scale compared to the begin event
	double scale;
	// Relative angle in degrees clockwise compared to the previous event.
	double rotation;
};

struct wlr_event_pointer_pinch_end {
	struct wlr_input_device *device;
	uint32_t time_msec;
	bool cancelled;
};

struct wlr_event_pointer_hold_begin {
	struct wlr_input_device *device;
	uint32_t time_msec;
	uint32_t fingers;
};

struct wlr_event_pointer_hold_end {
	struct wlr_input_device *device;
	uint32_t time_msec;
	bool cancelled;
};

#endif
