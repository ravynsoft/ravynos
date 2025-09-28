/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2013 Jonas Ådahl
 * Copyright © 2013-2017 Red Hat, Inc.
 * Copyright © 2017 James Ye <jye836@gmail.com>
 * Copyright © 2021 José Expósito
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

#include "evdev-fallback.h"
#include "util-input-event.h"

#define ACC_V120_THRESHOLD 60
#define WHEEL_SCROLL_TIMEOUT ms2us(500)

enum wheel_event {
	WHEEL_EVENT_SCROLL_ACCUMULATED,
	WHEEL_EVENT_SCROLL,
	WHEEL_EVENT_SCROLL_TIMEOUT,
	WHEEL_EVENT_SCROLL_DIR_CHANGED,
};

static inline const char *
wheel_state_to_str(enum wheel_state state)
{
	switch(state) {
	CASE_RETURN_STRING(WHEEL_STATE_NONE);
	CASE_RETURN_STRING(WHEEL_STATE_ACCUMULATING_SCROLL);
	CASE_RETURN_STRING(WHEEL_STATE_SCROLLING);
	}
	return NULL;
}

static inline const char*
wheel_event_to_str(enum wheel_event event)
{
	switch(event) {
	CASE_RETURN_STRING(WHEEL_EVENT_SCROLL_ACCUMULATED);
	CASE_RETURN_STRING(WHEEL_EVENT_SCROLL);
	CASE_RETURN_STRING(WHEEL_EVENT_SCROLL_TIMEOUT);
	CASE_RETURN_STRING(WHEEL_EVENT_SCROLL_DIR_CHANGED);
	}
	return NULL;
}

static inline void
log_wheel_bug(struct fallback_dispatch *dispatch, enum wheel_event event)
{
	evdev_log_bug_libinput(dispatch->device,
			       "invalid wheel event %s in state %s\n",
			       wheel_event_to_str(event),
			       wheel_state_to_str(dispatch->wheel.state));
}

static inline void
wheel_set_scroll_timer(struct fallback_dispatch *dispatch, uint64_t time)
{
	libinput_timer_set(&dispatch->wheel.scroll_timer,
			   time + WHEEL_SCROLL_TIMEOUT);
}

static inline void
wheel_cancel_scroll_timer(struct fallback_dispatch *dispatch)
{
	libinput_timer_cancel(&dispatch->wheel.scroll_timer);
}

static void
wheel_handle_event_on_state_none(struct fallback_dispatch *dispatch,
				 enum wheel_event event,
				 uint64_t time)
{
	switch (event) {
	case WHEEL_EVENT_SCROLL:
		dispatch->wheel.state = WHEEL_STATE_ACCUMULATING_SCROLL;
		break;
	case WHEEL_EVENT_SCROLL_DIR_CHANGED:
		break;
	case WHEEL_EVENT_SCROLL_ACCUMULATED:
	case WHEEL_EVENT_SCROLL_TIMEOUT:
		log_wheel_bug(dispatch, event);
		break;
	}
}

static void
wheel_handle_event_on_state_accumulating_scroll(struct fallback_dispatch *dispatch,
						enum wheel_event event,
						uint64_t time)
{
	switch (event) {
	case WHEEL_EVENT_SCROLL_ACCUMULATED:
		dispatch->wheel.state = WHEEL_STATE_SCROLLING;
		wheel_set_scroll_timer(dispatch, time);
		break;
	case WHEEL_EVENT_SCROLL:
		/* Ignore scroll while accumulating deltas */
		break;
	case WHEEL_EVENT_SCROLL_DIR_CHANGED:
		dispatch->wheel.state = WHEEL_STATE_NONE;
		break;
	case WHEEL_EVENT_SCROLL_TIMEOUT:
		log_wheel_bug(dispatch, event);
		break;
	}
}

static void
wheel_handle_event_on_state_scrolling(struct fallback_dispatch *dispatch,
				      enum wheel_event event,
				      uint64_t time)
{
	switch (event) {
	case WHEEL_EVENT_SCROLL:
		wheel_cancel_scroll_timer(dispatch);
		wheel_set_scroll_timer(dispatch, time);
		break;
	case WHEEL_EVENT_SCROLL_TIMEOUT:
		dispatch->wheel.state = WHEEL_STATE_NONE;
		break;
	case WHEEL_EVENT_SCROLL_DIR_CHANGED:
		wheel_cancel_scroll_timer(dispatch);
		dispatch->wheel.state = WHEEL_STATE_NONE;
		break;
	case WHEEL_EVENT_SCROLL_ACCUMULATED:
		log_wheel_bug(dispatch, event);
		break;
	}
}

static void
wheel_handle_event(struct fallback_dispatch *dispatch,
		   enum wheel_event event,
		   uint64_t time)
{
	enum wheel_state oldstate = dispatch->wheel.state;

	switch (oldstate) {
	case WHEEL_STATE_NONE:
		wheel_handle_event_on_state_none(dispatch, event, time);
		break;
	case WHEEL_STATE_ACCUMULATING_SCROLL:
		wheel_handle_event_on_state_accumulating_scroll(dispatch,
								event,
								time);
		break;
	case WHEEL_STATE_SCROLLING:
		wheel_handle_event_on_state_scrolling(dispatch, event, time);
		break;
	}

	if (oldstate != dispatch->wheel.state) {
		evdev_log_debug(dispatch->device,
				"wheel state %s → %s → %s\n",
				wheel_state_to_str(oldstate),
				wheel_event_to_str(event),
				wheel_state_to_str(dispatch->wheel.state));
	}
}

static void
wheel_flush_scroll(struct fallback_dispatch *dispatch,
		   struct evdev_device *device,
		   uint64_t time)
{
	struct normalized_coords wheel_degrees = { 0.0, 0.0 };
	struct discrete_coords discrete = { 0.0, 0.0 };
	struct wheel_v120 v120 = { 0.0, 0.0 };

	/* This mouse has a trackstick instead of a mouse wheel and sends
	 * trackstick data via REL_WHEEL. Normalize it like normal x/y coordinates.
	 */
	if (device->model_flags & EVDEV_MODEL_LENOVO_SCROLLPOINT) {
		const struct device_float_coords raw = {
			.x = dispatch->wheel.lo_res.x,
			.y = dispatch->wheel.lo_res.y * -1,
		};
		const struct normalized_coords normalized =
				filter_dispatch_scroll(device->pointer.filter,
						       &raw,
						       device,
						       time);
		evdev_post_scroll(device,
				  time,
				  LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS,
				  &normalized);
		dispatch->wheel.hi_res.x = 0;
		dispatch->wheel.hi_res.y = 0;
		dispatch->wheel.lo_res.x = 0;
		dispatch->wheel.lo_res.y = 0;

		return;
	}

	if (dispatch->wheel.hi_res.y != 0) {
		int value = dispatch->wheel.hi_res.y;

		v120.y = -1 * value;
		wheel_degrees.y = -1 * value/120.0 * device->scroll.wheel_click_angle.y;
		evdev_notify_axis_wheel(
			device,
			time,
			bit(LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL),
			&wheel_degrees,
			&v120);
		dispatch->wheel.hi_res.y = 0;
	}

	if (dispatch->wheel.lo_res.y != 0) {
		int value = dispatch->wheel.lo_res.y;

		wheel_degrees.y = -1 * value * device->scroll.wheel_click_angle.y;
		discrete.y = -1 * value;
		evdev_notify_axis_legacy_wheel(
			device,
			time,
			bit(LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL),
			&wheel_degrees,
			&discrete);
		dispatch->wheel.lo_res.y = 0;
	}

	if (dispatch->wheel.hi_res.x != 0) {
		int value = dispatch->wheel.hi_res.x;

		v120.x = value;
		wheel_degrees.x = value/120.0 * device->scroll.wheel_click_angle.x;
		evdev_notify_axis_wheel(
			device,
			time,
			bit(LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL),
			&wheel_degrees,
			&v120);
		dispatch->wheel.hi_res.x = 0;
	}

	if (dispatch->wheel.lo_res.x != 0) {
		int value = dispatch->wheel.lo_res.x;

		wheel_degrees.x = value * device->scroll.wheel_click_angle.x;
		discrete.x = value;
		evdev_notify_axis_legacy_wheel(
			device,
			time,
			bit(LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL),
			&wheel_degrees,
			&discrete);
		dispatch->wheel.lo_res.x = 0;
	}
}

static void
wheel_handle_state_none(struct fallback_dispatch *dispatch,
			struct evdev_device *device,
			uint64_t time)
{

}

static void
wheel_handle_state_accumulating_scroll(struct fallback_dispatch *dispatch,
				       struct evdev_device *device,
				       uint64_t time)
{
	if (abs(dispatch->wheel.hi_res.x) >= ACC_V120_THRESHOLD ||
	    abs(dispatch->wheel.hi_res.y) >= ACC_V120_THRESHOLD) {
		wheel_handle_event(dispatch,
				   WHEEL_EVENT_SCROLL_ACCUMULATED,
				   time);
		wheel_flush_scroll(dispatch, device, time);
	}
}

static void
wheel_handle_state_scrolling(struct fallback_dispatch *dispatch,
			     struct evdev_device *device,
			     uint64_t time)
{
	wheel_flush_scroll(dispatch, device, time);
}

static void
wheel_handle_direction_change(struct fallback_dispatch *dispatch,
			      struct input_event *e,
			      uint64_t time)
{
	enum wheel_direction new_dir = WHEEL_DIR_UNKNOW;

	switch (e->code) {
	case REL_WHEEL_HI_RES:
		new_dir = (e->value > 0) ? WHEEL_DIR_VPOS : WHEEL_DIR_VNEG;
		break;
	case REL_HWHEEL_HI_RES:
		new_dir = (e->value > 0) ? WHEEL_DIR_HPOS : WHEEL_DIR_HNEG;
		break;
	}

	if (new_dir != WHEEL_DIR_UNKNOW && new_dir != dispatch->wheel.dir) {
		dispatch->wheel.dir = new_dir;
		wheel_handle_event(dispatch,
				   WHEEL_EVENT_SCROLL_DIR_CHANGED,
				   time);
	}
}

static void
fallback_rotate_wheel(struct fallback_dispatch *dispatch,
		      struct evdev_device *device,
		      struct input_event *e)
{
	/* Special case: if we're upside down (-ish),
	 * swap the direction of the wheels so that user-down
	 * means scroll down. This isn't done for any other angle
	 * since it's not clear what the heuristics should be.*/
	if (dispatch->rotation.angle >= 160.0 &&
	    dispatch->rotation.angle <= 220.0) {
		e->value *= -1;
	}
}

void
fallback_wheel_process_relative(struct fallback_dispatch *dispatch,
				struct evdev_device *device,
				struct input_event *e, uint64_t time)
{
	switch (e->code) {
	case REL_WHEEL:
		fallback_rotate_wheel(dispatch, device, e);
		dispatch->wheel.lo_res.y += e->value;
		if (dispatch->wheel.emulate_hi_res_wheel)
			dispatch->wheel.hi_res.y += e->value * 120;
		dispatch->pending_event |= EVDEV_WHEEL;
		wheel_handle_event(dispatch, WHEEL_EVENT_SCROLL, time);
		break;
	case REL_HWHEEL:
		fallback_rotate_wheel(dispatch, device, e);
		dispatch->wheel.lo_res.x += e->value;
		if (dispatch->wheel.emulate_hi_res_wheel)
			dispatch->wheel.hi_res.x += e->value * 120;
		dispatch->pending_event |= EVDEV_WHEEL;
		wheel_handle_event(dispatch, WHEEL_EVENT_SCROLL, time);
		break;
	case REL_WHEEL_HI_RES:
		fallback_rotate_wheel(dispatch, device, e);
		dispatch->wheel.hi_res.y += e->value;
		dispatch->wheel.hi_res_event_received = true;
		dispatch->pending_event |= EVDEV_WHEEL;
		wheel_handle_direction_change(dispatch, e, time);
		wheel_handle_event(dispatch, WHEEL_EVENT_SCROLL, time);
		break;
	case REL_HWHEEL_HI_RES:
		fallback_rotate_wheel(dispatch, device, e);
		dispatch->wheel.hi_res.x += e->value;
		dispatch->wheel.hi_res_event_received = true;
		dispatch->pending_event |= EVDEV_WHEEL;
		wheel_handle_direction_change(dispatch, e, time);
		wheel_handle_event(dispatch, WHEEL_EVENT_SCROLL, time);
		break;
	}
}

void
fallback_wheel_handle_state(struct fallback_dispatch *dispatch,
			    struct evdev_device *device,
			    uint64_t time)
{
	if (!(device->seat_caps & EVDEV_DEVICE_POINTER))
		return;

	if (!dispatch->wheel.emulate_hi_res_wheel &&
	    !dispatch->wheel.hi_res_event_received &&
	    (dispatch->wheel.lo_res.x != 0 || dispatch->wheel.lo_res.y != 0)) {
		evdev_log_bug_kernel(device,
				     "device supports high-resolution scroll but only low-resolution events have been received.\n"
				     "See %s/incorrectly-enabled-hires.html for details\n",
				     HTTP_DOC_LINK);
		dispatch->wheel.emulate_hi_res_wheel = true;
		dispatch->wheel.hi_res.x = dispatch->wheel.lo_res.x * 120;
		dispatch->wheel.hi_res.y = dispatch->wheel.lo_res.y * 120;
	}

	switch (dispatch->wheel.state) {
	case WHEEL_STATE_NONE:
		wheel_handle_state_none(dispatch, device, time);
		break;
	case WHEEL_STATE_ACCUMULATING_SCROLL:
		wheel_handle_state_accumulating_scroll(dispatch, device, time);
		break;
	case WHEEL_STATE_SCROLLING:
		wheel_handle_state_scrolling(dispatch, device, time);
		break;
	}
}

static void
wheel_init_scroll_timer(uint64_t now, void *data)
{
	struct evdev_device *device = data;
	struct fallback_dispatch *dispatch =
		fallback_dispatch(device->dispatch);

	wheel_handle_event(dispatch, WHEEL_EVENT_SCROLL_TIMEOUT, now);
}

void
fallback_init_wheel(struct fallback_dispatch *dispatch,
		    struct evdev_device *device)
{
	char timer_name[64];

	dispatch->wheel.state = WHEEL_STATE_NONE;
	dispatch->wheel.dir = WHEEL_DIR_UNKNOW;

	/* On kernel < 5.0 we need to emulate high-resolution
	   wheel scroll events */
	if ((libevdev_has_event_code(device->evdev,
				     EV_REL,
				     REL_WHEEL) &&
	     !libevdev_has_event_code(device->evdev,
				      EV_REL,
				      REL_WHEEL_HI_RES)) ||
	    (libevdev_has_event_code(device->evdev,
				     EV_REL,
				     REL_HWHEEL) &&
	     !libevdev_has_event_code(device->evdev,
				      EV_REL,
				      REL_HWHEEL_HI_RES)))
		dispatch->wheel.emulate_hi_res_wheel = true;

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s wheel scroll",
		 evdev_device_get_sysname(device));
	libinput_timer_init(&dispatch->wheel.scroll_timer,
			    evdev_libinput_context(device),
			    timer_name,
			    wheel_init_scroll_timer,
			    device);
}
