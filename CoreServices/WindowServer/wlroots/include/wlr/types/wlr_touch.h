/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_TOUCH_H
#define WLR_TYPES_WLR_TOUCH_H

#include <stdint.h>
#include <wlr/types/wlr_input_device.h>
#include <wayland-server-core.h>

struct wlr_touch_impl;

struct wlr_touch {
	struct wlr_input_device base;

	const struct wlr_touch_impl *impl;

	struct {
		struct wl_signal down; // struct wlr_event_touch_down
		struct wl_signal up; // struct wlr_event_touch_up
		struct wl_signal motion; // struct wlr_event_touch_motion
		struct wl_signal cancel; // struct wlr_event_touch_cancel
		struct wl_signal frame;
	} events;

	void *data;
};

struct wlr_event_touch_down {
	struct wlr_input_device *device;
	uint32_t time_msec;
	int32_t touch_id;
	// From 0..1
	double x, y;
};

struct wlr_event_touch_up {
	struct wlr_input_device *device;
	uint32_t time_msec;
	int32_t touch_id;
};

struct wlr_event_touch_motion {
	struct wlr_input_device *device;
	uint32_t time_msec;
	int32_t touch_id;
	// From 0..1
	double x, y;
};

struct wlr_event_touch_cancel {
	struct wlr_input_device *device;
	uint32_t time_msec;
	int32_t touch_id;
};

#endif
