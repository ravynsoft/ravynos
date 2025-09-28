/*
 * Copyright © 2013 Jonas Ådahl
 * Copyright © 2013-2018 Red Hat, Inc.
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

#include "config.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>

#include "libinput.h"
#include "libinput-private.h"
#include "evdev.h"
#include "timer.h"
#include "quirks.h"

#define require_event_type(li_, type_, retval_, ...)	\
	if (type_ == LIBINPUT_EVENT_NONE) abort(); \
	if (!check_event_type(li_, __func__, type_, __VA_ARGS__, -1)) \
		return retval_; \

#define ASSERT_INT_SIZE(type_) \
	static_assert(sizeof(type_) == sizeof(unsigned int), \
		      "sizeof("  #type_ ") must be sizeof(uint)")

ASSERT_INT_SIZE(enum libinput_log_priority);
ASSERT_INT_SIZE(enum libinput_device_capability);
ASSERT_INT_SIZE(enum libinput_key_state);
ASSERT_INT_SIZE(enum libinput_led);
ASSERT_INT_SIZE(enum libinput_button_state);
ASSERT_INT_SIZE(enum libinput_pointer_axis);
ASSERT_INT_SIZE(enum libinput_pointer_axis_source);
ASSERT_INT_SIZE(enum libinput_tablet_pad_ring_axis_source);
ASSERT_INT_SIZE(enum libinput_tablet_pad_strip_axis_source);
ASSERT_INT_SIZE(enum libinput_tablet_tool_type);
ASSERT_INT_SIZE(enum libinput_tablet_tool_proximity_state);
ASSERT_INT_SIZE(enum libinput_tablet_tool_tip_state);
ASSERT_INT_SIZE(enum libinput_switch_state);
ASSERT_INT_SIZE(enum libinput_switch);
ASSERT_INT_SIZE(enum libinput_event_type);
ASSERT_INT_SIZE(enum libinput_config_status);
ASSERT_INT_SIZE(enum libinput_config_tap_state);
ASSERT_INT_SIZE(enum libinput_config_tap_button_map);
ASSERT_INT_SIZE(enum libinput_config_drag_state);
ASSERT_INT_SIZE(enum libinput_config_drag_lock_state);
ASSERT_INT_SIZE(enum libinput_config_send_events_mode);
ASSERT_INT_SIZE(enum libinput_config_accel_profile);
ASSERT_INT_SIZE(enum libinput_config_click_method);
ASSERT_INT_SIZE(enum libinput_config_middle_emulation_state);
ASSERT_INT_SIZE(enum libinput_config_scroll_method);
ASSERT_INT_SIZE(enum libinput_config_dwt_state);
ASSERT_INT_SIZE(enum libinput_config_dwtp_state);

static inline const char *
event_type_to_str(enum libinput_event_type type)
{
	switch(type) {
	CASE_RETURN_STRING(LIBINPUT_EVENT_DEVICE_ADDED);
	CASE_RETURN_STRING(LIBINPUT_EVENT_DEVICE_REMOVED);
	CASE_RETURN_STRING(LIBINPUT_EVENT_KEYBOARD_KEY);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_MOTION);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_BUTTON);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_AXIS);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_SCROLL_WHEEL);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_SCROLL_FINGER);
	CASE_RETURN_STRING(LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TOUCH_DOWN);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TOUCH_UP);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TOUCH_MOTION);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TOUCH_CANCEL);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TOUCH_FRAME);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_TOOL_AXIS);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_TOOL_TIP);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_TOOL_BUTTON);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_PAD_BUTTON);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_PAD_RING);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_PAD_STRIP);
	CASE_RETURN_STRING(LIBINPUT_EVENT_TABLET_PAD_KEY);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_SWIPE_END);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_PINCH_BEGIN);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_PINCH_UPDATE);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_PINCH_END);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_HOLD_BEGIN);
	CASE_RETURN_STRING(LIBINPUT_EVENT_GESTURE_HOLD_END);
	CASE_RETURN_STRING(LIBINPUT_EVENT_SWITCH_TOGGLE);
	case LIBINPUT_EVENT_NONE:
		abort();
	}

	return NULL;
}

static inline bool
check_event_type(struct libinput *libinput,
		 const char *function_name,
		 unsigned int type_in,
		 ...)
{
	bool rc = false;
	va_list args;
	unsigned int type_permitted;

	va_start(args, type_in);
	type_permitted = va_arg(args, unsigned int);

	while (type_permitted != (unsigned int)-1) {
		if (type_permitted == type_in) {
			rc = true;
			break;
		}
		type_permitted = va_arg(args, unsigned int);
	}

	va_end(args);

	if (!rc) {
		const char *name = event_type_to_str(type_in);
		log_bug_client(libinput,
			       "Invalid event type %s (%d) passed to %s()\n",
			       name, type_in, function_name);
	}

	return rc;
}

struct libinput_source {
	libinput_source_dispatch_t dispatch;
	void *user_data;
	int fd;
	struct list link;
};

struct libinput_event_device_notify {
	struct libinput_event base;
};

struct libinput_event_keyboard {
	struct libinput_event base;
	uint64_t time;
	uint32_t key;
	uint32_t seat_key_count;
	enum libinput_key_state state;
};

struct libinput_event_pointer {
	struct libinput_event base;
	uint64_t time;
	struct normalized_coords delta;
	struct device_float_coords delta_raw;
	struct device_coords absolute;
	struct discrete_coords discrete;
	struct wheel_v120 v120;
	uint32_t button;
	uint32_t seat_button_count;
	enum libinput_button_state state;
	enum libinput_pointer_axis_source source;
	uint32_t axes;
};

struct libinput_event_touch {
	struct libinput_event base;
	uint64_t time;
	int32_t slot;
	int32_t seat_slot;
	struct device_coords point;
};

struct libinput_event_gesture {
	struct libinput_event base;
	uint64_t time;
	int finger_count;
	int cancelled;
	struct normalized_coords delta;
	struct normalized_coords delta_unaccel;
	double scale;
	double angle;
};

struct libinput_event_tablet_tool {
	struct libinput_event base;
	uint32_t button;
	enum libinput_button_state state;
	uint32_t seat_button_count;
	uint64_t time;
	struct tablet_axes axes;
	unsigned char changed_axes[NCHARS(LIBINPUT_TABLET_TOOL_AXIS_MAX + 1)];
	struct libinput_tablet_tool *tool;
	enum libinput_tablet_tool_proximity_state proximity_state;
	enum libinput_tablet_tool_tip_state tip_state;
};

struct libinput_event_tablet_pad {
	struct libinput_event base;
	unsigned int mode;
	struct libinput_tablet_pad_mode_group *mode_group;
	uint64_t time;
	struct {
		uint32_t number;
		enum libinput_button_state state;
	} button;
	struct {
		uint32_t code;
		enum libinput_key_state state;
	} key;
	struct {
		enum libinput_tablet_pad_ring_axis_source source;
		double position;
		int number;
	} ring;
	struct {
		enum libinput_tablet_pad_strip_axis_source source;
		double position;
		int number;
	} strip;
};

struct libinput_event_switch {
	struct libinput_event base;
	uint64_t time;
	enum libinput_switch sw;
	enum libinput_switch_state state;
};

LIBINPUT_ATTRIBUTE_PRINTF(3, 0)
static void
libinput_default_log_func(struct libinput *libinput,
			  enum libinput_log_priority priority,
			  const char *format, va_list args)
{
	const char *prefix;

	switch(priority) {
	case LIBINPUT_LOG_PRIORITY_DEBUG: prefix = "debug"; break;
	case LIBINPUT_LOG_PRIORITY_INFO: prefix = "info"; break;
	case LIBINPUT_LOG_PRIORITY_ERROR: prefix = "error"; break;
	default: prefix="<invalid priority>"; break;
	}

	fprintf(stderr, "libinput %s: ", prefix);
	vfprintf(stderr, format, args);
}

void
log_msg_va(struct libinput *libinput,
	   enum libinput_log_priority priority,
	   const char *format,
	   va_list args)
{
	if (is_logged(libinput, priority))
		libinput->log_handler(libinput, priority, format, args);
}

void
log_msg(struct libinput *libinput,
	enum libinput_log_priority priority,
	const char *format, ...)
{
	va_list args;

	va_start(args, format);
	log_msg_va(libinput, priority, format, args);
	va_end(args);
}

void
log_msg_ratelimit(struct libinput *libinput,
		  struct ratelimit *ratelimit,
		  enum libinput_log_priority priority,
		  const char *format, ...)
{
	va_list args;
	enum ratelimit_state state;

	state = ratelimit_test(ratelimit);
	if (state == RATELIMIT_EXCEEDED)
		return;

	va_start(args, format);
	log_msg_va(libinput, priority, format, args);
	va_end(args);

	if (state == RATELIMIT_THRESHOLD)
		log_msg(libinput,
			priority,
			"WARNING: log rate limit exceeded (%d msgs per %dms). Discarding future messages.\n",
			ratelimit->burst,
			us2ms(ratelimit->interval));
}

LIBINPUT_EXPORT void
libinput_log_set_priority(struct libinput *libinput,
			  enum libinput_log_priority priority)
{
	libinput->log_priority = priority;
}

LIBINPUT_EXPORT enum libinput_log_priority
libinput_log_get_priority(const struct libinput *libinput)
{
	return libinput->log_priority;
}

LIBINPUT_EXPORT void
libinput_log_set_handler(struct libinput *libinput,
			 libinput_log_handler log_handler)
{
	libinput->log_handler = log_handler;
}

static void
libinput_device_group_destroy(struct libinput_device_group *group);

static void
libinput_post_event(struct libinput *libinput,
		    struct libinput_event *event);

LIBINPUT_EXPORT enum libinput_event_type
libinput_event_get_type(struct libinput_event *event)
{
	return event->type;
}

LIBINPUT_EXPORT struct libinput *
libinput_event_get_context(struct libinput_event *event)
{
	return event->device->seat->libinput;
}

LIBINPUT_EXPORT struct libinput_device *
libinput_event_get_device(struct libinput_event *event)
{
	return event->device;
}

LIBINPUT_EXPORT struct libinput_event_pointer *
libinput_event_get_pointer_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_POINTER_MOTION,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE,
			   LIBINPUT_EVENT_POINTER_BUTTON,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			   LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			   LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			   LIBINPUT_EVENT_POINTER_AXIS);

	return (struct libinput_event_pointer *) event;
}

LIBINPUT_EXPORT struct libinput_event_keyboard *
libinput_event_get_keyboard_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return (struct libinput_event_keyboard *) event;
}

LIBINPUT_EXPORT struct libinput_event_touch *
libinput_event_get_touch_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_UP,
			   LIBINPUT_EVENT_TOUCH_MOTION,
			   LIBINPUT_EVENT_TOUCH_CANCEL,
			   LIBINPUT_EVENT_TOUCH_FRAME);
	return (struct libinput_event_touch *) event;
}

LIBINPUT_EXPORT struct libinput_event_gesture *
libinput_event_get_gesture_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
			   LIBINPUT_EVENT_GESTURE_HOLD_END);

	return (struct libinput_event_gesture *) event;
}

LIBINPUT_EXPORT struct libinput_event_tablet_tool *
libinput_event_get_tablet_tool_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON);

	return (struct libinput_event_tablet_tool *) event;
}

LIBINPUT_EXPORT struct libinput_event_tablet_pad *
libinput_event_get_tablet_pad_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_TABLET_PAD_RING,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON,
			   LIBINPUT_EVENT_TABLET_PAD_KEY);

	return (struct libinput_event_tablet_pad *) event;
}

LIBINPUT_EXPORT struct libinput_event_device_notify *
libinput_event_get_device_notify_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_DEVICE_ADDED,
			   LIBINPUT_EVENT_DEVICE_REMOVED);

	return (struct libinput_event_device_notify *) event;
}

LIBINPUT_EXPORT struct libinput_event_switch *
libinput_event_get_switch_event(struct libinput_event *event)
{
	require_event_type(libinput_event_get_context(event),
			   event->type,
			   NULL,
			   LIBINPUT_EVENT_SWITCH_TOGGLE);

	return (struct libinput_event_switch *) event;
}

LIBINPUT_EXPORT uint32_t
libinput_event_keyboard_get_time(struct libinput_event_keyboard *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_keyboard_get_time_usec(struct libinput_event_keyboard *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return event->time;
}

LIBINPUT_EXPORT uint32_t
libinput_event_keyboard_get_key(struct libinput_event_keyboard *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return event->key;
}

LIBINPUT_EXPORT enum libinput_key_state
libinput_event_keyboard_get_key_state(struct libinput_event_keyboard *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return event->state;
}

LIBINPUT_EXPORT uint32_t
libinput_event_keyboard_get_seat_key_count(
	struct libinput_event_keyboard *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return event->seat_key_count;
}

LIBINPUT_EXPORT uint32_t
libinput_event_pointer_get_time(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE,
			   LIBINPUT_EVENT_POINTER_BUTTON,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			   LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			   LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			   LIBINPUT_EVENT_POINTER_AXIS);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_pointer_get_time_usec(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE,
			   LIBINPUT_EVENT_POINTER_BUTTON,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			   LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			   LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			   LIBINPUT_EVENT_POINTER_AXIS);

	return event->time;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_dx(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION);

	return event->delta.x;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_dy(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION);

	return event->delta.y;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_dx_unaccelerated(
	struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION);

	return event->delta_raw.x;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_dy_unaccelerated(
	struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION);

	return event->delta_raw.y;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_absolute_x(struct libinput_event_pointer *event)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);

	return evdev_convert_to_mm(device->abs.absinfo_x, event->absolute.x);
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_absolute_y(struct libinput_event_pointer *event)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);

	return evdev_convert_to_mm(device->abs.absinfo_y, event->absolute.y);
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_absolute_x_transformed(
	struct libinput_event_pointer *event,
	uint32_t width)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);

	return evdev_device_transform_x(device, event->absolute.x, width);
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_absolute_y_transformed(
	struct libinput_event_pointer *event,
	uint32_t height)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);

	return evdev_device_transform_y(device, event->absolute.y, height);
}

LIBINPUT_EXPORT uint32_t
libinput_event_pointer_get_button(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_BUTTON);

	return event->button;
}

LIBINPUT_EXPORT enum libinput_button_state
libinput_event_pointer_get_button_state(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_BUTTON);

	return event->state;
}

LIBINPUT_EXPORT uint32_t
libinput_event_pointer_get_seat_button_count(
	struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_BUTTON);

	return event->seat_button_count;
}

LIBINPUT_EXPORT int
libinput_event_pointer_has_axis(struct libinput_event_pointer *event,
				enum libinput_pointer_axis axis)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			   LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			   LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			   LIBINPUT_EVENT_POINTER_AXIS);

	switch (axis) {
	case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
	case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
		return !!(event->axes & bit(axis));
	}

	return 0;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_axis_value(struct libinput_event_pointer *event,
				      enum libinput_pointer_axis axis)
{
	struct libinput *libinput = event->base.device->seat->libinput;
	double value = 0;

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_POINTER_AXIS);

	if (!libinput_event_pointer_has_axis(event, axis)) {
		log_bug_client(libinput, "value requested for unset axis\n");
	} else {
		switch (axis) {
		case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
			value = event->delta.x;
			break;
		case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
			value = event->delta.y;
			break;
		}
	}

	return value;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_axis_value_discrete(struct libinput_event_pointer *event,
					       enum libinput_pointer_axis axis)
{
	struct libinput *libinput = event->base.device->seat->libinput;
	double value = 0;

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_POINTER_AXIS);

	if (!libinput_event_pointer_has_axis(event, axis)) {
		log_bug_client(libinput, "value requested for unset axis\n");
	} else {
		switch (axis) {
		case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
			value = event->discrete.x;
			break;
		case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
			value = event->discrete.y;
			break;
		}
	}
	return value;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_scroll_value(struct libinput_event_pointer *event,
					enum libinput_pointer_axis axis)
{
	struct libinput *libinput = event->base.device->seat->libinput;
	double value = 0;

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			   LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			   LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS);

	if (!libinput_event_pointer_has_axis(event, axis)) {
		log_bug_client(libinput, "value requested for unset axis\n");
	} else {
		switch (axis) {
		case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
			value = event->delta.x;
			break;
		case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
			value = event->delta.y;
			break;
		}
	}
	return value;
}

LIBINPUT_EXPORT double
libinput_event_pointer_get_scroll_value_v120(struct libinput_event_pointer *event,
					     enum libinput_pointer_axis axis)
{
	struct libinput *libinput = event->base.device->seat->libinput;
	double value = 0;

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL);

	if (!libinput_event_pointer_has_axis(event, axis)) {
		log_bug_client(libinput, "value requested for unset axis\n");
	} else {
		switch (axis) {
		case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
			value = event->v120.x;
			break;
		case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
			value = event->v120.y;
			break;
		}
	}
	return value;
}

LIBINPUT_EXPORT enum libinput_pointer_axis_source
libinput_event_pointer_get_axis_source(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_POINTER_AXIS);

	return event->source;
}

LIBINPUT_EXPORT uint32_t
libinput_event_touch_get_time(struct libinput_event_touch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_UP,
			   LIBINPUT_EVENT_TOUCH_MOTION,
			   LIBINPUT_EVENT_TOUCH_CANCEL,
			   LIBINPUT_EVENT_TOUCH_FRAME);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_touch_get_time_usec(struct libinput_event_touch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_UP,
			   LIBINPUT_EVENT_TOUCH_MOTION,
			   LIBINPUT_EVENT_TOUCH_CANCEL,
			   LIBINPUT_EVENT_TOUCH_FRAME);

	return event->time;
}

LIBINPUT_EXPORT int32_t
libinput_event_touch_get_slot(struct libinput_event_touch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_UP,
			   LIBINPUT_EVENT_TOUCH_MOTION,
			   LIBINPUT_EVENT_TOUCH_CANCEL);

	return event->slot;
}

LIBINPUT_EXPORT int32_t
libinput_event_touch_get_seat_slot(struct libinput_event_touch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_UP,
			   LIBINPUT_EVENT_TOUCH_MOTION,
			   LIBINPUT_EVENT_TOUCH_CANCEL);

	return event->seat_slot;
}

LIBINPUT_EXPORT double
libinput_event_touch_get_x(struct libinput_event_touch *event)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_MOTION);

	return evdev_convert_to_mm(device->abs.absinfo_x, event->point.x);
}

LIBINPUT_EXPORT double
libinput_event_touch_get_x_transformed(struct libinput_event_touch *event,
				       uint32_t width)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_MOTION);

	return evdev_device_transform_x(device, event->point.x, width);
}

LIBINPUT_EXPORT double
libinput_event_touch_get_y_transformed(struct libinput_event_touch *event,
				       uint32_t height)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_MOTION);

	return evdev_device_transform_y(device, event->point.y, height);
}

LIBINPUT_EXPORT double
libinput_event_touch_get_y(struct libinput_event_touch *event)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_MOTION);

	return evdev_convert_to_mm(device->abs.absinfo_y, event->point.y);
}

LIBINPUT_EXPORT uint32_t
libinput_event_gesture_get_time(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END,
			   LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
			   LIBINPUT_EVENT_GESTURE_HOLD_END);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_gesture_get_time_usec(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END,
			   LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
			   LIBINPUT_EVENT_GESTURE_HOLD_END);

	return event->time;
}

LIBINPUT_EXPORT int
libinput_event_gesture_get_finger_count(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END,
			   LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
			   LIBINPUT_EVENT_GESTURE_HOLD_END);

	return event->finger_count;
}

LIBINPUT_EXPORT int
libinput_event_gesture_get_cancelled(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END,
			   LIBINPUT_EVENT_GESTURE_HOLD_END);

	return event->cancelled;
}

LIBINPUT_EXPORT double
libinput_event_gesture_get_dx(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END);

	return event->delta.x;
}

LIBINPUT_EXPORT double
libinput_event_gesture_get_dy(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END);

	return event->delta.y;
}

LIBINPUT_EXPORT double
libinput_event_gesture_get_dx_unaccelerated(
	struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END);

	return event->delta_unaccel.x;
}

LIBINPUT_EXPORT double
libinput_event_gesture_get_dy_unaccelerated(
	struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END);

	return event->delta_unaccel.y;
}

LIBINPUT_EXPORT double
libinput_event_gesture_get_scale(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END);

	return event->scale;
}

LIBINPUT_EXPORT double
libinput_event_gesture_get_angle_delta(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END);

	return event->angle;
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_x_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_X);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_y_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_Y);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_pressure_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_distance_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_DISTANCE);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_tilt_x_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_TILT_X);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_tilt_y_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_TILT_Y);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_rotation_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_slider_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_SLIDER);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_size_major_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_SIZE_MAJOR);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_size_minor_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_SIZE_MINOR);
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_wheel_has_changed(
				struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return bit_is_set(event->changed_axes,
			  LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL);
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_x(struct libinput_event_tablet_tool *event)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return evdev_convert_to_mm(device->abs.absinfo_x,
				   event->axes.point.x);
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_y(struct libinput_event_tablet_tool *event)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return evdev_convert_to_mm(device->abs.absinfo_y,
				   event->axes.point.y);
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_dx(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.delta.x;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_dy(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.delta.y;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_pressure(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.pressure;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_distance(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.distance;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_tilt_x(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.tilt.x;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_tilt_y(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.tilt.y;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_rotation(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.rotation;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_slider_position(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.slider;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_size_major(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.size.major;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_size_minor(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.size.minor;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_wheel_delta(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.wheel;
}

LIBINPUT_EXPORT int
libinput_event_tablet_tool_get_wheel_delta_discrete(
				      struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->axes.wheel_discrete;
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_x_transformed(struct libinput_event_tablet_tool *event,
					uint32_t width)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return evdev_device_transform_x(device,
					event->axes.point.x,
					width);
}

LIBINPUT_EXPORT double
libinput_event_tablet_tool_get_y_transformed(struct libinput_event_tablet_tool *event,
					uint32_t height)
{
	struct evdev_device *device = evdev_device(event->base.device);

	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return evdev_device_transform_y(device,
					event->axes.point.y,
					height);
}

LIBINPUT_EXPORT struct libinput_tablet_tool *
libinput_event_tablet_tool_get_tool(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->tool;
}

LIBINPUT_EXPORT enum libinput_tablet_tool_proximity_state
libinput_event_tablet_tool_get_proximity_state(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->proximity_state;
}

LIBINPUT_EXPORT enum libinput_tablet_tool_tip_state
libinput_event_tablet_tool_get_tip_state(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->tip_state;
}

LIBINPUT_EXPORT uint32_t
libinput_event_tablet_tool_get_time(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_tablet_tool_get_time_usec(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);

	return event->time;
}

LIBINPUT_EXPORT uint32_t
libinput_event_tablet_tool_get_button(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON);

	return event->button;
}

LIBINPUT_EXPORT enum libinput_button_state
libinput_event_tablet_tool_get_button_state(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON);

	return event->state;
}

LIBINPUT_EXPORT uint32_t
libinput_event_tablet_tool_get_seat_button_count(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON);

	return event->seat_button_count;
}

LIBINPUT_EXPORT enum libinput_tablet_tool_type
libinput_tablet_tool_get_type(struct libinput_tablet_tool *tool)
{
	return tool->type;
}

LIBINPUT_EXPORT uint64_t
libinput_tablet_tool_get_tool_id(struct libinput_tablet_tool *tool)
{
	return tool->tool_id;
}

LIBINPUT_EXPORT int
libinput_tablet_tool_is_unique(struct libinput_tablet_tool *tool)
{
	return tool->serial != 0;
}

LIBINPUT_EXPORT uint64_t
libinput_tablet_tool_get_serial(struct libinput_tablet_tool *tool)
{
	return tool->serial;
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_pressure(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_distance(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_DISTANCE);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_tilt(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_TILT_X);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_rotation(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_slider(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_SLIDER);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_wheel(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_size(struct libinput_tablet_tool *tool)
{
	return bit_is_set(tool->axis_caps,
			  LIBINPUT_TABLET_TOOL_AXIS_SIZE_MAJOR);
}

LIBINPUT_EXPORT int
libinput_tablet_tool_has_button(struct libinput_tablet_tool *tool,
				uint32_t code)
{
	if (NCHARS(code) > sizeof(tool->buttons))
		return 0;

	return bit_is_set(tool->buttons, code);
}

LIBINPUT_EXPORT void
libinput_tablet_tool_set_user_data(struct libinput_tablet_tool *tool,
				   void *user_data)
{
	tool->user_data = user_data;
}

LIBINPUT_EXPORT void *
libinput_tablet_tool_get_user_data(struct libinput_tablet_tool *tool)
{
	return tool->user_data;
}

LIBINPUT_EXPORT struct libinput_tablet_tool *
libinput_tablet_tool_ref(struct libinput_tablet_tool *tool)
{
	tool->refcount++;
	return tool;
}

LIBINPUT_EXPORT struct libinput_tablet_tool *
libinput_tablet_tool_unref(struct libinput_tablet_tool *tool)
{
	assert(tool->refcount > 0);

	tool->refcount--;
	if (tool->refcount > 0)
		return tool;

	list_remove(&tool->link);
	free(tool);
	return NULL;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_switch_get_base_event(struct libinput_event_switch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_SWITCH_TOGGLE);

	return &event->base;
}

LIBINPUT_EXPORT enum libinput_switch
libinput_event_switch_get_switch(struct libinput_event_switch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_SWITCH_TOGGLE);

	return event->sw;
}

LIBINPUT_EXPORT enum libinput_switch_state
libinput_event_switch_get_switch_state(struct libinput_event_switch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_SWITCH_TOGGLE);

	return event->state;
}

LIBINPUT_EXPORT uint32_t
libinput_event_switch_get_time(struct libinput_event_switch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_SWITCH_TOGGLE);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_switch_get_time_usec(struct libinput_event_switch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_SWITCH_TOGGLE);

	return event->time;
}

struct libinput_source *
libinput_add_fd(struct libinput *libinput,
		int fd,
		libinput_source_dispatch_t dispatch,
		void *user_data)
{
	struct libinput_source *source;
	struct epoll_event ep;

	source = zalloc(sizeof *source);
	source->dispatch = dispatch;
	source->user_data = user_data;
	source->fd = fd;

	memset(&ep, 0, sizeof ep);
	ep.events = EPOLLIN;
	ep.data.ptr = source;

	if (epoll_ctl(libinput->epoll_fd, EPOLL_CTL_ADD, fd, &ep) < 0) {
		free(source);
		return NULL;
	}

	return source;
}

void
libinput_remove_source(struct libinput *libinput,
		       struct libinput_source *source)
{
	epoll_ctl(libinput->epoll_fd, EPOLL_CTL_DEL, source->fd, NULL);
	source->fd = -1;
	list_insert(&libinput->source_destroy_list, &source->link);
}

int
libinput_init(struct libinput *libinput,
	      const struct libinput_interface *interface,
	      const struct libinput_interface_backend *interface_backend,
	      void *user_data)
{
	assert(interface->open_restricted != NULL);
	assert(interface->close_restricted != NULL);

	libinput->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (libinput->epoll_fd < 0)
		return -1;

	libinput->events_len = 4;
	libinput->events = zalloc(libinput->events_len * sizeof(*libinput->events));
	libinput->log_handler = libinput_default_log_func;
	libinput->log_priority = LIBINPUT_LOG_PRIORITY_ERROR;
	libinput->interface = interface;
	libinput->interface_backend = interface_backend;
	libinput->user_data = user_data;
	libinput->refcount = 1;
	list_init(&libinput->source_destroy_list);
	list_init(&libinput->seat_list);
	list_init(&libinput->device_group_list);
	list_init(&libinput->tool_list);

	if (libinput_timer_subsys_init(libinput) != 0) {
		free(libinput->events);
		close(libinput->epoll_fd);
		return -1;
	}

	return 0;
}

void
libinput_init_quirks(struct libinput *libinput)
{
	const char *data_path,
	           *override_file = NULL;
	struct quirks_context *quirks;

	if (libinput->quirks_initialized)
		return;

	/* If we fail, we'll fail next time too */
	libinput->quirks_initialized = true;

	data_path = getenv("LIBINPUT_QUIRKS_DIR");
	if (!data_path) {
		data_path = LIBINPUT_QUIRKS_DIR;
		override_file = LIBINPUT_QUIRKS_OVERRIDE_FILE;
	}

	quirks = quirks_init_subsystem(data_path,
				       override_file,
				       log_msg_va,
				       libinput,
				       QLOG_LIBINPUT_LOGGING);
	if (!quirks) {
		log_error(libinput,
			  "Failed to load the device quirks from %s%s%s. "
			  "This will negatively affect device behavior. "
			  "See %s/device-quirks.html for details.\n",
			  data_path,
			  override_file ? " and " : "",
			  override_file ? override_file : "",
			  HTTP_DOC_LINK
			  );
		return;
	}

	libinput->quirks = quirks;
}

static void
libinput_device_destroy(struct libinput_device *device);

static void
libinput_seat_destroy(struct libinput_seat *seat);

static void
libinput_drop_destroyed_sources(struct libinput *libinput)
{
	struct libinput_source *source;

	list_for_each_safe(source, &libinput->source_destroy_list, link)
		free(source);
	list_init(&libinput->source_destroy_list);
}

LIBINPUT_EXPORT struct libinput *
libinput_ref(struct libinput *libinput)
{
	libinput->refcount++;
	return libinput;
}

LIBINPUT_EXPORT struct libinput *
libinput_unref(struct libinput *libinput)
{
	struct libinput_event *event;
	struct libinput_device *device;
	struct libinput_seat *seat;
	struct libinput_tablet_tool *tool;
	struct libinput_device_group *group;

	if (libinput == NULL)
		return NULL;

	assert(libinput->refcount > 0);
	libinput->refcount--;
	if (libinput->refcount > 0)
		return libinput;

	libinput_suspend(libinput);

	libinput->interface_backend->destroy(libinput);

	while ((event = libinput_get_event(libinput)))
	       libinput_event_destroy(event);

	free(libinput->events);

	list_for_each_safe(seat, &libinput->seat_list, link) {
		list_for_each_safe(device,
				   &seat->devices_list,
				   link)
			libinput_device_destroy(device);

		libinput_seat_destroy(seat);
	}

	list_for_each_safe(group,
			   &libinput->device_group_list,
			   link) {
		libinput_device_group_destroy(group);
	}

	list_for_each_safe(tool, &libinput->tool_list, link) {
		libinput_tablet_tool_unref(tool);
	}

	libinput_timer_subsys_destroy(libinput);
	libinput_drop_destroyed_sources(libinput);
	quirks_context_unref(libinput->quirks);
	close(libinput->epoll_fd);
	free(libinput);

	return NULL;
}

static void
libinput_event_tablet_tool_destroy(struct libinput_event_tablet_tool *event)
{
	libinput_tablet_tool_unref(event->tool);
}

static void
libinput_event_tablet_pad_destroy(struct libinput_event_tablet_pad *event)
{
	if (event->base.type != LIBINPUT_EVENT_TABLET_PAD_KEY)
		libinput_tablet_pad_mode_group_unref(event->mode_group);
}

LIBINPUT_EXPORT void
libinput_event_destroy(struct libinput_event *event)
{
	if (event == NULL)
		return;

	switch(event->type) {
	case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
	case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
	case LIBINPUT_EVENT_TABLET_TOOL_TIP:
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		libinput_event_tablet_tool_destroy(
		   libinput_event_get_tablet_tool_event(event));
		break;
	case LIBINPUT_EVENT_TABLET_PAD_RING:
	case LIBINPUT_EVENT_TABLET_PAD_STRIP:
	case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
	case LIBINPUT_EVENT_TABLET_PAD_KEY:
		libinput_event_tablet_pad_destroy(
		   libinput_event_get_tablet_pad_event(event));
		break;
	default:
		break;
	}

	if (event->device)
		libinput_device_unref(event->device);

	free(event);
}

int
open_restricted(struct libinput *libinput,
		const char *path, int flags)
{
	return libinput->interface->open_restricted(path,
						    flags,
						    libinput->user_data);
}

void
close_restricted(struct libinput *libinput, int fd)
{
	libinput->interface->close_restricted(fd, libinput->user_data);
}

bool
ignore_litest_test_suite_device(struct udev_device *device)
{
	if (!getenv("LIBINPUT_RUNNING_TEST_SUITE") &&
	    udev_device_get_property_value(device, "LIBINPUT_TEST_DEVICE"))
		return true;

	return false;
}

void
libinput_seat_init(struct libinput_seat *seat,
		   struct libinput *libinput,
		   const char *physical_name,
		   const char *logical_name,
		   libinput_seat_destroy_func destroy)
{
	seat->refcount = 1;
	seat->libinput = libinput;
	seat->physical_name = safe_strdup(physical_name);
	seat->logical_name = safe_strdup(logical_name);
	seat->destroy = destroy;
	list_init(&seat->devices_list);
	list_insert(&libinput->seat_list, &seat->link);
}

LIBINPUT_EXPORT struct libinput_seat *
libinput_seat_ref(struct libinput_seat *seat)
{
	seat->refcount++;
	return seat;
}

static void
libinput_seat_destroy(struct libinput_seat *seat)
{
	list_remove(&seat->link);
	free(seat->logical_name);
	free(seat->physical_name);
	seat->destroy(seat);
}

LIBINPUT_EXPORT struct libinput_seat *
libinput_seat_unref(struct libinput_seat *seat)
{
	assert(seat->refcount > 0);
	seat->refcount--;
	if (seat->refcount == 0) {
		libinput_seat_destroy(seat);
		return NULL;
	}

	return seat;
}

LIBINPUT_EXPORT void
libinput_seat_set_user_data(struct libinput_seat *seat, void *user_data)
{
	seat->user_data = user_data;
}

LIBINPUT_EXPORT void *
libinput_seat_get_user_data(struct libinput_seat *seat)
{
	return seat->user_data;
}

LIBINPUT_EXPORT struct libinput *
libinput_seat_get_context(struct libinput_seat *seat)
{
	return seat->libinput;
}

LIBINPUT_EXPORT const char *
libinput_seat_get_physical_name(struct libinput_seat *seat)
{
	return seat->physical_name;
}

LIBINPUT_EXPORT const char *
libinput_seat_get_logical_name(struct libinput_seat *seat)
{
	return seat->logical_name;
}

void
libinput_device_init(struct libinput_device *device,
		     struct libinput_seat *seat)
{
	device->seat = seat;
	device->refcount = 1;
	list_init(&device->event_listeners);
}

LIBINPUT_EXPORT struct libinput_device *
libinput_device_ref(struct libinput_device *device)
{
	device->refcount++;
	return device;
}

static void
libinput_device_destroy(struct libinput_device *device)
{
	assert(list_empty(&device->event_listeners));
	evdev_device_destroy(evdev_device(device));
}

LIBINPUT_EXPORT struct libinput_device *
libinput_device_unref(struct libinput_device *device)
{
	assert(device->refcount > 0);
	device->refcount--;
	if (device->refcount == 0) {
		libinput_device_destroy(device);
		return NULL;
	}

	return device;
}

LIBINPUT_EXPORT int
libinput_get_fd(struct libinput *libinput)
{
	return libinput->epoll_fd;
}

LIBINPUT_EXPORT int
libinput_dispatch(struct libinput *libinput)
{
	static uint8_t take_time_snapshot;
	struct libinput_source *source;
	struct epoll_event ep[32];
	int i, count;

	/* Every 10 calls to libinput_dispatch() we take the current time so
	 * we can check the delay between our current time and the event
	 * timestamps */
	if ((++take_time_snapshot % 10) == 0)
		libinput->dispatch_time = libinput_now(libinput);
	else if (libinput->dispatch_time)
		libinput->dispatch_time = 0;

	count = epoll_wait(libinput->epoll_fd, ep, ARRAY_LENGTH(ep), 0);
	if (count < 0)
		return -errno;

	for (i = 0; i < count; ++i) {
		source = ep[i].data.ptr;
		if (source->fd == -1)
			continue;

		source->dispatch(source->user_data);
	}

	libinput_drop_destroyed_sources(libinput);

	return 0;
}

void
libinput_device_init_event_listener(struct libinput_event_listener *listener)
{
	list_init(&listener->link);
}

void
libinput_device_add_event_listener(struct libinput_device *device,
				   struct libinput_event_listener *listener,
				   void (*notify_func)(
						uint64_t time,
						struct libinput_event *event,
						void *notify_func_data),
				   void *notify_func_data)
{
	listener->notify_func = notify_func;
	listener->notify_func_data = notify_func_data;
	list_insert(&device->event_listeners, &listener->link);
}

void
libinput_device_remove_event_listener(struct libinput_event_listener *listener)
{
	list_remove(&listener->link);
}

static uint32_t
update_seat_key_count(struct libinput_seat *seat,
		      int32_t key,
		      enum libinput_key_state state)
{
	assert(key >= 0 && key <= KEY_MAX);

	switch (state) {
	case LIBINPUT_KEY_STATE_PRESSED:
		return ++seat->button_count[key];
	case LIBINPUT_KEY_STATE_RELEASED:
		/* We might not have received the first PRESSED event. */
		if (seat->button_count[key] == 0)
			return 0;

		return --seat->button_count[key];
	}

	return 0;
}

static uint32_t
update_seat_button_count(struct libinput_seat *seat,
			 int32_t button,
			 enum libinput_button_state state)
{
	assert(button >= 0 && button <= KEY_MAX);

	switch (state) {
	case LIBINPUT_BUTTON_STATE_PRESSED:
		return ++seat->button_count[button];
	case LIBINPUT_BUTTON_STATE_RELEASED:
		/* We might not have received the first PRESSED event. */
		if (seat->button_count[button] == 0)
			return 0;

		return --seat->button_count[button];
	}

	return 0;
}

static void
init_event_base(struct libinput_event *event,
		struct libinput_device *device,
		enum libinput_event_type type)
{
	event->type = type;
	event->device = device;
}

static void
post_base_event(struct libinput_device *device,
		enum libinput_event_type type,
		struct libinput_event *event)
{
	struct libinput *libinput = device->seat->libinput;
	init_event_base(event, device, type);
	libinput_post_event(libinput, event);
}

static void
post_device_event(struct libinput_device *device,
		  uint64_t time,
		  enum libinput_event_type type,
		  struct libinput_event *event)
{
	struct libinput_event_listener *listener;
#if 0
	struct libinput *libinput = device->seat->libinput;

	if (libinput->last_event_time > time) {
		log_bug_libinput(device->seat->libinput,
				 "out-of-order timestamps for %s time %" PRIu64 "\n",
				 event_type_to_str(type),
				 time);
	}
	libinput->last_event_time = time;
#endif

	init_event_base(event, device, type);

	list_for_each_safe(listener, &device->event_listeners, link)
		listener->notify_func(time, event, listener->notify_func_data);

	libinput_post_event(device->seat->libinput, event);
}

void
notify_added_device(struct libinput_device *device)
{
	struct libinput_event_device_notify *added_device_event;

	added_device_event = zalloc(sizeof *added_device_event);

	post_base_event(device,
			LIBINPUT_EVENT_DEVICE_ADDED,
			&added_device_event->base);

#ifdef __clang_analyzer__
	/* clang doesn't realize we're not leaking the event here, so
	 * pretend to free it  */
	free(added_device_event);
#endif
}

void
notify_removed_device(struct libinput_device *device)
{
	struct libinput_event_device_notify *removed_device_event;

	removed_device_event = zalloc(sizeof *removed_device_event);

	post_base_event(device,
			LIBINPUT_EVENT_DEVICE_REMOVED,
			&removed_device_event->base);

#ifdef __clang_analyzer__
	/* clang doesn't realize we're not leaking the event here, so
	 * pretend to free it  */
	free(removed_device_event);
#endif
}

static inline bool
device_has_cap(struct libinput_device *device,
	       enum libinput_device_capability cap)
{
	const char *capability;

	if (libinput_device_has_capability(device, cap))
		return true;

	switch (cap) {
	case LIBINPUT_DEVICE_CAP_POINTER:
		capability = "CAP_POINTER";
		break;
	case LIBINPUT_DEVICE_CAP_KEYBOARD:
		capability = "CAP_KEYBOARD";
		break;
	case LIBINPUT_DEVICE_CAP_TOUCH:
		capability = "CAP_TOUCH";
		break;
	case LIBINPUT_DEVICE_CAP_GESTURE:
		capability = "CAP_GESTURE";
		break;
	case LIBINPUT_DEVICE_CAP_TABLET_TOOL:
		capability = "CAP_TABLET_TOOL";
		break;
	case LIBINPUT_DEVICE_CAP_TABLET_PAD:
		capability = "CAP_TABLET_PAD";
		break;
	case LIBINPUT_DEVICE_CAP_SWITCH:
		capability = "CAP_SWITCH";
		break;
	}

	log_bug_libinput(device->seat->libinput,
			 "Event for missing capability %s on device \"%s\"\n",
			 capability,
			 libinput_device_get_name(device));

	return false;
}

void
keyboard_notify_key(struct libinput_device *device,
		    uint64_t time,
		    uint32_t key,
		    enum libinput_key_state state)
{
	struct libinput_event_keyboard *key_event;
	uint32_t seat_key_count;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_KEYBOARD))
		return;

	key_event = zalloc(sizeof *key_event);

	seat_key_count = update_seat_key_count(device->seat, key, state);

	*key_event = (struct libinput_event_keyboard) {
		.time = time,
		.key = key,
		.state = state,
		.seat_key_count = seat_key_count,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_KEYBOARD_KEY,
			  &key_event->base);
}

void
pointer_notify_motion(struct libinput_device *device,
		      uint64_t time,
		      const struct normalized_coords *delta,
		      const struct device_float_coords *raw)
{
	struct libinput_event_pointer *motion_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	motion_event = zalloc(sizeof *motion_event);

	*motion_event = (struct libinput_event_pointer) {
		.time = time,
		.delta = *delta,
		.delta_raw = *raw,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_MOTION,
			  &motion_event->base);
}

void
pointer_notify_motion_absolute(struct libinput_device *device,
			       uint64_t time,
			       const struct device_coords *point)
{
	struct libinput_event_pointer *motion_absolute_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	motion_absolute_event = zalloc(sizeof *motion_absolute_event);

	*motion_absolute_event = (struct libinput_event_pointer) {
		.time = time,
		.absolute = *point,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE,
			  &motion_absolute_event->base);
}

void
pointer_notify_button(struct libinput_device *device,
		      uint64_t time,
		      int32_t button,
		      enum libinput_button_state state)
{
	struct libinput_event_pointer *button_event;
	int32_t seat_button_count;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	button_event = zalloc(sizeof *button_event);

	seat_button_count = update_seat_button_count(device->seat,
						     button,
						     state);

	*button_event = (struct libinput_event_pointer) {
		.time = time,
		.button = button,
		.state = state,
		.seat_button_count = seat_button_count,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_BUTTON,
			  &button_event->base);
}

void
pointer_notify_axis_finger(struct libinput_device *device,
			  uint64_t time,
			  uint32_t axes,
			  const struct normalized_coords *delta)
{
	struct libinput_event_pointer *axis_event, *axis_event_legacy;
	const struct discrete_coords zero_discrete = {0};
	const struct wheel_v120 zero_v120 = {0};

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	axis_event = zalloc(sizeof *axis_event);
	axis_event_legacy = zalloc(sizeof *axis_event_legacy);

	*axis_event = (struct libinput_event_pointer) {
		.time = time,
		.delta = *delta,
		.source = LIBINPUT_POINTER_AXIS_SOURCE_FINGER,
		.axes = axes,
		.discrete = zero_discrete,
		.v120 = zero_v120,
	};
	*axis_event_legacy = *axis_event;

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			  &axis_event->base);
	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_AXIS,
			  &axis_event_legacy->base);
}

void
pointer_notify_axis_continuous(struct libinput_device *device,
			       uint64_t time,
			       uint32_t axes,
			       const struct normalized_coords *delta)
{
	struct libinput_event_pointer *axis_event, *axis_event_legacy;
	const struct discrete_coords zero_discrete = {0};
	const struct wheel_v120 zero_v120 = {0};

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	axis_event = zalloc(sizeof *axis_event);
	axis_event_legacy = zalloc(sizeof *axis_event_legacy);

	*axis_event = (struct libinput_event_pointer) {
		.time = time,
		.delta = *delta,
		.source = LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS,
		.axes = axes,
		.discrete = zero_discrete,
		.v120 = zero_v120,
	};
	*axis_event_legacy = *axis_event;

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			  &axis_event->base);
	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_AXIS,
			  &axis_event_legacy->base);
}

void
pointer_notify_axis_legacy_wheel(struct libinput_device *device,
				 uint64_t time,
				 uint32_t axes,
				 const struct normalized_coords *delta,
				 const struct discrete_coords *discrete)
{
	struct libinput_event_pointer *axis_event;
	const struct wheel_v120 zero_v120 = {0};

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	axis_event = zalloc(sizeof *axis_event);

	*axis_event = (struct libinput_event_pointer) {
		.time = time,
		.delta = *delta,
		.source = LIBINPUT_POINTER_AXIS_SOURCE_WHEEL,
		.axes = axes,
		.discrete = *discrete,
		.v120 = zero_v120,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_AXIS,
			  &axis_event->base);
}

void
pointer_notify_axis_wheel(struct libinput_device *device,
			  uint64_t time,
			  uint32_t axes,
			  const struct normalized_coords *delta,
			  const struct wheel_v120 *v120)
{
	struct libinput_event_pointer *axis_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_POINTER))
		return;

	axis_event = zalloc(sizeof *axis_event);

	*axis_event = (struct libinput_event_pointer) {
		.time = time,
		.delta = *delta,
		.source = LIBINPUT_POINTER_AXIS_SOURCE_WHEEL,
		.axes = axes,
		.discrete.x = 0,
		.discrete.y = 0,
		.v120 = *v120,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			  &axis_event->base);

	/* legacy wheel events are sent separately */
}

void
touch_notify_touch_down(struct libinput_device *device,
			uint64_t time,
			int32_t slot,
			int32_t seat_slot,
			const struct device_coords *point)
{
	struct libinput_event_touch *touch_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_TOUCH))
		return;

	touch_event = zalloc(sizeof *touch_event);

	*touch_event = (struct libinput_event_touch) {
		.time = time,
		.slot = slot,
		.seat_slot = seat_slot,
		.point = *point,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_TOUCH_DOWN,
			  &touch_event->base);
}

void
touch_notify_touch_motion(struct libinput_device *device,
			  uint64_t time,
			  int32_t slot,
			  int32_t seat_slot,
			  const struct device_coords *point)
{
	struct libinput_event_touch *touch_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_TOUCH))
		return;

	touch_event = zalloc(sizeof *touch_event);

	*touch_event = (struct libinput_event_touch) {
		.time = time,
		.slot = slot,
		.seat_slot = seat_slot,
		.point = *point,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_TOUCH_MOTION,
			  &touch_event->base);
}

void
touch_notify_touch_up(struct libinput_device *device,
		      uint64_t time,
		      int32_t slot,
		      int32_t seat_slot)
{
	struct libinput_event_touch *touch_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_TOUCH))
		return;

	touch_event = zalloc(sizeof *touch_event);

	*touch_event = (struct libinput_event_touch) {
		.time = time,
		.slot = slot,
		.seat_slot = seat_slot,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_TOUCH_UP,
			  &touch_event->base);
}

void
touch_notify_touch_cancel(struct libinput_device *device,
			  uint64_t time,
			  int32_t slot,
			  int32_t seat_slot)
{
	struct libinput_event_touch *touch_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_TOUCH))
		return;

	touch_event = zalloc(sizeof *touch_event);

	*touch_event = (struct libinput_event_touch) {
		.time = time,
		.slot = slot,
		.seat_slot = seat_slot,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_TOUCH_CANCEL,
			  &touch_event->base);
}

void
touch_notify_frame(struct libinput_device *device,
		   uint64_t time)
{
	struct libinput_event_touch *touch_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_TOUCH))
		return;

	touch_event = zalloc(sizeof *touch_event);

	*touch_event = (struct libinput_event_touch) {
		.time = time,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_TOUCH_FRAME,
			  &touch_event->base);
}

void
tablet_notify_axis(struct libinput_device *device,
		   uint64_t time,
		   struct libinput_tablet_tool *tool,
		   enum libinput_tablet_tool_tip_state tip_state,
		   unsigned char *changed_axes,
		   const struct tablet_axes *axes)
{
	struct libinput_event_tablet_tool *axis_event;

	axis_event = zalloc(sizeof *axis_event);

	*axis_event = (struct libinput_event_tablet_tool) {
		.time = time,
		.tool = libinput_tablet_tool_ref(tool),
		.proximity_state = LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN,
		.tip_state = tip_state,
		.axes = *axes,
	};

	memcpy(axis_event->changed_axes,
	       changed_axes,
	       sizeof(axis_event->changed_axes));

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			  &axis_event->base);
}

void
tablet_notify_proximity(struct libinput_device *device,
			uint64_t time,
			struct libinput_tablet_tool *tool,
			enum libinput_tablet_tool_proximity_state proximity_state,
			unsigned char *changed_axes,
			const struct tablet_axes *axes)
{
	struct libinput_event_tablet_tool *proximity_event;

	proximity_event = zalloc(sizeof *proximity_event);

	*proximity_event = (struct libinput_event_tablet_tool) {
		.time = time,
		.tool = libinput_tablet_tool_ref(tool),
		.tip_state = LIBINPUT_TABLET_TOOL_TIP_UP,
		.proximity_state = proximity_state,
		.axes = *axes,
	};
	memcpy(proximity_event->changed_axes,
	       changed_axes,
	       sizeof(proximity_event->changed_axes));

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY,
			  &proximity_event->base);
}

void
tablet_notify_tip(struct libinput_device *device,
		  uint64_t time,
		  struct libinput_tablet_tool *tool,
		  enum libinput_tablet_tool_tip_state tip_state,
		  unsigned char *changed_axes,
		  const struct tablet_axes *axes)
{
	struct libinput_event_tablet_tool *tip_event;

	tip_event = zalloc(sizeof *tip_event);

	*tip_event = (struct libinput_event_tablet_tool) {
		.time = time,
		.tool = libinput_tablet_tool_ref(tool),
		.tip_state = tip_state,
		.proximity_state = LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN,
		.axes = *axes,
	};
	memcpy(tip_event->changed_axes,
	       changed_axes,
	       sizeof(tip_event->changed_axes));

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_TOOL_TIP,
			  &tip_event->base);
}

void
tablet_notify_button(struct libinput_device *device,
		     uint64_t time,
		     struct libinput_tablet_tool *tool,
		     enum libinput_tablet_tool_tip_state tip_state,
		     const struct tablet_axes *axes,
		     int32_t button,
		     enum libinput_button_state state)
{
	struct libinput_event_tablet_tool *button_event;
	int32_t seat_button_count;

	button_event = zalloc(sizeof *button_event);

	seat_button_count = update_seat_button_count(device->seat,
						     button,
						     state);

	*button_event = (struct libinput_event_tablet_tool) {
		.time = time,
		.tool = libinput_tablet_tool_ref(tool),
		.button = button,
		.state = state,
		.seat_button_count = seat_button_count,
		.proximity_state = LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN,
		.tip_state = tip_state,
		.axes = *axes,
	};

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_TOOL_BUTTON,
			  &button_event->base);
}

void
tablet_pad_notify_button(struct libinput_device *device,
			 uint64_t time,
			 int32_t button,
			 enum libinput_button_state state,
			 struct libinput_tablet_pad_mode_group *group)
{
	struct libinput_event_tablet_pad *button_event;
	unsigned int mode;

	button_event = zalloc(sizeof *button_event);

	mode = libinput_tablet_pad_mode_group_get_mode(group);

	*button_event = (struct libinput_event_tablet_pad) {
		.time = time,
		.button.number = button,
		.button.state = state,
		.mode_group = libinput_tablet_pad_mode_group_ref(group),
		.mode = mode,
	};

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_PAD_BUTTON,
			  &button_event->base);
}

void
tablet_pad_notify_ring(struct libinput_device *device,
		       uint64_t time,
		       unsigned int number,
		       double value,
		       enum libinput_tablet_pad_ring_axis_source source,
		       struct libinput_tablet_pad_mode_group *group)
{
	struct libinput_event_tablet_pad *ring_event;
	unsigned int mode;

	ring_event = zalloc(sizeof *ring_event);

	mode = libinput_tablet_pad_mode_group_get_mode(group);

	*ring_event = (struct libinput_event_tablet_pad) {
		.time = time,
		.ring.number = number,
		.ring.position = value,
		.ring.source = source,
		.mode_group = libinput_tablet_pad_mode_group_ref(group),
		.mode = mode,
	};

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_PAD_RING,
			  &ring_event->base);
}

void
tablet_pad_notify_strip(struct libinput_device *device,
			uint64_t time,
			unsigned int number,
			double value,
			enum libinput_tablet_pad_strip_axis_source source,
			struct libinput_tablet_pad_mode_group *group)
{
	struct libinput_event_tablet_pad *strip_event;
	unsigned int mode;

	strip_event = zalloc(sizeof *strip_event);

	mode = libinput_tablet_pad_mode_group_get_mode(group);

	*strip_event = (struct libinput_event_tablet_pad) {
		.time = time,
		.strip.number = number,
		.strip.position = value,
		.strip.source = source,
		.mode_group = libinput_tablet_pad_mode_group_ref(group),
		.mode = mode,
	};

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_PAD_STRIP,
			  &strip_event->base);
}

void
tablet_pad_notify_key(struct libinput_device *device,
		      uint64_t time,
		      int32_t key,
		      enum libinput_key_state state)
{
	struct libinput_event_tablet_pad *key_event;

	key_event = zalloc(sizeof *key_event);

	*key_event = (struct libinput_event_tablet_pad) {
		.time = time,
		.key.code = key,
		.key.state = state,
	};

	post_device_event(device,
			  time,
			  LIBINPUT_EVENT_TABLET_PAD_KEY,
			  &key_event->base);
}

static void
gesture_notify(struct libinput_device *device,
	       uint64_t time,
	       enum libinput_event_type type,
	       int finger_count,
	       bool cancelled,
	       const struct normalized_coords *delta,
	       const struct normalized_coords *unaccel,
	       double scale,
	       double angle)
{
	struct libinput_event_gesture *gesture_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	gesture_event = zalloc(sizeof *gesture_event);

	*gesture_event = (struct libinput_event_gesture) {
		.time = time,
		.finger_count = finger_count,
		.cancelled = cancelled,
		.delta = *delta,
		.delta_unaccel = *unaccel,
		.scale = scale,
		.angle = angle,
	};

	post_device_event(device, time, type,
			  &gesture_event->base);
}

void
gesture_notify_swipe(struct libinput_device *device,
		     uint64_t time,
		     enum libinput_event_type type,
		     int finger_count,
		     const struct normalized_coords *delta,
		     const struct normalized_coords *unaccel)
{
	gesture_notify(device, time, type, finger_count, 0, delta, unaccel,
		       0.0, 0.0);
}

void
gesture_notify_swipe_end(struct libinput_device *device,
			 uint64_t time,
			 int finger_count,
			 bool cancelled)
{
	const struct normalized_coords zero = { 0.0, 0.0 };

	gesture_notify(device, time, LIBINPUT_EVENT_GESTURE_SWIPE_END,
		       finger_count, cancelled, &zero, &zero, 0.0, 0.0);
}

void
gesture_notify_pinch(struct libinput_device *device,
		     uint64_t time,
		     enum libinput_event_type type,
		     int finger_count,
		     const struct normalized_coords *delta,
		     const struct normalized_coords *unaccel,
		     double scale,
		     double angle)
{
	gesture_notify(device, time, type, finger_count, 0,
		       delta, unaccel, scale, angle);
}

void
gesture_notify_pinch_end(struct libinput_device *device,
			 uint64_t time,
			 int finger_count,
			 double scale,
			 bool cancelled)
{
	const struct normalized_coords zero = { 0.0, 0.0 };

	gesture_notify(device, time, LIBINPUT_EVENT_GESTURE_PINCH_END,
		       finger_count, cancelled, &zero, &zero, scale, 0.0);
}

void
gesture_notify_hold(struct libinput_device *device,
		    uint64_t time,
		    int finger_count)
{
	const struct normalized_coords zero = { 0.0, 0.0 };

	gesture_notify(device, time, LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
		       finger_count, 0, &zero, &zero, 0.0, 0.0);
}

void
gesture_notify_hold_end(struct libinput_device *device,
			uint64_t time,
			int finger_count,
			bool cancelled)
{
	const struct normalized_coords zero = { 0.0, 0.0 };

	gesture_notify(device, time, LIBINPUT_EVENT_GESTURE_HOLD_END,
		       finger_count, cancelled, &zero, &zero, 0, 0.0);
}

void
switch_notify_toggle(struct libinput_device *device,
		     uint64_t time,
		     enum libinput_switch sw,
		     enum libinput_switch_state state)
{
	struct libinput_event_switch *switch_event;

	if (!device_has_cap(device, LIBINPUT_DEVICE_CAP_SWITCH))
		return;

	switch_event = zalloc(sizeof *switch_event);

	*switch_event = (struct libinput_event_switch) {
		.time = time,
		.sw = sw,
		.state = state,
	};

	post_device_event(device, time,
			  LIBINPUT_EVENT_SWITCH_TOGGLE,
			  &switch_event->base);

#ifdef __clang_analyzer__
	/* clang doesn't realize we're not leaking the event here, so
	 * pretend to free it  */
	free(switch_event);
#endif
}

static void
libinput_post_event(struct libinput *libinput,
		    struct libinput_event *event)
{
	struct libinput_event **events = libinput->events;
	size_t events_len = libinput->events_len;
	size_t events_count = libinput->events_count;
	size_t move_len;
	size_t new_out;

#if 0
	log_debug(libinput, "Queuing %s\n", event_type_to_str(event->type));
#endif

	events_count++;
	if (events_count > events_len) {
		void *tmp;

		events_len *= 2;
		tmp = realloc(events, events_len * sizeof *events);
		if (!tmp) {
			log_error(libinput,
				  "Failed to reallocate event ring buffer. "
				  "Events may be discarded\n");
			return;
		}

		events = tmp;

		if (libinput->events_count > 0 && libinput->events_in == 0) {
			libinput->events_in = libinput->events_len;
		} else if (libinput->events_count > 0 &&
			   libinput->events_out >= libinput->events_in) {
			move_len = libinput->events_len - libinput->events_out;
			new_out = events_len - move_len;
			memmove(events + new_out,
				events + libinput->events_out,
				move_len * sizeof *events);
			libinput->events_out = new_out;
		}

		libinput->events = events;
		libinput->events_len = events_len;
	}

	if (event->device)
		libinput_device_ref(event->device);

	libinput->events_count = events_count;
	events[libinput->events_in] = event;
	libinput->events_in = (libinput->events_in + 1) % libinput->events_len;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_get_event(struct libinput *libinput)
{
	struct libinput_event *event;

	if (libinput->events_count == 0)
		return NULL;

	event = libinput->events[libinput->events_out];
	libinput->events_out =
		(libinput->events_out + 1) % libinput->events_len;
	libinput->events_count--;

	return event;
}

LIBINPUT_EXPORT enum libinput_event_type
libinput_next_event_type(struct libinput *libinput)
{
	struct libinput_event *event;

	if (libinput->events_count == 0)
		return LIBINPUT_EVENT_NONE;

	event = libinput->events[libinput->events_out];
	return event->type;
}

LIBINPUT_EXPORT void
libinput_set_user_data(struct libinput *libinput,
		       void *user_data)
{
	libinput->user_data = user_data;
}

LIBINPUT_EXPORT void *
libinput_get_user_data(struct libinput *libinput)
{
	return libinput->user_data;
}

LIBINPUT_EXPORT int
libinput_resume(struct libinput *libinput)
{
	return libinput->interface_backend->resume(libinput);
}

LIBINPUT_EXPORT void
libinput_suspend(struct libinput *libinput)
{
	libinput->interface_backend->suspend(libinput);
}

LIBINPUT_EXPORT void
libinput_device_set_user_data(struct libinput_device *device, void *user_data)
{
	device->user_data = user_data;
}

LIBINPUT_EXPORT void *
libinput_device_get_user_data(struct libinput_device *device)
{
	return device->user_data;
}

LIBINPUT_EXPORT struct libinput *
libinput_device_get_context(struct libinput_device *device)
{
	return libinput_seat_get_context(device->seat);
}

LIBINPUT_EXPORT struct libinput_device_group *
libinput_device_get_device_group(struct libinput_device *device)
{
	return device->group;
}

LIBINPUT_EXPORT const char *
libinput_device_get_sysname(struct libinput_device *device)
{
	return evdev_device_get_sysname((struct evdev_device *) device);
}

LIBINPUT_EXPORT const char *
libinput_device_get_name(struct libinput_device *device)
{
	return evdev_device_get_name((struct evdev_device *) device);
}

LIBINPUT_EXPORT unsigned int
libinput_device_get_id_product(struct libinput_device *device)
{
	return evdev_device_get_id_product((struct evdev_device *) device);
}

LIBINPUT_EXPORT unsigned int
libinput_device_get_id_vendor(struct libinput_device *device)
{
	return evdev_device_get_id_vendor((struct evdev_device *) device);
}

LIBINPUT_EXPORT const char *
libinput_device_get_output_name(struct libinput_device *device)
{
	return evdev_device_get_output((struct evdev_device *) device);
}

LIBINPUT_EXPORT struct libinput_seat *
libinput_device_get_seat(struct libinput_device *device)
{
	return device->seat;
}

LIBINPUT_EXPORT int
libinput_device_set_seat_logical_name(struct libinput_device *device,
				      const char *name)
{
	struct libinput *libinput = device->seat->libinput;

	if (name == NULL)
		return -1;

	return libinput->interface_backend->device_change_seat(device,
							       name);
}

LIBINPUT_EXPORT struct udev_device *
libinput_device_get_udev_device(struct libinput_device *device)
{
	return evdev_device_get_udev_device((struct evdev_device *)device);
}

LIBINPUT_EXPORT void
libinput_device_led_update(struct libinput_device *device,
			   enum libinput_led leds)
{
	evdev_device_led_update((struct evdev_device *) device, leds);
}

LIBINPUT_EXPORT int
libinput_device_has_capability(struct libinput_device *device,
			       enum libinput_device_capability capability)
{
	return evdev_device_has_capability((struct evdev_device *) device,
					   capability);
}

LIBINPUT_EXPORT int
libinput_device_get_size(struct libinput_device *device,
			 double *width,
			 double *height)
{
	return evdev_device_get_size((struct evdev_device *)device,
				     width,
				     height);
}

LIBINPUT_EXPORT int
libinput_device_pointer_has_button(struct libinput_device *device, uint32_t code)
{
	return evdev_device_has_button((struct evdev_device *)device, code);
}

LIBINPUT_EXPORT int
libinput_device_keyboard_has_key(struct libinput_device *device, uint32_t code)
{
	return evdev_device_has_key((struct evdev_device *)device, code);
}

LIBINPUT_EXPORT int
libinput_device_touch_get_touch_count(struct libinput_device *device)
{
	return evdev_device_get_touch_count((struct evdev_device *)device);
}

LIBINPUT_EXPORT int
libinput_device_switch_has_switch(struct libinput_device *device,
				  enum libinput_switch sw)
{
	return evdev_device_has_switch((struct evdev_device *)device, sw);
}

LIBINPUT_EXPORT int
libinput_device_tablet_pad_has_key(struct libinput_device *device, uint32_t code)
{
	return evdev_device_tablet_pad_has_key((struct evdev_device *)device,
					       code);
}

LIBINPUT_EXPORT int
libinput_device_tablet_pad_get_num_buttons(struct libinput_device *device)
{
	return evdev_device_tablet_pad_get_num_buttons((struct evdev_device *)device);
}

LIBINPUT_EXPORT int
libinput_device_tablet_pad_get_num_rings(struct libinput_device *device)
{
	return evdev_device_tablet_pad_get_num_rings((struct evdev_device *)device);
}

LIBINPUT_EXPORT int
libinput_device_tablet_pad_get_num_strips(struct libinput_device *device)
{
	return evdev_device_tablet_pad_get_num_strips((struct evdev_device *)device);
}

LIBINPUT_EXPORT int
libinput_device_tablet_pad_get_num_mode_groups(struct libinput_device *device)
{
	return evdev_device_tablet_pad_get_num_mode_groups((struct evdev_device *)device);
}

LIBINPUT_EXPORT struct libinput_tablet_pad_mode_group*
libinput_device_tablet_pad_get_mode_group(struct libinput_device *device,
					  unsigned int index)
{
	return evdev_device_tablet_pad_get_mode_group((struct evdev_device *)device,
						      index);
}

LIBINPUT_EXPORT unsigned int
libinput_tablet_pad_mode_group_get_num_modes(
				     struct libinput_tablet_pad_mode_group *group)
{
	return group->num_modes;
}

LIBINPUT_EXPORT unsigned int
libinput_tablet_pad_mode_group_get_mode(struct libinput_tablet_pad_mode_group *group)
{
	return group->current_mode;
}

LIBINPUT_EXPORT unsigned int
libinput_tablet_pad_mode_group_get_index(struct libinput_tablet_pad_mode_group *group)
{
	return group->index;
}

LIBINPUT_EXPORT int
libinput_tablet_pad_mode_group_has_button(struct libinput_tablet_pad_mode_group *group,
					  unsigned int button)
{
	if ((int)button >=
	    libinput_device_tablet_pad_get_num_buttons(group->device))
		return 0;

	return !!(group->button_mask & bit(button));
}

LIBINPUT_EXPORT int
libinput_tablet_pad_mode_group_has_ring(struct libinput_tablet_pad_mode_group *group,
					unsigned int ring)
{
	if ((int)ring >=
	    libinput_device_tablet_pad_get_num_rings(group->device))
		return 0;

	return !!(group->ring_mask & bit(ring));
}

LIBINPUT_EXPORT int
libinput_tablet_pad_mode_group_has_strip(struct libinput_tablet_pad_mode_group *group,
					 unsigned int strip)
{
	if ((int)strip >=
	    libinput_device_tablet_pad_get_num_strips(group->device))
		return 0;

	return !!(group->strip_mask & bit(strip));
}

LIBINPUT_EXPORT int
libinput_tablet_pad_mode_group_button_is_toggle(struct libinput_tablet_pad_mode_group *group,
						unsigned int button)
{
	if ((int)button >=
	    libinput_device_tablet_pad_get_num_buttons(group->device))
		return 0;

	return !!(group->toggle_button_mask & bit(button));
}

LIBINPUT_EXPORT struct libinput_tablet_pad_mode_group *
libinput_tablet_pad_mode_group_ref(
			struct libinput_tablet_pad_mode_group *group)
{
	group->refcount++;
	return group;
}

LIBINPUT_EXPORT struct libinput_tablet_pad_mode_group *
libinput_tablet_pad_mode_group_unref(
			struct libinput_tablet_pad_mode_group *group)
{
	assert(group->refcount > 0);

	group->refcount--;
	if (group->refcount > 0)
		return group;

	list_remove(&group->link);
	group->destroy(group);
	return NULL;
}

LIBINPUT_EXPORT void
libinput_tablet_pad_mode_group_set_user_data(
			struct libinput_tablet_pad_mode_group *group,
			void *user_data)
{
	group->user_data = user_data;
}

LIBINPUT_EXPORT void *
libinput_tablet_pad_mode_group_get_user_data(
			struct libinput_tablet_pad_mode_group *group)
{
	return group->user_data;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_device_notify_get_base_event(struct libinput_event_device_notify *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_DEVICE_ADDED,
			   LIBINPUT_EVENT_DEVICE_REMOVED);

	return &event->base;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_keyboard_get_base_event(struct libinput_event_keyboard *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_KEYBOARD_KEY);

	return &event->base;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_pointer_get_base_event(struct libinput_event_pointer *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_POINTER_MOTION,
			   LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE,
			   LIBINPUT_EVENT_POINTER_BUTTON,
			   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
			   LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			   LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			   LIBINPUT_EVENT_POINTER_AXIS);

	return &event->base;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_touch_get_base_event(struct libinput_event_touch *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_TOUCH_DOWN,
			   LIBINPUT_EVENT_TOUCH_UP,
			   LIBINPUT_EVENT_TOUCH_MOTION,
			   LIBINPUT_EVENT_TOUCH_CANCEL,
			   LIBINPUT_EVENT_TOUCH_FRAME);

	return &event->base;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_gesture_get_base_event(struct libinput_event_gesture *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
			   LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
			   LIBINPUT_EVENT_GESTURE_SWIPE_END,
			   LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
			   LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			   LIBINPUT_EVENT_GESTURE_PINCH_END,
			   LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
			   LIBINPUT_EVENT_GESTURE_HOLD_END);

	return &event->base;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_tablet_tool_get_base_event(struct libinput_event_tablet_tool *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_TABLET_TOOL_AXIS,
			   LIBINPUT_EVENT_TABLET_TOOL_TIP,
			   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY,
			   LIBINPUT_EVENT_TABLET_TOOL_BUTTON);

	return &event->base;
}

LIBINPUT_EXPORT double
libinput_event_tablet_pad_get_ring_position(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_TABLET_PAD_RING);

	return event->ring.position;
}

LIBINPUT_EXPORT unsigned int
libinput_event_tablet_pad_get_ring_number(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_RING);

	return event->ring.number;
}

LIBINPUT_EXPORT enum libinput_tablet_pad_ring_axis_source
libinput_event_tablet_pad_get_ring_source(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   LIBINPUT_TABLET_PAD_RING_SOURCE_UNKNOWN,
			   LIBINPUT_EVENT_TABLET_PAD_RING);

	return event->ring.source;
}

LIBINPUT_EXPORT double
libinput_event_tablet_pad_get_strip_position(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0.0,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP);

	return event->strip.position;
}

LIBINPUT_EXPORT unsigned int
libinput_event_tablet_pad_get_strip_number(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP);

	return event->strip.number;
}

LIBINPUT_EXPORT enum libinput_tablet_pad_strip_axis_source
libinput_event_tablet_pad_get_strip_source(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   LIBINPUT_TABLET_PAD_STRIP_SOURCE_UNKNOWN,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP);

	return event->strip.source;
}

LIBINPUT_EXPORT uint32_t
libinput_event_tablet_pad_get_button_number(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON);

	return event->button.number;
}

LIBINPUT_EXPORT enum libinput_button_state
libinput_event_tablet_pad_get_button_state(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   LIBINPUT_BUTTON_STATE_RELEASED,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON);

	return event->button.state;
}

LIBINPUT_EXPORT uint32_t
libinput_event_tablet_pad_get_key(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_KEY);

	return event->key.code;
}

LIBINPUT_EXPORT enum libinput_key_state
libinput_event_tablet_pad_get_key_state(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   LIBINPUT_KEY_STATE_RELEASED,
			   LIBINPUT_EVENT_TABLET_PAD_KEY);

	return event->key.state;
}

LIBINPUT_EXPORT unsigned int
libinput_event_tablet_pad_get_mode(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_RING,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON);

	return event->mode;
}

LIBINPUT_EXPORT struct libinput_tablet_pad_mode_group *
libinput_event_tablet_pad_get_mode_group(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_TABLET_PAD_RING,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON);

	return event->mode_group;
}

LIBINPUT_EXPORT uint32_t
libinput_event_tablet_pad_get_time(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_RING,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON,
			   LIBINPUT_EVENT_TABLET_PAD_KEY);

	return us2ms(event->time);
}

LIBINPUT_EXPORT uint64_t
libinput_event_tablet_pad_get_time_usec(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   0,
			   LIBINPUT_EVENT_TABLET_PAD_RING,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON,
			   LIBINPUT_EVENT_TABLET_PAD_KEY);

	return event->time;
}

LIBINPUT_EXPORT struct libinput_event *
libinput_event_tablet_pad_get_base_event(struct libinput_event_tablet_pad *event)
{
	require_event_type(libinput_event_get_context(&event->base),
			   event->base.type,
			   NULL,
			   LIBINPUT_EVENT_TABLET_PAD_RING,
			   LIBINPUT_EVENT_TABLET_PAD_STRIP,
			   LIBINPUT_EVENT_TABLET_PAD_BUTTON,
			   LIBINPUT_EVENT_TABLET_PAD_KEY);

	return &event->base;
}

LIBINPUT_EXPORT struct libinput_device_group *
libinput_device_group_ref(struct libinput_device_group *group)
{
	group->refcount++;
	return group;
}

struct libinput_device_group *
libinput_device_group_create(struct libinput *libinput,
			     const char *identifier)
{
	struct libinput_device_group *group;

	group = zalloc(sizeof *group);
	group->refcount = 1;
	group->identifier = safe_strdup(identifier);

	list_init(&group->link);
	list_insert(&libinput->device_group_list, &group->link);

	return group;
}

struct libinput_device_group *
libinput_device_group_find_group(struct libinput *libinput,
				 const char *identifier)
{
	struct libinput_device_group *g = NULL;

	list_for_each(g, &libinput->device_group_list, link) {
		if (identifier && g->identifier &&
		    streq(g->identifier, identifier)) {
			return g;
		}
	}

	return NULL;
}

void
libinput_device_set_device_group(struct libinput_device *device,
				 struct libinput_device_group *group)
{
	device->group = group;
	libinput_device_group_ref(group);
}

static void
libinput_device_group_destroy(struct libinput_device_group *group)
{
	list_remove(&group->link);
	free(group->identifier);
	free(group);
}

LIBINPUT_EXPORT struct libinput_device_group *
libinput_device_group_unref(struct libinput_device_group *group)
{
	assert(group->refcount > 0);
	group->refcount--;
	if (group->refcount == 0) {
		libinput_device_group_destroy(group);
		return NULL;
	}

	return group;
}

LIBINPUT_EXPORT void
libinput_device_group_set_user_data(struct libinput_device_group *group,
				    void *user_data)
{
	group->user_data = user_data;
}

LIBINPUT_EXPORT void *
libinput_device_group_get_user_data(struct libinput_device_group *group)
{
	return group->user_data;
}

LIBINPUT_EXPORT const char *
libinput_config_status_to_str(enum libinput_config_status status)
{
	const char *str = NULL;

	switch(status) {
	case LIBINPUT_CONFIG_STATUS_SUCCESS:
		str = "Success";
		break;
	case LIBINPUT_CONFIG_STATUS_UNSUPPORTED:
		str = "Unsupported configuration option";
		break;
	case LIBINPUT_CONFIG_STATUS_INVALID:
		str = "Invalid argument range";
		break;
	}

	return str;
}

LIBINPUT_EXPORT int
libinput_device_config_tap_get_finger_count(struct libinput_device *device)
{
	return device->config.tap ? device->config.tap->count(device) : 0;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_tap_set_enabled(struct libinput_device *device,
				       enum libinput_config_tap_state enable)
{
	if (enable != LIBINPUT_CONFIG_TAP_ENABLED &&
	    enable != LIBINPUT_CONFIG_TAP_DISABLED)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return enable ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				LIBINPUT_CONFIG_STATUS_SUCCESS;

	return device->config.tap->set_enabled(device, enable);

}

LIBINPUT_EXPORT enum libinput_config_tap_state
libinput_device_config_tap_get_enabled(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_TAP_DISABLED;

	return device->config.tap->get_enabled(device);
}

LIBINPUT_EXPORT enum libinput_config_tap_state
libinput_device_config_tap_get_default_enabled(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_TAP_DISABLED;

	return device->config.tap->get_default(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_tap_set_button_map(struct libinput_device *device,
					    enum libinput_config_tap_button_map map)
{
	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	return device->config.tap->set_map(device, map);
}

LIBINPUT_EXPORT enum libinput_config_tap_button_map
libinput_device_config_tap_get_button_map(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_TAP_MAP_LRM;

	return device->config.tap->get_map(device);
}

LIBINPUT_EXPORT enum libinput_config_tap_button_map
libinput_device_config_tap_get_default_button_map(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_TAP_MAP_LRM;

	return device->config.tap->get_default_map(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_tap_set_drag_enabled(struct libinput_device *device,
					    enum libinput_config_drag_state enable)
{
	if (enable != LIBINPUT_CONFIG_DRAG_ENABLED &&
	    enable != LIBINPUT_CONFIG_DRAG_DISABLED)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return enable ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				LIBINPUT_CONFIG_STATUS_SUCCESS;

	return device->config.tap->set_drag_enabled(device, enable);
}

LIBINPUT_EXPORT enum libinput_config_drag_state
libinput_device_config_tap_get_drag_enabled(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_DRAG_DISABLED;

	return device->config.tap->get_drag_enabled(device);
}

LIBINPUT_EXPORT enum libinput_config_drag_state
libinput_device_config_tap_get_default_drag_enabled(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_DRAG_DISABLED;

	return device->config.tap->get_default_drag_enabled(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_tap_set_drag_lock_enabled(struct libinput_device *device,
						 enum libinput_config_drag_lock_state enable)
{
	if (enable != LIBINPUT_CONFIG_DRAG_LOCK_ENABLED &&
	    enable != LIBINPUT_CONFIG_DRAG_LOCK_DISABLED)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return enable ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				LIBINPUT_CONFIG_STATUS_SUCCESS;

	return device->config.tap->set_draglock_enabled(device, enable);
}

LIBINPUT_EXPORT enum libinput_config_drag_lock_state
libinput_device_config_tap_get_drag_lock_enabled(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_DRAG_LOCK_DISABLED;

	return device->config.tap->get_draglock_enabled(device);
}

LIBINPUT_EXPORT enum libinput_config_drag_lock_state
libinput_device_config_tap_get_default_drag_lock_enabled(struct libinput_device *device)
{
	if (libinput_device_config_tap_get_finger_count(device) == 0)
		return LIBINPUT_CONFIG_DRAG_LOCK_DISABLED;

	return device->config.tap->get_default_draglock_enabled(device);
}

LIBINPUT_EXPORT int
libinput_device_config_calibration_has_matrix(struct libinput_device *device)
{
	return device->config.calibration ?
		device->config.calibration->has_matrix(device) : 0;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_calibration_set_matrix(struct libinput_device *device,
					      const float matrix[6])
{
	if (!libinput_device_config_calibration_has_matrix(device))
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	return device->config.calibration->set_matrix(device, matrix);
}

LIBINPUT_EXPORT int
libinput_device_config_calibration_get_matrix(struct libinput_device *device,
					      float matrix[6])
{
	if (!libinput_device_config_calibration_has_matrix(device))
		return 0;

	return device->config.calibration->get_matrix(device, matrix);
}

LIBINPUT_EXPORT int
libinput_device_config_calibration_get_default_matrix(struct libinput_device *device,
						      float matrix[6])
{
	if (!libinput_device_config_calibration_has_matrix(device))
		return 0;

	return device->config.calibration->get_default_matrix(device, matrix);
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_send_events_get_modes(struct libinput_device *device)
{
	uint32_t modes = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

	if (device->config.sendevents)
		modes |= device->config.sendevents->get_modes(device);

	return modes;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_send_events_set_mode(struct libinput_device *device,
					    uint32_t mode)
{
	if ((libinput_device_config_send_events_get_modes(device) & mode) != mode)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	if (device->config.sendevents)
		return device->config.sendevents->set_mode(device, mode);

	/* mode must be _ENABLED to get here */
	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_send_events_get_mode(struct libinput_device *device)
{
	if (device->config.sendevents)
		return device->config.sendevents->get_mode(device);

	return LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_send_events_get_default_mode(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
}

LIBINPUT_EXPORT int
libinput_device_config_accel_is_available(struct libinput_device *device)
{
	return device->config.accel ?
		device->config.accel->available(device) : 0;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_accel_set_speed(struct libinput_device *device,
				       double speed)
{
	/* Need the negation in case speed is NaN */
	if (!(speed >= -1.0 && speed <= 1.0))
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (!libinput_device_config_accel_is_available(device))
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	return device->config.accel->set_speed(device, speed);
}
LIBINPUT_EXPORT double
libinput_device_config_accel_get_speed(struct libinput_device *device)
{
	if (!libinput_device_config_accel_is_available(device))
		return 0;

	return device->config.accel->get_speed(device);
}

LIBINPUT_EXPORT double
libinput_device_config_accel_get_default_speed(struct libinput_device *device)
{
	if (!libinput_device_config_accel_is_available(device))
		return 0;

	return device->config.accel->get_default_speed(device);
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_accel_get_profiles(struct libinput_device *device)
{
	if (!libinput_device_config_accel_is_available(device))
		return 0;

	return device->config.accel->get_profiles(device);
}

LIBINPUT_EXPORT enum libinput_config_accel_profile
libinput_device_config_accel_get_profile(struct libinput_device *device)
{
	if (!libinput_device_config_accel_is_available(device))
		return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;

	return device->config.accel->get_profile(device);
}

LIBINPUT_EXPORT enum libinput_config_accel_profile
libinput_device_config_accel_get_default_profile(struct libinput_device *device)
{
	if (!libinput_device_config_accel_is_available(device))
		return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;

	return device->config.accel->get_default_profile(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_accel_set_profile(struct libinput_device *device,
					 enum libinput_config_accel_profile profile)
{
	switch (profile) {
	case LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT:
	case LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE:
	case LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	if (!libinput_device_config_accel_is_available(device) ||
	    (libinput_device_config_accel_get_profiles(device) & profile) == 0)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	return device->config.accel->set_profile(device, profile);
}

static inline struct libinput_config_accel_custom_func *
libinput_config_accel_custom_func_create(void)
{
	struct libinput_config_accel_custom_func *func = zalloc(sizeof(*func));

	func->step = 1.0;
	func->npoints = 2;
	func->points[0] = 0.0; /* default to a flat unaccelerated function */
	func->points[1] = 1.0;

	return func;
}

static inline void
libinput_config_accel_custom_func_destroy(struct libinput_config_accel_custom_func * func)
{
	free(func);
}

LIBINPUT_EXPORT struct libinput_config_accel *
libinput_config_accel_create(enum libinput_config_accel_profile profile)
{
	struct libinput_config_accel *config = zalloc(sizeof(*config));

	config->profile = profile;

	switch (profile) {
	case LIBINPUT_CONFIG_ACCEL_PROFILE_NONE:
		break;
	case LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT:
	case LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE:
		return config;
	case LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM:
		config->custom.fallback = libinput_config_accel_custom_func_create();
		return config;
	}

	free(config);
	return NULL;
}

LIBINPUT_EXPORT void
libinput_config_accel_destroy(struct libinput_config_accel *accel_config)
{
	libinput_config_accel_custom_func_destroy(accel_config->custom.fallback);
	libinput_config_accel_custom_func_destroy(accel_config->custom.motion);
	libinput_config_accel_custom_func_destroy(accel_config->custom.scroll);
	free(accel_config);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_accel_apply(struct libinput_device *device,
				   struct libinput_config_accel *accel_config)
{
	enum libinput_config_status status;
	status = libinput_device_config_accel_set_profile(device, accel_config->profile);
	if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
		return status;

	switch (accel_config->profile) {
	case LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT:
	case LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE:
	{
		double speed = libinput_device_config_accel_get_default_speed(device);
		return libinput_device_config_accel_set_speed(device, speed);
	}
	case LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM:
		return device->config.accel->set_accel_config(device, accel_config);

	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_config_accel_set_points(struct libinput_config_accel *config,
				 enum libinput_config_accel_type accel_type,
				 double step, size_t npoints, double *points)
{
	if (config->profile != LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	switch (accel_type) {
	case LIBINPUT_ACCEL_TYPE_FALLBACK:
	case LIBINPUT_ACCEL_TYPE_MOTION:
	case LIBINPUT_ACCEL_TYPE_SCROLL:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	if (step <= 0 || step > LIBINPUT_ACCEL_STEP_MAX)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (npoints < LIBINPUT_ACCEL_NPOINTS_MIN || npoints > LIBINPUT_ACCEL_NPOINTS_MAX)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	for (size_t idx = 0; idx < npoints; idx++) {
		if (points[idx] < LIBINPUT_ACCEL_POINT_MIN_VALUE ||
		    points[idx] > LIBINPUT_ACCEL_POINT_MAX_VALUE)
			return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	struct libinput_config_accel_custom_func *func = libinput_config_accel_custom_func_create();

	func->step = step;
	func->npoints = npoints;
	memcpy(func->points, points, sizeof(*points) * npoints);

	switch (accel_type) {
	case LIBINPUT_ACCEL_TYPE_FALLBACK:
		libinput_config_accel_custom_func_destroy(config->custom.fallback);
		config->custom.fallback = func;
		break;
	case LIBINPUT_ACCEL_TYPE_MOTION:
		libinput_config_accel_custom_func_destroy(config->custom.motion);
		config->custom.motion = func;
		break;
	case LIBINPUT_ACCEL_TYPE_SCROLL:
		libinput_config_accel_custom_func_destroy(config->custom.scroll);
		config->custom.scroll = func;
		break;
	}

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

LIBINPUT_EXPORT int
libinput_device_config_scroll_has_natural_scroll(struct libinput_device *device)
{
	if (!device->config.natural_scroll)
		return 0;

	return device->config.natural_scroll->has(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_scroll_set_natural_scroll_enabled(struct libinput_device *device,
							 int enabled)
{
	if (!libinput_device_config_scroll_has_natural_scroll(device))
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	return device->config.natural_scroll->set_enabled(device, enabled);
}

LIBINPUT_EXPORT int
libinput_device_config_scroll_get_natural_scroll_enabled(struct libinput_device *device)
{
	if (!device->config.natural_scroll)
		return 0;

	return device->config.natural_scroll->get_enabled(device);
}

LIBINPUT_EXPORT int
libinput_device_config_scroll_get_default_natural_scroll_enabled(struct libinput_device *device)
{
	if (!device->config.natural_scroll)
		return 0;

	return device->config.natural_scroll->get_default_enabled(device);
}

LIBINPUT_EXPORT int
libinput_device_config_left_handed_is_available(struct libinput_device *device)
{
	if (!device->config.left_handed)
		return 0;

	return device->config.left_handed->has(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_left_handed_set(struct libinput_device *device,
				       int left_handed)
{
	if (!libinput_device_config_left_handed_is_available(device))
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	return device->config.left_handed->set(device, left_handed);
}

LIBINPUT_EXPORT int
libinput_device_config_left_handed_get(struct libinput_device *device)
{
	if (!libinput_device_config_left_handed_is_available(device))
		return 0;

	return device->config.left_handed->get(device);
}

LIBINPUT_EXPORT int
libinput_device_config_left_handed_get_default(struct libinput_device *device)
{
	if (!libinput_device_config_left_handed_is_available(device))
		return 0;

	return device->config.left_handed->get_default(device);
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_click_get_methods(struct libinput_device *device)
{
	if (device->config.click_method)
		return device->config.click_method->get_methods(device);

	return 0;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_click_set_method(struct libinput_device *device,
					enum libinput_config_click_method method)
{
	/* Check method is a single valid method */
	switch (method) {
	case LIBINPUT_CONFIG_CLICK_METHOD_NONE:
	case LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS:
	case LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	if ((libinput_device_config_click_get_methods(device) & method) != method)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	if (device->config.click_method)
		return device->config.click_method->set_method(device, method);

	/* method must be _NONE to get here */
	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

LIBINPUT_EXPORT enum libinput_config_click_method
libinput_device_config_click_get_method(struct libinput_device *device)
{
	if (device->config.click_method)
		return device->config.click_method->get_method(device);

	return LIBINPUT_CONFIG_CLICK_METHOD_NONE;
}

LIBINPUT_EXPORT enum libinput_config_click_method
libinput_device_config_click_get_default_method(struct libinput_device *device)
{
	if (device->config.click_method)
		return device->config.click_method->get_default_method(device);

	return LIBINPUT_CONFIG_CLICK_METHOD_NONE;
}

LIBINPUT_EXPORT int
libinput_device_config_middle_emulation_is_available(
		struct libinput_device *device)
{
	if (device->config.middle_emulation)
		return device->config.middle_emulation->available(device);

	return LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_middle_emulation_set_enabled(
		struct libinput_device *device,
		enum libinput_config_middle_emulation_state enable)
{
	int available =
		libinput_device_config_middle_emulation_is_available(device);

	switch (enable) {
	case LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED:
		if (!available)
			return LIBINPUT_CONFIG_STATUS_SUCCESS;
		break;
	case LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED:
		if (!available)
			return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	return device->config.middle_emulation->set(device, enable);
}

LIBINPUT_EXPORT enum libinput_config_middle_emulation_state
libinput_device_config_middle_emulation_get_enabled(
		struct libinput_device *device)
{
	if (!libinput_device_config_middle_emulation_is_available(device))
		return LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED;

	return device->config.middle_emulation->get(device);
}

LIBINPUT_EXPORT enum libinput_config_middle_emulation_state
libinput_device_config_middle_emulation_get_default_enabled(
		struct libinput_device *device)
{
	if (!libinput_device_config_middle_emulation_is_available(device))
		return LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED;

	return device->config.middle_emulation->get_default(device);
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_scroll_get_methods(struct libinput_device *device)
{
	if (device->config.scroll_method)
		return device->config.scroll_method->get_methods(device);

	return 0;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_scroll_set_method(struct libinput_device *device,
					 enum libinput_config_scroll_method method)
{
	/* Check method is a single valid method */
	switch (method) {
	case LIBINPUT_CONFIG_SCROLL_NO_SCROLL:
	case LIBINPUT_CONFIG_SCROLL_2FG:
	case LIBINPUT_CONFIG_SCROLL_EDGE:
	case LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	if ((libinput_device_config_scroll_get_methods(device) & method) != method)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	if (device->config.scroll_method)
		return device->config.scroll_method->set_method(device, method);

	/* method must be _NO_SCROLL to get here */
	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

LIBINPUT_EXPORT enum libinput_config_scroll_method
libinput_device_config_scroll_get_method(struct libinput_device *device)
{
	if (device->config.scroll_method)
		return device->config.scroll_method->get_method(device);

	return LIBINPUT_CONFIG_SCROLL_NO_SCROLL;
}

LIBINPUT_EXPORT enum libinput_config_scroll_method
libinput_device_config_scroll_get_default_method(struct libinput_device *device)
{
	if (device->config.scroll_method)
		return device->config.scroll_method->get_default_method(device);

	return LIBINPUT_CONFIG_SCROLL_NO_SCROLL;
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_scroll_set_button(struct libinput_device *device,
					 uint32_t button)
{
	if ((libinput_device_config_scroll_get_methods(device) &
	     LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) == 0)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	if (button && !libinput_device_pointer_has_button(device, button))
		return LIBINPUT_CONFIG_STATUS_INVALID;

	return device->config.scroll_method->set_button(device, button);
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_scroll_get_button(struct libinput_device *device)
{
	if ((libinput_device_config_scroll_get_methods(device) &
	     LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) == 0)
		return 0;

	return device->config.scroll_method->get_button(device);
}

LIBINPUT_EXPORT uint32_t
libinput_device_config_scroll_get_default_button(struct libinput_device *device)
{
	if ((libinput_device_config_scroll_get_methods(device) &
	     LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) == 0)
		return 0;

	return device->config.scroll_method->get_default_button(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_scroll_set_button_lock(struct libinput_device *device,
					      enum libinput_config_scroll_button_lock_state state)
{
	if ((libinput_device_config_scroll_get_methods(device) &
	     LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) == 0)
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;

	switch (state) {
	case LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_ENABLED:
	case LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_DISABLED:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	return device->config.scroll_method->set_button_lock(device, state);
}

LIBINPUT_EXPORT enum libinput_config_scroll_button_lock_state
libinput_device_config_scroll_get_button_lock(struct libinput_device *device)
{
	if ((libinput_device_config_scroll_get_methods(device) &
	     LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) == 0)
		return LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_DISABLED;

	return device->config.scroll_method->get_button_lock(device);
}

LIBINPUT_EXPORT enum libinput_config_scroll_button_lock_state
libinput_device_config_scroll_get_default_button_lock(struct libinput_device *device)
{
	if ((libinput_device_config_scroll_get_methods(device) &
	     LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) == 0)
		return LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_DISABLED;

	return device->config.scroll_method->get_default_button_lock(device);
}

LIBINPUT_EXPORT int
libinput_device_config_dwt_is_available(struct libinput_device *device)
{
	if (!device->config.dwt)
		return 0;

	return device->config.dwt->is_available(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_dwt_set_enabled(struct libinput_device *device,
				       enum libinput_config_dwt_state enable)
{
	if (enable != LIBINPUT_CONFIG_DWT_ENABLED &&
	    enable != LIBINPUT_CONFIG_DWT_DISABLED)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (!libinput_device_config_dwt_is_available(device))
		return enable ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				LIBINPUT_CONFIG_STATUS_SUCCESS;

	return device->config.dwt->set_enabled(device, enable);
}

LIBINPUT_EXPORT enum libinput_config_dwt_state
libinput_device_config_dwt_get_enabled(struct libinput_device *device)
{
	if (!libinput_device_config_dwt_is_available(device))
		return LIBINPUT_CONFIG_DWT_DISABLED;

	return device->config.dwt->get_enabled(device);
}

LIBINPUT_EXPORT enum libinput_config_dwt_state
libinput_device_config_dwt_get_default_enabled(struct libinput_device *device)
{
	if (!libinput_device_config_dwt_is_available(device))
		return LIBINPUT_CONFIG_DWT_DISABLED;

	return device->config.dwt->get_default_enabled(device);
}

LIBINPUT_EXPORT int
libinput_device_config_dwtp_is_available(struct libinput_device *device)
{
	if (!device->config.dwtp)
		return 0;

	return device->config.dwtp->is_available(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_dwtp_set_enabled(struct libinput_device *device,
				       enum libinput_config_dwtp_state enable)
{
	if (enable != LIBINPUT_CONFIG_DWTP_ENABLED &&
	    enable != LIBINPUT_CONFIG_DWTP_DISABLED)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (!libinput_device_config_dwtp_is_available(device))
		return enable ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				LIBINPUT_CONFIG_STATUS_SUCCESS;

	return device->config.dwtp->set_enabled(device, enable);
}

LIBINPUT_EXPORT enum libinput_config_dwtp_state
libinput_device_config_dwtp_get_enabled(struct libinput_device *device)
{
	if (!libinput_device_config_dwtp_is_available(device))
		return LIBINPUT_CONFIG_DWTP_DISABLED;

	return device->config.dwtp->get_enabled(device);
}

LIBINPUT_EXPORT enum libinput_config_dwtp_state
libinput_device_config_dwtp_get_default_enabled(struct libinput_device *device)
{
	if (!libinput_device_config_dwtp_is_available(device))
		return LIBINPUT_CONFIG_DWTP_DISABLED;

	return device->config.dwtp->get_default_enabled(device);
}

LIBINPUT_EXPORT int
libinput_device_config_rotation_is_available(struct libinput_device *device)
{
	if (!device->config.rotation)
		return 0;

	return device->config.rotation->is_available(device);
}

LIBINPUT_EXPORT enum libinput_config_status
libinput_device_config_rotation_set_angle(struct libinput_device *device,
					  unsigned int degrees_cw)
{
	if (!libinput_device_config_rotation_is_available(device))
		return degrees_cw ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				    LIBINPUT_CONFIG_STATUS_SUCCESS;

	if (degrees_cw >= 360)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	return device->config.rotation->set_angle(device, degrees_cw);
}

LIBINPUT_EXPORT unsigned int
libinput_device_config_rotation_get_angle(struct libinput_device *device)
{
	if (!libinput_device_config_rotation_is_available(device))
		return 0;

	return device->config.rotation->get_angle(device);
}

LIBINPUT_EXPORT unsigned int
libinput_device_config_rotation_get_default_angle(struct libinput_device *device)
{
	if (!libinput_device_config_rotation_is_available(device))
		return 0;

	return device->config.rotation->get_default_angle(device);
}

#if HAVE_LIBWACOM
WacomDeviceDatabase *
libinput_libwacom_ref(struct libinput *li)
{
	WacomDeviceDatabase *db = NULL;
	if (!li->libwacom.db) {
		db = libwacom_database_new();
		if (!db) {
			log_error(li,
				  "Failed to initialize libwacom context\n");
			return NULL;
		}

		li->libwacom.db = db;
		li->libwacom.refcount = 0;
	}

	li->libwacom.refcount++;
	db = li->libwacom.db;
	return db;
}

void
libinput_libwacom_unref(struct libinput *li)
{
	if (!li->libwacom.db)
		return;

	assert(li->libwacom.refcount >= 1);

	if (--li->libwacom.refcount == 0) {
		libwacom_database_destroy(li->libwacom.db);
		li->libwacom.db = NULL;
	}
}
#endif
