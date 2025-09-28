/*
 * Copyright © 2014-2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef EVDEV_MT_TOUCHPAD_H
#define EVDEV_MT_TOUCHPAD_H

#include <stdbool.h>

#include "evdev.h"
#include "timer.h"

#define TOUCHPAD_HISTORY_LENGTH 4
#define TOUCHPAD_MIN_SAMPLES 4

/* Convert mm to a distance normalized to DEFAULT_MOUSE_DPI */
#define TP_MM_TO_DPI_NORMALIZED(mm) (DEFAULT_MOUSE_DPI/25.4 * mm)

enum touchpad_event {
	TOUCHPAD_EVENT_NONE		= 0,
	TOUCHPAD_EVENT_MOTION		= bit(0),
	TOUCHPAD_EVENT_BUTTON_PRESS	= bit(1),
	TOUCHPAD_EVENT_BUTTON_RELEASE	= bit(2),
	TOUCHPAD_EVENT_OTHERAXIS	= bit(3),
	TOUCHPAD_EVENT_TIMESTAMP	= bit(4),
};

enum touch_state {
	TOUCH_NONE = 0,
	TOUCH_HOVERING = 1,
	TOUCH_BEGIN = 2,
	TOUCH_UPDATE = 3,
	TOUCH_MAYBE_END = 4,
	TOUCH_END = 5,
};

static inline const char *
touch_state_to_str(enum touch_state state)
{
	switch(state) {
	CASE_RETURN_STRING(TOUCH_NONE);
	CASE_RETURN_STRING(TOUCH_HOVERING);
	CASE_RETURN_STRING(TOUCH_BEGIN);
	CASE_RETURN_STRING(TOUCH_UPDATE);
	CASE_RETURN_STRING(TOUCH_MAYBE_END);
	CASE_RETURN_STRING(TOUCH_END);
	}
	return NULL;
}

enum touch_palm_state {
	PALM_NONE = 0,
	PALM_EDGE,
	PALM_TYPING,
	PALM_TRACKPOINT,
	PALM_TOOL_PALM,
	PALM_PRESSURE,
	PALM_TOUCH_SIZE,
	PALM_ARBITRATION,
};

enum button_event {
	BUTTON_EVENT_IN_BOTTOM_R = 30,
	BUTTON_EVENT_IN_BOTTOM_M,
	BUTTON_EVENT_IN_BOTTOM_L,
	BUTTON_EVENT_IN_TOP_R,
	BUTTON_EVENT_IN_TOP_M,
	BUTTON_EVENT_IN_TOP_L,
	BUTTON_EVENT_IN_AREA,
	BUTTON_EVENT_UP,
	BUTTON_EVENT_PRESS,
	BUTTON_EVENT_RELEASE,
	BUTTON_EVENT_TIMEOUT,
};

enum button_state {
	BUTTON_STATE_NONE,
	BUTTON_STATE_AREA,
	BUTTON_STATE_BOTTOM,
	BUTTON_STATE_TOP,
	BUTTON_STATE_TOP_NEW,
	BUTTON_STATE_TOP_TO_IGNORE,
	BUTTON_STATE_IGNORE,
};

enum tp_tap_state {
	TAP_STATE_IDLE = 4,
	TAP_STATE_TOUCH,
	TAP_STATE_HOLD,
	TAP_STATE_1FGTAP_TAPPED,
	TAP_STATE_2FGTAP_TAPPED,
	TAP_STATE_3FGTAP_TAPPED,
	TAP_STATE_TOUCH_2,
	TAP_STATE_TOUCH_2_HOLD,
	TAP_STATE_TOUCH_2_RELEASE,
	TAP_STATE_TOUCH_3,
	TAP_STATE_TOUCH_3_HOLD,
	TAP_STATE_TOUCH_3_RELEASE,
	TAP_STATE_TOUCH_3_RELEASE_2,
	TAP_STATE_1FGTAP_DRAGGING_OR_DOUBLETAP,
	TAP_STATE_2FGTAP_DRAGGING_OR_DOUBLETAP,
	TAP_STATE_3FGTAP_DRAGGING_OR_DOUBLETAP,
	TAP_STATE_1FGTAP_DRAGGING_OR_TAP,
	TAP_STATE_2FGTAP_DRAGGING_OR_TAP,
	TAP_STATE_3FGTAP_DRAGGING_OR_TAP,
	TAP_STATE_1FGTAP_DRAGGING,
	TAP_STATE_2FGTAP_DRAGGING,
	TAP_STATE_3FGTAP_DRAGGING,
	TAP_STATE_1FGTAP_DRAGGING_WAIT,
	TAP_STATE_2FGTAP_DRAGGING_WAIT,
	TAP_STATE_3FGTAP_DRAGGING_WAIT,
	TAP_STATE_1FGTAP_DRAGGING_2,
	TAP_STATE_2FGTAP_DRAGGING_2,
	TAP_STATE_3FGTAP_DRAGGING_2,
	TAP_STATE_DEAD, /**< finger count exceeded */
};

enum tp_tap_touch_state {
	TAP_TOUCH_STATE_IDLE = 16,	/**< not in touch */
	TAP_TOUCH_STATE_TOUCH,		/**< touching, may tap */
	TAP_TOUCH_STATE_DEAD,		/**< exceeded motion/timeout */
};

/* For edge scrolling, so we only care about right and bottom */
enum tp_edge {
	EDGE_NONE	= 0,
	EDGE_RIGHT	= bit(0),
	EDGE_BOTTOM	= bit(1),
};

enum tp_edge_scroll_touch_state {
	EDGE_SCROLL_TOUCH_STATE_NONE,
	EDGE_SCROLL_TOUCH_STATE_EDGE_NEW,
	EDGE_SCROLL_TOUCH_STATE_EDGE,
	EDGE_SCROLL_TOUCH_STATE_AREA,
};

enum tp_gesture_state {
	GESTURE_STATE_NONE,
	GESTURE_STATE_UNKNOWN,
	GESTURE_STATE_HOLD,
	GESTURE_STATE_HOLD_AND_MOTION,
	GESTURE_STATE_POINTER_MOTION,
	GESTURE_STATE_SCROLL,
	GESTURE_STATE_PINCH,
	GESTURE_STATE_SWIPE,
};

enum tp_thumb_state {
	THUMB_STATE_FINGER,
	THUMB_STATE_JAILED,
	THUMB_STATE_PINCH,
	THUMB_STATE_SUPPRESSED,
	THUMB_STATE_REVIVED,
	THUMB_STATE_REVIVED_JAILED,
	THUMB_STATE_DEAD,
};

enum tp_jump_state {
	JUMP_STATE_IGNORE = 0,
	JUMP_STATE_EXPECT_FIRST,
	JUMP_STATE_EXPECT_DELAY,
};

struct tp_touch {
	struct tp_dispatch *tp;
	unsigned int index;
	enum touch_state state;
	bool has_ended;				/* TRACKING_ID == -1 */
	bool dirty;
	struct device_coords point;
	uint64_t initial_time;
	int pressure;
	bool is_tool_palm; /* MT_TOOL_PALM */
	int major, minor;

	bool was_down; /* if distance == 0, false for pure hovering
			  touches */

	struct {
		/* A quirk mostly used on Synaptics touchpads. In a
		   transition to/from fake touches > num_slots, the current
		   event data is likely garbage and the subsequent event
		   is likely too. This marker tells us to reset the motion
		   history again -> this effectively swallows any motion */
		bool reset_motion_history;
	} quirks;

	struct {
		struct tp_history_point {
			uint64_t time;
			struct device_coords point;
		} samples[TOUCHPAD_HISTORY_LENGTH];
		unsigned int index;
		unsigned int count;
	} history;

	struct {
		double last_delta_mm;
	} jumps;

	struct {
		struct device_coords center;
		uint8_t x_motion_history;
	} hysteresis;

	/* A pinned touchpoint is the one that pressed the physical button
	 * on a clickpad. After the release, it won't move until the center
	 * moves more than a threshold away from the original coordinates
	 */
	struct {
		bool is_pinned;
		struct device_coords center;
	} pinned;

	/* Software-button state and timeout if applicable */
	struct {
		enum button_state state;
		/* We use button_event here so we can use == on events */
		enum button_event current;
		struct libinput_timer timer;
		struct device_coords initial;
		bool has_moved; /* has moved more than threshold */
		uint64_t initial_time;
	} button;

	struct {
		enum tp_tap_touch_state state;
		struct device_coords initial;
		bool is_thumb;
		bool is_palm;
	} tap;

	struct {
		enum tp_edge_scroll_touch_state edge_state;
		uint32_t edge;
		int direction;
		struct libinput_timer timer;
		struct device_coords initial;
	} scroll;

	struct {
		enum touch_palm_state state;
		struct device_coords first; /* first coordinates if is_palm == true */
		uint64_t time; /* first timestamp if is_palm == true */
	} palm;

	struct {
		struct device_coords initial;
	} gesture;

	struct {
		double last_speed; /* speed in mm/s at last sample */
		unsigned int exceeded_count;
	} speed;
};

enum suspend_trigger {
	SUSPEND_NO_FLAG         = 0x0,
	SUSPEND_EXTERNAL_MOUSE  = 0x1,
	SUSPEND_SENDEVENTS      = 0x2,
	SUSPEND_LID             = 0x4,
	SUSPEND_TABLET_MODE     = 0x8,
};

struct tp_dispatch {
	struct evdev_dispatch base;
	struct evdev_device *device;
	unsigned int nfingers_down;		/* number of fingers down */
	unsigned int old_nfingers_down;		/* previous no fingers down */
	unsigned int slot;			/* current slot */
	bool has_mt;
	bool semi_mt;

	uint32_t suspend_reason;

	/* pen/touch arbitration */
	struct {
		enum evdev_arbitration_state state;
		struct libinput_timer arbitration_timer;
	} arbitration;

	unsigned int nactive_slots;		/* number of active slots */
	unsigned int num_slots;			/* number of slots */
	unsigned int ntouches;			/* no slots inc. fakes */
	struct tp_touch *touches;		/* len == ntouches */
	/* bit 0: BTN_TOUCH
	 * bit 1: BTN_TOOL_FINGER
	 * bit 2: BTN_TOOL_DOUBLETAP
	 * ...
	 */
	unsigned int fake_touches;

	struct {
		bool detection_disabled;
		struct ratelimit warning;
	} jump;

	/* if pressure goes above high -> touch down,
	   if pressure then goes below low -> touch up */
	struct {
		bool use_pressure;
		int high;
		int low;
	} pressure;

	/* If touch size (either axis) goes above high -> touch down,
	   if touch size (either axis) goes below low -> touch up */
	struct  {
		bool use_touch_size;
		int high;
		int low;

		/* convert device units to angle */
		double orientation_to_angle;
	} touch_size;

	struct {
		bool enabled;
		struct device_coords margin;
		unsigned int other_event_count;
		uint64_t last_motion_time;
	} hysteresis;

	struct {
		double x_scale_coeff;
		double y_scale_coeff;
		double xy_scale_coeff;
	} accel;

	struct {
		struct libinput_device_config_gesture config;
		bool enabled;
		bool started;
		unsigned int finger_count;
		unsigned int finger_count_pending;
		struct libinput_timer finger_count_switch_timer;
		enum tp_gesture_state state;
		struct tp_touch *touches[2];
		uint64_t initial_time;
		double initial_distance;
		double prev_scale;
		double angle;
		struct device_float_coords center;
		struct libinput_timer hold_timer;
		bool hold_enabled;
	} gesture;

	struct {
		bool is_clickpad;		/* true for clickpads */
		bool has_topbuttons;
		bool use_clickfinger;		/* number of fingers decides button number */
		bool click_pending;
		uint32_t state;
		uint32_t old_state;
		struct {
			double x_scale_coeff;
			double y_scale_coeff;
		} motion_dist;			/* for pinned touches */
		unsigned int active;		/* currently active button, for release event */
		bool active_is_topbutton;	/* is active a top button? */

		/* Only used for clickpads. The software button areas are
		 * always 2 horizontal stripes across the touchpad.
		 * The buttons are split according to the edge settings.
		 */
		struct {
			int32_t top_edge;	/* in device coordinates */
			int32_t rightbutton_left_edge; /* in device coordinates */
			int32_t middlebutton_left_edge; /* in device coordinates */
		} bottom_area;

		struct {
			int32_t bottom_edge;	/* in device coordinates */
			int32_t rightbutton_left_edge; /* in device coordinates */
			int32_t leftbutton_right_edge; /* in device coordinates */
		} top_area;

		struct evdev_device *trackpoint;

		enum libinput_config_click_method click_method;
		struct libinput_device_config_click_method config_method;
	} buttons;

	struct {
		struct libinput_device_config_scroll_method config_method;
		enum libinput_config_scroll_method method;
		int32_t right_edge;		/* in device coordinates */
		int32_t bottom_edge;		/* in device coordinates */
		struct {
			bool h, v;
		} active;
		struct phys_coords vector;
		uint64_t time_prev;
		struct {
			uint64_t h, v;
		} duration;
	} scroll;

	enum touchpad_event queued;

	struct {
		struct libinput_device_config_tap config;
		bool enabled;
		bool suspended;
		struct libinput_timer timer;
		enum tp_tap_state state;
		uint32_t buttons_pressed;
		uint64_t saved_press_time,
			 saved_release_time;

		enum libinput_config_tap_button_map map;
		enum libinput_config_tap_button_map want_map;

		bool drag_enabled;
		bool drag_lock_enabled;

		unsigned int nfingers_down;	/* number of fingers down for tapping (excl. thumb/palm) */
	} tap;

	struct {
		struct libinput_device_config_dwtp config;
		bool dwtp_enabled;

		int32_t right_edge;		/* in device coordinates */
		int32_t left_edge;		/* in device coordinates */
		int32_t upper_edge;		/* in device coordinates */

		bool trackpoint_active;
		struct libinput_event_listener trackpoint_listener;
		struct libinput_timer trackpoint_timer;
		uint64_t trackpoint_last_event_time;
		uint32_t trackpoint_event_count;
		bool monitor_trackpoint;

		bool use_mt_tool;

		bool use_pressure;
		int pressure_threshold;

		bool use_size;
		int size_threshold;
	} palm;

	struct {
		struct libinput_device_config_send_events config;
		enum libinput_config_send_events_mode current_mode;
	} sendevents;

	struct {
		struct libinput_device_config_dwt config;
		bool dwt_enabled;

		/* We have to allow for more than one device node to be the
		 * internal dwt keyboard (Razer Blade). But they're the same
		 * physical device, so we don't care about per-keyboard
		 * key/modifier masks.
		 */
		struct list paired_keyboard_list;

		unsigned long key_mask[NLONGS(KEY_CNT)];
		unsigned long mod_mask[NLONGS(KEY_CNT)];
		bool keyboard_active;
		struct libinput_timer keyboard_timer;
		uint64_t keyboard_last_press_time;
	} dwt;

	struct {
		bool detect_thumbs;
		int upper_thumb_line;
		int lower_thumb_line;

		bool use_pressure;
		int pressure_threshold;

		bool use_size;
		int size_threshold;

		enum tp_thumb_state state;
		unsigned int index;
		bool pinch_eligible;
	} thumb;

	struct {
		/* A quirk used on the T450 series Synaptics hardware.
		 * Slowly moving the finger causes multiple events with only
		 * ABS_MT_PRESSURE but no x/y information. When the x/y
		 * event comes, it will be a jump of ~20 units. We use the
		 * below to count non-motion events to discard that first
		 * event with the jump.
		 */
		unsigned int nonmotion_event_count;

		struct msc_timestamp {
			enum tp_jump_state state;
			uint32_t interval;
			uint32_t now;
		} msc_timestamp;
	} quirks;

	struct {
		struct libinput_event_listener listener;
		struct evdev_device *lid_switch;
	} lid_switch;

	struct {
		struct libinput_event_listener listener;
		struct evdev_device *tablet_mode_switch;
	} tablet_mode_switch;

	struct {
		bool rotate;
		bool want_rotate;

		bool must_rotate; /* true if we should rotate when applicable */
		struct evdev_device *tablet_device;
		bool tablet_left_handed_state;
	} left_handed;
};

static inline struct tp_dispatch*
tp_dispatch(struct evdev_dispatch *dispatch)
{
	evdev_verify_dispatch_type(dispatch, DISPATCH_TOUCHPAD);

	return container_of(dispatch, struct tp_dispatch, base);
}

#define tp_for_each_touch(_tp, _t) \
	for (unsigned int _i = 0; _i < (_tp)->ntouches && (_t = &(_tp)->touches[_i]); _i++)

static inline struct libinput*
tp_libinput_context(const struct tp_dispatch *tp)
{
	return evdev_libinput_context(tp->device);
}

static inline struct normalized_coords
tp_normalize_delta(const struct tp_dispatch *tp,
		   struct device_float_coords delta)
{
	struct normalized_coords normalized;

	normalized.x = delta.x * tp->accel.x_scale_coeff;
	normalized.y = delta.y * tp->accel.y_scale_coeff;

	return normalized;
}

static inline struct phys_coords
tp_phys_delta(const struct tp_dispatch *tp,
	      struct device_float_coords delta)
{
	struct phys_coords mm;

	mm.x = delta.x / tp->device->abs.absinfo_x->resolution;
	mm.y = delta.y / tp->device->abs.absinfo_y->resolution;

	return mm;
}

/**
 * Takes a set of device coordinates, returns that set of coordinates in the
 * x-axis' resolution.
 */
static inline struct device_float_coords
tp_scale_to_xaxis(const struct tp_dispatch *tp,
		  struct device_float_coords delta)
{
	struct device_float_coords raw;

	raw.x = delta.x;
	raw.y = delta.y * tp->accel.xy_scale_coeff;

	return raw;
}

struct device_coords
tp_get_delta(struct tp_touch *t);

struct normalized_coords
tp_filter_motion(struct tp_dispatch *tp,
		 const struct device_float_coords *unaccelerated,
		 uint64_t time);

struct normalized_coords
tp_filter_motion_unaccelerated(struct tp_dispatch *tp,
			       const struct device_float_coords *unaccelerated,
			       uint64_t time);

struct normalized_coords
tp_filter_scroll(struct tp_dispatch *tp,
		 const struct device_float_coords *unaccelerated,
		 uint64_t time);

bool
tp_touch_active(const struct tp_dispatch *tp, const struct tp_touch *t);

bool
tp_touch_active_for_gesture(const struct tp_dispatch *tp,
			    const struct tp_touch *t);

int
tp_tap_handle_state(struct tp_dispatch *tp, uint64_t time);

void
tp_tap_post_process_state(struct tp_dispatch *tp);

void
tp_init_tap(struct tp_dispatch *tp);

void
tp_remove_tap(struct tp_dispatch *tp);

void
tp_init_buttons(struct tp_dispatch *tp, struct evdev_device *device);

void
tp_init_top_softbuttons(struct tp_dispatch *tp,
			struct evdev_device *device,
			double topbutton_size_mult);

void
tp_remove_buttons(struct tp_dispatch *tp);

void
tp_process_button(struct tp_dispatch *tp,
		  const struct input_event *e,
		  uint64_t time);

void
tp_release_all_buttons(struct tp_dispatch *tp,
		       uint64_t time);

int
tp_post_button_events(struct tp_dispatch *tp, uint64_t time);

void
tp_button_handle_state(struct tp_dispatch *tp, uint64_t time);

bool
tp_button_touch_active(const struct tp_dispatch *tp,
		       const struct tp_touch *t);

bool
tp_button_is_inside_softbutton_area(const struct tp_dispatch *tp,
				    const struct tp_touch *t);

void
tp_release_all_taps(struct tp_dispatch *tp,
		    uint64_t now);

void
tp_tap_suspend(struct tp_dispatch *tp, uint64_t time);

void
tp_tap_resume(struct tp_dispatch *tp, uint64_t time);

bool
tp_tap_dragging(const struct tp_dispatch *tp);

bool
tp_tap_dragging_or_double_tapping(const struct tp_dispatch *tp);

void
tp_edge_scroll_init(struct tp_dispatch *tp, struct evdev_device *device);

void
tp_remove_edge_scroll(struct tp_dispatch *tp);

void
tp_edge_scroll_handle_state(struct tp_dispatch *tp, uint64_t time);

int
tp_edge_scroll_post_events(struct tp_dispatch *tp, uint64_t time);

void
tp_edge_scroll_stop_events(struct tp_dispatch *tp, uint64_t time);

int
tp_edge_scroll_touch_active(const struct tp_dispatch *tp,
			    const struct tp_touch *t);

uint32_t
tp_touch_get_edge(const struct tp_dispatch *tp, const struct tp_touch *t);

void
tp_init_gesture(struct tp_dispatch *tp);

void
tp_remove_gesture(struct tp_dispatch *tp);

void
tp_gesture_stop(struct tp_dispatch *tp, uint64_t time);

void
tp_gesture_cancel(struct tp_dispatch *tp, uint64_t time);

void
tp_gesture_cancel_motion_gestures(struct tp_dispatch *tp, uint64_t time);

void
tp_gesture_handle_state(struct tp_dispatch *tp, uint64_t time);

void
tp_gesture_post_events(struct tp_dispatch *tp, uint64_t time,
		       bool ignore_motion);

void
tp_gesture_stop_twofinger_scroll(struct tp_dispatch *tp, uint64_t time);

void
tp_gesture_tap_timeout(struct tp_dispatch *tp, uint64_t time);

void
tp_clickpad_middlebutton_apply_config(struct evdev_device *device);

bool
tp_thumb_ignored(const struct tp_dispatch *tp, const struct tp_touch *t);

void
tp_thumb_reset(struct tp_dispatch *tp);

bool
tp_thumb_ignored_for_gesture(const struct tp_dispatch *tp, const struct tp_touch *t);

bool
tp_thumb_ignored_for_tap(const struct tp_dispatch *tp,
			 const struct tp_touch *t);

void
tp_thumb_suppress(struct tp_dispatch *tp, struct tp_touch *t);

void
tp_thumb_update_touch(struct tp_dispatch *tp,
		      struct tp_touch *t,
		      uint64_t time);

void
tp_detect_thumb_while_moving(struct tp_dispatch *tp);

void
tp_thumb_update_multifinger(struct tp_dispatch *tp);

void
tp_init_thumb(struct tp_dispatch *tp);

struct tp_touch*
tp_thumb_get_touch(struct tp_dispatch *tp);

#endif
