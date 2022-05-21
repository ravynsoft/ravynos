/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_SWITCH_H
#define WLR_TYPES_WLR_SWITCH_H

#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>

struct wlr_switch_impl;

struct wlr_switch {
	struct wlr_switch_impl *impl;

	struct {
		struct wl_signal toggle;
	} events;

	void *data;
};

enum wlr_switch_type {
	WLR_SWITCH_TYPE_LID = 1,
	WLR_SWITCH_TYPE_TABLET_MODE,
};

enum wlr_switch_state {
	WLR_SWITCH_STATE_OFF = 0,
	WLR_SWITCH_STATE_ON,
	WLR_SWITCH_STATE_TOGGLE
};

struct wlr_event_switch_toggle {
	struct wlr_input_device *device;
	uint32_t time_msec;
	enum wlr_switch_type switch_type;
	enum wlr_switch_state switch_state;
};

#endif
