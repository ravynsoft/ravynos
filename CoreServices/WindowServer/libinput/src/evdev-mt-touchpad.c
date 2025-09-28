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

#include "config.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>

#if HAVE_LIBWACOM
#include <libwacom/libwacom.h>
#endif

#include "quirks.h"
#include "evdev-mt-touchpad.h"
#include "util-input-event.h"

#define DEFAULT_TRACKPOINT_ACTIVITY_TIMEOUT ms2us(300)
#define DEFAULT_TRACKPOINT_EVENT_TIMEOUT ms2us(40)
#define DEFAULT_KEYBOARD_ACTIVITY_TIMEOUT_1 ms2us(200)
#define DEFAULT_KEYBOARD_ACTIVITY_TIMEOUT_2 ms2us(500)
#define FAKE_FINGER_OVERFLOW bit(7)
#define THUMB_IGNORE_SPEED_THRESHOLD 20 /* mm/s */

enum notify {
	DONT_NOTIFY,
	DO_NOTIFY,
};

static inline struct tp_history_point*
tp_motion_history_offset(struct tp_touch *t, int offset)
{
	int offset_index =
		(t->history.index - offset + TOUCHPAD_HISTORY_LENGTH) %
		TOUCHPAD_HISTORY_LENGTH;

	return &t->history.samples[offset_index];
}

struct normalized_coords
tp_filter_motion(struct tp_dispatch *tp,
		 const struct device_float_coords *unaccelerated,
		 uint64_t time)
{
	struct device_float_coords raw;
	const struct normalized_coords zero = { 0.0, 0.0 };

	if (device_float_is_zero(*unaccelerated))
		return zero;

	/* Convert to device units with x/y in the same resolution */
	raw = tp_scale_to_xaxis(tp, *unaccelerated);

	return filter_dispatch(tp->device->pointer.filter,
			       &raw, tp, time);
}

struct normalized_coords
tp_filter_motion_unaccelerated(struct tp_dispatch *tp,
			       const struct device_float_coords *unaccelerated,
			       uint64_t time)
{
	struct device_float_coords raw;
	const struct normalized_coords zero = { 0.0, 0.0 };

	if (device_float_is_zero(*unaccelerated))
		return zero;

	/* Convert to device units with x/y in the same resolution */
	raw = tp_scale_to_xaxis(tp, *unaccelerated);

	return filter_dispatch_constant(tp->device->pointer.filter,
					&raw, tp, time);
}

struct normalized_coords
tp_filter_scroll(struct tp_dispatch *tp,
		 const struct device_float_coords *unaccelerated,
		 uint64_t time)
{
	struct device_float_coords raw;
	const struct normalized_coords zero = { 0.0, 0.0 };

	if (device_float_is_zero(*unaccelerated))
		return zero;

	/* Convert to device units with x/y in the same resolution */
	raw = tp_scale_to_xaxis(tp, *unaccelerated);

	return filter_dispatch_scroll(tp->device->pointer.filter,
				      &raw, tp, time);
}

static inline void
tp_calculate_motion_speed(struct tp_dispatch *tp,
			  struct tp_touch *t,
			  uint64_t time)
{
	const struct tp_history_point *last;
	struct device_coords delta;
	struct phys_coords mm;
	double distance;
	double speed;

	/* Don't do this on single-touch or semi-mt devices */
	if (!tp->has_mt || tp->semi_mt)
		return;

	if (t->state != TOUCH_UPDATE)
		return;

	/* This doesn't kick in until we have at least 4 events in the
	 * motion history. As a side-effect, this automatically handles the
	 * 2fg scroll where a finger is down and moving fast before the
	 * other finger comes down for the scroll.
	 *
	 * We do *not* reset the speed to 0 here though. The motion history
	 * is reset whenever a new finger is down, so we'd be resetting the
	 * speed and failing.
	 */
	if (t->history.count < 4)
		return;

	/* TODO: we probably need a speed history here so we can average
	 * across a few events */
	last = tp_motion_history_offset(t, 1);
	delta.x = abs(t->point.x - last->point.x);
	delta.y = abs(t->point.y - last->point.y);
	mm = evdev_device_unit_delta_to_mm(tp->device, &delta);

	distance = length_in_mm(mm);
	speed = distance/(time - last->time); /* mm/us */
	speed *= 1000000; /* mm/s */

	t->speed.last_speed = speed;
}

static inline void
tp_motion_history_push(struct tp_touch *t, uint64_t time)
{
	int motion_index = (t->history.index + 1) % TOUCHPAD_HISTORY_LENGTH;

	if (t->history.count < TOUCHPAD_HISTORY_LENGTH)
		t->history.count++;

	t->history.samples[motion_index].point = t->point;
	t->history.samples[motion_index].time = time;
	t->history.index = motion_index;
}

/* Idea: if we got a tuple of *very* quick moves like {Left, Right,
 * Left}, or {Right, Left, Right}, it means touchpad jitters since no
 * human can move like that within thresholds.
 *
 * We encode left moves as zeroes, and right as ones. We also drop
 * the array to all zeroes when constraints are not satisfied. Then we
 * search for the pattern {1,0,1}. It can't match {Left, Right, Left},
 * but it does match {Left, Right, Left, Right}, so it's okay.
 *
 * This only looks at x changes, y changes are ignored.
 */
static inline void
tp_detect_wobbling(struct tp_dispatch *tp,
		   struct tp_touch *t,
		   uint64_t time)
{
	int dx, dy;
	uint64_t dtime;
	const struct device_coords* prev_point;

	if (tp->nfingers_down != 1 ||
	    tp->nfingers_down != tp->old_nfingers_down)
		return;

	if (tp->hysteresis.enabled || t->history.count == 0)
		return;

	if (!(tp->queued & TOUCHPAD_EVENT_MOTION)) {
		t->hysteresis.x_motion_history = 0;
		return;
	}

	prev_point = &tp_motion_history_offset(t, 0)->point;
	dx = prev_point->x - t->point.x;
	dy = prev_point->y - t->point.y;
	dtime = time - tp->hysteresis.last_motion_time;

	tp->hysteresis.last_motion_time = time;

	if ((dx == 0 && dy != 0) || dtime > ms2us(40)) {
		t->hysteresis.x_motion_history = 0;
		return;
	}

	t->hysteresis.x_motion_history >>= 1;
	if (dx > 0) { /* right move */
		static const char r_l_r = 0x5; /* {Right, Left, Right} */

		t->hysteresis.x_motion_history |= bit(2);
		if (t->hysteresis.x_motion_history == r_l_r) {
			tp->hysteresis.enabled = true;
			evdev_log_debug(tp->device,
					"hysteresis enabled. "
					"See %s/touchpad-jitter.html for details\n",
					HTTP_DOC_LINK);
		}
	}
}

static inline void
tp_motion_hysteresis(struct tp_dispatch *tp,
		     struct tp_touch *t)
{
	if (!tp->hysteresis.enabled)
		return;

	if (t->history.count > 0)
		t->point = evdev_hysteresis(&t->point,
					    &t->hysteresis.center,
					    &tp->hysteresis.margin);

	t->hysteresis.center = t->point;
}

static inline void
tp_motion_history_reset(struct tp_touch *t)
{
	t->history.count = 0;
}

static inline struct tp_touch *
tp_current_touch(struct tp_dispatch *tp)
{
	return &tp->touches[min(tp->slot, tp->ntouches - 1)];
}

static inline struct tp_touch *
tp_get_touch(struct tp_dispatch *tp, unsigned int slot)
{
	assert(slot < tp->ntouches);
	return &tp->touches[slot];
}

static inline unsigned int
tp_fake_finger_count(struct tp_dispatch *tp)
{
	unsigned int fake_touches =
		tp->fake_touches & ~(FAKE_FINGER_OVERFLOW|0x1);

	/* Only one of BTN_TOOL_DOUBLETAP/TRIPLETAP/... may be set at any
	 * time */
	if (fake_touches & (fake_touches - 1))
		evdev_log_bug_kernel(tp->device,
				     "Invalid fake finger state %#x\n",
				     tp->fake_touches);

	if (tp->fake_touches & FAKE_FINGER_OVERFLOW)
		return FAKE_FINGER_OVERFLOW;

	/* don't count BTN_TOUCH */
	return ffs(tp->fake_touches >> 1);
}

static inline bool
tp_fake_finger_is_touching(struct tp_dispatch *tp)
{
	return tp->fake_touches & 0x1;
}

static inline void
tp_fake_finger_set(struct tp_dispatch *tp,
		   unsigned int code,
		   bool is_press)
{
	unsigned int shift;

	switch (code) {
	case BTN_TOUCH:
		if (!is_press)
			tp->fake_touches &= ~FAKE_FINGER_OVERFLOW;
		shift = 0;
		break;
	case BTN_TOOL_FINGER:
		shift = 1;
		break;
	case BTN_TOOL_DOUBLETAP:
	case BTN_TOOL_TRIPLETAP:
	case BTN_TOOL_QUADTAP:
		shift = code - BTN_TOOL_DOUBLETAP + 2;
		break;
	/* when QUINTTAP is released we're either switching to 6 fingers
	   (flag stays in place until BTN_TOUCH is released) or
	   one of DOUBLE/TRIPLE/QUADTAP (will clear the flag on press) */
	case BTN_TOOL_QUINTTAP:
		if (is_press)
			tp->fake_touches |= FAKE_FINGER_OVERFLOW;
		return;
	default:
		return;
	}

	if (is_press) {
		tp->fake_touches &= ~FAKE_FINGER_OVERFLOW;
		tp->fake_touches |= bit(shift);

	} else {
		tp->fake_touches &= ~bit(shift);
	}
}

static inline void
tp_new_touch(struct tp_dispatch *tp, struct tp_touch *t, uint64_t time)
{
	if (t->state == TOUCH_BEGIN ||
	    t->state == TOUCH_UPDATE ||
	    t->state == TOUCH_HOVERING)
		return;

	/* Bug #161: touch ends in the same event frame where it restarts
	   again. That's a kernel bug, so let's complain. */
	if (t->state == TOUCH_MAYBE_END) {
		evdev_log_bug_kernel(tp->device,
				     "touch %d ended and began in in same frame.\n",
				     t->index);
		tp->nfingers_down++;
		t->state = TOUCH_UPDATE;
		t->has_ended = false;
		return;
	}

	/* we begin the touch as hovering because until BTN_TOUCH happens we
	 * don't know if it's a touch down or not. And BTN_TOUCH may happen
	 * after ABS_MT_TRACKING_ID */
	tp_motion_history_reset(t);
	t->dirty = true;
	t->has_ended = false;
	t->was_down = false;
	t->palm.state = PALM_NONE;
	t->state = TOUCH_HOVERING;
	t->pinned.is_pinned = false;
	t->speed.last_speed = 0;
	t->speed.exceeded_count = 0;
	t->hysteresis.x_motion_history = 0;
	tp->queued |= TOUCHPAD_EVENT_MOTION;
}

static inline void
tp_begin_touch(struct tp_dispatch *tp, struct tp_touch *t, uint64_t time)
{
	t->dirty = true;
	t->state = TOUCH_BEGIN;
	t->initial_time = time;
	t->was_down = true;
	tp->nfingers_down++;
	t->palm.time = time;
	t->tap.is_thumb = false;
	t->tap.is_palm = false;
	t->speed.exceeded_count = 0;
	assert(tp->nfingers_down >= 1);
	tp->hysteresis.last_motion_time = time;
}

/**
 * Schedule a touch to be ended, based on either the events or some
 * attributes of the touch (size, pressure). In some cases we need to
 * resurrect a touch that has ended, so this doesn't actually end the touch
 * yet. All the TOUCH_MAYBE_END touches get properly ended once the device
 * state has been processed once and we know how many zombie touches we
 * need.
 */
static inline void
tp_maybe_end_touch(struct tp_dispatch *tp,
		   struct tp_touch *t,
		   uint64_t time)
{
	switch (t->state) {
	case TOUCH_NONE:
	case TOUCH_MAYBE_END:
		return;
	case TOUCH_END:
		evdev_log_bug_libinput(tp->device,
				       "touch %d: already in TOUCH_END\n",
				       t->index);
		return;
	case TOUCH_HOVERING:
	case TOUCH_BEGIN:
	case TOUCH_UPDATE:
		break;
	}

	if (t->state != TOUCH_HOVERING) {
		assert(tp->nfingers_down >= 1);
		tp->nfingers_down--;
		t->state = TOUCH_MAYBE_END;
	} else {
		t->state = TOUCH_NONE;
	}

	t->dirty = true;
}

/**
 * Inverse to tp_maybe_end_touch(), restores a touch back to its previous
 * state.
 */
static inline void
tp_recover_ended_touch(struct tp_dispatch *tp,
		       struct tp_touch *t)
{
	t->dirty = true;
	t->state = TOUCH_UPDATE;
	tp->nfingers_down++;
}

/**
 * End a touch, even if the touch sequence is still active.
 * Use tp_maybe_end_touch() instead.
 */
static inline void
tp_end_touch(struct tp_dispatch *tp, struct tp_touch *t, uint64_t time)
{
	if (t->state != TOUCH_MAYBE_END) {
		evdev_log_bug_libinput(tp->device,
				       "touch %d should be MAYBE_END, is %d\n",
				       t->index,
				       t->state);
		return;
	}

	t->dirty = true;
	t->palm.state = PALM_NONE;
	t->state = TOUCH_END;
	t->pinned.is_pinned = false;
	t->palm.time = 0;
	t->speed.exceeded_count = 0;
	tp->queued |= TOUCHPAD_EVENT_MOTION;
}

/**
 * End the touch sequence on ABS_MT_TRACKING_ID -1 or when the BTN_TOOL_* 0 is received.
 */
static inline void
tp_end_sequence(struct tp_dispatch *tp, struct tp_touch *t, uint64_t time)
{
	t->has_ended = true;
	tp_maybe_end_touch(tp, t, time);
}

static void
tp_stop_actions(struct tp_dispatch *tp, uint64_t time)
{
	tp_edge_scroll_stop_events(tp, time);
	tp_gesture_cancel(tp, time);
	tp_tap_suspend(tp, time);
}

struct device_coords
tp_get_delta(struct tp_touch *t)
{
	struct device_coords delta;
	const struct device_coords zero = { 0.0, 0.0 };

	if (t->history.count <= 1)
		return zero;

	delta.x = tp_motion_history_offset(t, 0)->point.x -
		  tp_motion_history_offset(t, 1)->point.x;
	delta.y = tp_motion_history_offset(t, 0)->point.y -
		  tp_motion_history_offset(t, 1)->point.y;

	return delta;
}

static inline int32_t
rotated(struct tp_dispatch *tp, unsigned int code, int value)
{
	const struct input_absinfo *absinfo;

	if (!tp->left_handed.rotate)
		return value;

	switch (code) {
	case ABS_X:
	case ABS_MT_POSITION_X:
		absinfo = tp->device->abs.absinfo_x;
		break;
	case ABS_Y:
	case ABS_MT_POSITION_Y:
		absinfo = tp->device->abs.absinfo_y;
		break;
	default:
		abort();
	}
	return absinfo->maximum - (value - absinfo->minimum);
}

static void
tp_process_absolute(struct tp_dispatch *tp,
		    const struct input_event *e,
		    uint64_t time)
{
	struct tp_touch *t = tp_current_touch(tp);

	switch(e->code) {
	case ABS_MT_POSITION_X:
		evdev_device_check_abs_axis_range(tp->device,
						  e->code,
						  e->value);
		t->point.x = rotated(tp, e->code, e->value);
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_MOTION;
		break;
	case ABS_MT_POSITION_Y:
		evdev_device_check_abs_axis_range(tp->device,
						  e->code,
						  e->value);
		t->point.y = rotated(tp, e->code, e->value);
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_MOTION;
		break;
	case ABS_MT_SLOT:
		tp->slot = e->value;
		break;
	case ABS_MT_TRACKING_ID:
		if (e->value != -1) {
			tp->nactive_slots += 1;
			tp_new_touch(tp, t, time);
		} else {
			assert(tp->nactive_slots >= 1);
			tp->nactive_slots -= 1;
			tp_end_sequence(tp, t, time);
		}
		break;
	case ABS_MT_PRESSURE:
		t->pressure = e->value;
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_OTHERAXIS;
		break;
	case ABS_MT_TOOL_TYPE:
		t->is_tool_palm = e->value == MT_TOOL_PALM;
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_OTHERAXIS;
		break;
	case ABS_MT_TOUCH_MAJOR:
		t->major = e->value;
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_OTHERAXIS;
		break;
	case ABS_MT_TOUCH_MINOR:
		t->minor = e->value;
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_OTHERAXIS;
		break;
	}
}

static void
tp_process_absolute_st(struct tp_dispatch *tp,
		       const struct input_event *e,
		       uint64_t time)
{
	struct tp_touch *t = tp_current_touch(tp);

	switch(e->code) {
	case ABS_X:
		evdev_device_check_abs_axis_range(tp->device,
						  e->code,
						  e->value);
		t->point.x = rotated(tp, e->code, e->value);
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_MOTION;
		break;
	case ABS_Y:
		evdev_device_check_abs_axis_range(tp->device,
						  e->code,
						  e->value);
		t->point.y = rotated(tp, e->code, e->value);
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_MOTION;
		break;
	case ABS_PRESSURE:
		t->pressure = e->value;
		t->dirty = true;
		tp->queued |= TOUCHPAD_EVENT_OTHERAXIS;
		break;
	}
}

static inline void
tp_restore_synaptics_touches(struct tp_dispatch *tp,
			     uint64_t time)
{
	unsigned int i;
	unsigned int nfake_touches;

	nfake_touches = tp_fake_finger_count(tp);
	if (nfake_touches < 3)
		return;

	if (tp->nfingers_down >= nfake_touches ||
	    (tp->nfingers_down == tp->num_slots && nfake_touches == tp->num_slots))
		return;

	/* Synaptics devices may end touch 2 on transition to/from
	 * BTN_TOOL_TRIPLETAP and start it again on the next frame with
	 * different coordinates (bz#91352, gitlab#434). We search the
	 * touches we have, if there is one that has just ended despite us
	 * being on tripletap, we move it back to update.
	 *
	 * Note: we only handle the transition from 2 to 3 touches, not the
	 * other way round (see gitlab#434)
	 */
	for (i = 0; i < tp->num_slots; i++) {
		struct tp_touch *t = tp_get_touch(tp, i);

		if (t->state != TOUCH_MAYBE_END)
			continue;

		/* new touch, move it through begin to update immediately */
		tp_recover_ended_touch(tp, t);
	}
}

static void
tp_process_fake_touches(struct tp_dispatch *tp,
			uint64_t time)
{
	struct tp_touch *t;
	unsigned int nfake_touches;
	unsigned int i, start;

	nfake_touches = tp_fake_finger_count(tp);
	if (nfake_touches == FAKE_FINGER_OVERFLOW)
		return;

	if (tp->device->model_flags &
	    EVDEV_MODEL_SYNAPTICS_SERIAL_TOUCHPAD)
		tp_restore_synaptics_touches(tp, time);

	/* ALPS serial touchpads always set 3 slots in the kernel, even
	 * where they support less than that. So we get BTN_TOOL_TRIPLETAP
	 * but never slot 2 because our slot count is wrong.
	 * This also means that the third touch falls through the cracks and
	 * is ignored.
	 *
	 * See https://gitlab.freedesktop.org/libinput/libinput/issues/408
	 *
	 * All touchpad devices have at least one slot so we only do this
	 * for 2 touches or higher.
	 *
	 * There's an bug in libevdev < 1.9.0 affecting slots after a
	 * SYN_DROPPED. Where a user release one or more touches during
	 * SYN_DROPPED and places new ones on the touchpad, we may end up
	 * with fake touches but no active slots.
	 * So let's check for nactive_slots > 0 to make sure we don't lose
	 * all fingers. That's a workaround only, this must be fixed in
	 * libevdev.
	 *
	 * For a long explanation of what happens, see
	 * https://gitlab.freedesktop.org/libevdev/libevdev/merge_requests/19
	 */
	if (tp->device->model_flags & EVDEV_MODEL_ALPS_SERIAL_TOUCHPAD &&
	    nfake_touches > 1 && tp->has_mt &&
	    tp->nactive_slots > 0 &&
	    nfake_touches > tp->nactive_slots &&
	    tp->nactive_slots < tp->num_slots) {
		evdev_log_bug_kernel(tp->device,
				     "Wrong slot count (%d), reducing to %d\n",
				     tp->num_slots,
				     tp->nactive_slots);
		/* This should be safe since we fill the slots from the
		 * first one so hiding the excessive slots shouldn't matter.
		 * There are sequences where we could accidentally lose an
		 * actual touch point but that requires specially crafted
		 * sequences and let's deal with that when it happens.
		 */
		tp->num_slots = tp->nactive_slots;
	}

	start = tp->has_mt ? tp->num_slots : 0;
	for (i = start; i < tp->ntouches; i++) {
		t = tp_get_touch(tp, i);
		if (i < nfake_touches)
			tp_new_touch(tp, t, time);
		else
			tp_end_sequence(tp, t, time);
	}
}

static void
tp_process_trackpoint_button(struct tp_dispatch *tp,
			     const struct input_event *e,
			     uint64_t time)
{
	struct evdev_dispatch *dispatch;
	struct input_event event;
	struct input_event syn_report = {
		 .input_event_sec = 0,
		 .input_event_usec = 0,
		 .type = EV_SYN,
		 .code = SYN_REPORT,
		 .value = 0
	};

	if (!tp->buttons.trackpoint)
		return;

	dispatch = tp->buttons.trackpoint->dispatch;

	event = *e;
	syn_report.input_event_sec = e->input_event_sec;
	syn_report.input_event_usec = e->input_event_usec;

	switch (event.code) {
	case BTN_0:
		event.code = BTN_LEFT;
		break;
	case BTN_1:
		event.code = BTN_RIGHT;
		break;
	case BTN_2:
		event.code = BTN_MIDDLE;
		break;
	default:
		return;
	}

	dispatch->interface->process(dispatch,
				     tp->buttons.trackpoint,
				     &event, time);
	dispatch->interface->process(dispatch,
				     tp->buttons.trackpoint,
				     &syn_report, time);
}

static void
tp_process_key(struct tp_dispatch *tp,
	       const struct input_event *e,
	       uint64_t time)
{
	/* ignore kernel key repeat */
	if (e->value == 2)
		return;

	switch (e->code) {
		case BTN_LEFT:
		case BTN_MIDDLE:
		case BTN_RIGHT:
			tp_process_button(tp, e, time);
			break;
		case BTN_TOUCH:
		case BTN_TOOL_FINGER:
		case BTN_TOOL_DOUBLETAP:
		case BTN_TOOL_TRIPLETAP:
		case BTN_TOOL_QUADTAP:
		case BTN_TOOL_QUINTTAP:
			tp_fake_finger_set(tp, e->code, !!e->value);
			break;
		case BTN_0:
		case BTN_1:
		case BTN_2:
			tp_process_trackpoint_button(tp, e, time);
			break;
	}
}

static void
tp_process_msc(struct tp_dispatch *tp,
	       const struct input_event *e,
	       uint64_t time)
{
	if (e->code != MSC_TIMESTAMP)
		return;

	tp->quirks.msc_timestamp.now = e->value;
	tp->queued |= TOUCHPAD_EVENT_TIMESTAMP;
}

static void
tp_unpin_finger(const struct tp_dispatch *tp, struct tp_touch *t)
{
	struct phys_coords mm;
	struct device_coords delta;

	if (!t->pinned.is_pinned)
		return;

	delta.x = abs(t->point.x - t->pinned.center.x);
	delta.y = abs(t->point.y - t->pinned.center.y);

	mm = evdev_device_unit_delta_to_mm(tp->device, &delta);

	/* 1.5mm movement -> unpin */
	if (hypot(mm.x, mm.y) >= 1.5) {
		t->pinned.is_pinned = false;
		return;
	}
}

static void
tp_pin_fingers(struct tp_dispatch *tp)
{
	struct tp_touch *t;

	tp_for_each_touch(tp, t) {
		t->pinned.is_pinned = true;
		t->pinned.center = t->point;
	}
}

bool
tp_touch_active(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return (t->state == TOUCH_BEGIN || t->state == TOUCH_UPDATE) &&
		t->palm.state == PALM_NONE &&
		!t->pinned.is_pinned &&
		!tp_thumb_ignored(tp, t) &&
		tp_button_touch_active(tp, t) &&
		tp_edge_scroll_touch_active(tp, t);
}

bool
tp_touch_active_for_gesture(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return (t->state == TOUCH_BEGIN || t->state == TOUCH_UPDATE) &&
		t->palm.state == PALM_NONE &&
		!t->pinned.is_pinned &&
		!tp_thumb_ignored_for_gesture(tp, t) &&
		tp_button_touch_active(tp, t) &&
		tp_edge_scroll_touch_active(tp, t);
}

static inline bool
tp_palm_was_in_side_edge(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return t->palm.first.x < tp->palm.left_edge ||
	       t->palm.first.x > tp->palm.right_edge;
}

static inline bool
tp_palm_was_in_top_edge(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return t->palm.first.y < tp->palm.upper_edge;
}

static inline bool
tp_palm_in_side_edge(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return t->point.x < tp->palm.left_edge ||
	       t->point.x > tp->palm.right_edge;
}

static inline bool
tp_palm_in_top_edge(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return t->point.y < tp->palm.upper_edge;
}

static inline bool
tp_palm_in_edge(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return tp_palm_in_side_edge(tp, t) || tp_palm_in_top_edge(tp, t);
}

static bool
tp_palm_detect_dwt_triggered(struct tp_dispatch *tp,
			     struct tp_touch *t,
			     uint64_t time)
{
	if (tp->dwt.dwt_enabled &&
	    tp->dwt.keyboard_active &&
	    t->state == TOUCH_BEGIN) {
		t->palm.state = PALM_TYPING;
		t->palm.first = t->point;
		return true;
	}

	if (!tp->dwt.keyboard_active &&
		   t->state == TOUCH_UPDATE &&
		   t->palm.state == PALM_TYPING) {
		/* If a touch has started before the first or after the last
		   key press, release it on timeout. Benefit: a palm rested
		   while typing on the touchpad will be ignored, but a touch
		   started once we stop typing will be able to control the
		   pointer (alas not tap, etc.).
		   */
		if (t->palm.time == 0 ||
		    t->palm.time > tp->dwt.keyboard_last_press_time) {
			t->palm.state = PALM_NONE;
			evdev_log_debug(tp->device,
					"palm: touch %d released, timeout after typing\n",
					t->index);
		}
	}

	return false;
}

static bool
tp_palm_detect_trackpoint_triggered(struct tp_dispatch *tp,
				    struct tp_touch *t,
				    uint64_t time)
{
	if (!tp->palm.monitor_trackpoint)
		return false;

	if (t->palm.state == PALM_NONE &&
	    t->state == TOUCH_BEGIN &&
	    tp->palm.trackpoint_active) {
		t->palm.state = PALM_TRACKPOINT;
		return true;
	}

	if (t->palm.state == PALM_TRACKPOINT &&
		   t->state == TOUCH_UPDATE &&
		   !tp->palm.trackpoint_active) {

		if (t->palm.time == 0 ||
		    t->palm.time > tp->palm.trackpoint_last_event_time) {
			t->palm.state = PALM_NONE;
			evdev_log_debug(tp->device,
				       "palm: touch %d released, timeout after trackpoint\n", t->index);
		}
	}

	return false;
}

static bool
tp_palm_detect_tool_triggered(struct tp_dispatch *tp,
			      struct tp_touch *t,
			      uint64_t time)
{
	if (!tp->palm.use_mt_tool)
		return false;

	if (t->palm.state != PALM_NONE &&
	    t->palm.state != PALM_TOOL_PALM)
		return false;

	if (t->palm.state == PALM_NONE &&
	    t->is_tool_palm)
		t->palm.state = PALM_TOOL_PALM;
	else if (t->palm.state == PALM_TOOL_PALM &&
		 !t->is_tool_palm)
		t->palm.state = PALM_NONE;

	return t->palm.state == PALM_TOOL_PALM;
}

static inline bool
tp_palm_detect_move_out_of_edge(struct tp_dispatch *tp,
				struct tp_touch *t,
				uint64_t time)
{
	const int PALM_TIMEOUT = ms2us(200);
	int directions = 0;
	struct device_float_coords delta;
	int dirs;

	if (time < t->palm.time + PALM_TIMEOUT && !tp_palm_in_edge(tp, t)) {
		if (tp_palm_was_in_side_edge(tp, t))
			directions = NE|E|SE|SW|W|NW;
		else if (tp_palm_was_in_top_edge(tp, t))
			directions = S|SE|SW;

		if (directions) {
			delta = device_delta(t->point, t->palm.first);
			dirs = phys_get_direction(tp_phys_delta(tp, delta));
			if ((dirs & directions) && !(dirs & ~directions))
				return true;
		}
	}

	return false;
}

static inline bool
tp_palm_detect_multifinger(struct tp_dispatch *tp, struct tp_touch *t, uint64_t time)
{
	struct tp_touch *other;

	if (tp->nfingers_down < 2)
		return false;

	/* If we have at least one other active non-palm touch make this
	 * touch non-palm too. This avoids palm detection during two-finger
	 * scrolling.
	 *
	 * Note: if both touches start in the palm zone within the same
	 * frame the second touch will still be PALM_NONE and thus detected
	 * here as non-palm touch. This is too niche to worry about for now.
	 */
	tp_for_each_touch(tp, other) {
		if (other == t)
			continue;

		if (tp_touch_active(tp, other) &&
		    other->palm.state == PALM_NONE) {
			return true;
		}
	}

	return false;
}

static inline bool
tp_palm_detect_touch_size_triggered(struct tp_dispatch *tp,
				    struct tp_touch *t,
				    uint64_t time)
{
	if (!tp->palm.use_size)
		return false;

	/* If a finger size is large enough for palm, we stick with that and
	 * force the user to release and reset the finger */
	if (t->palm.state != PALM_NONE && t->palm.state != PALM_TOUCH_SIZE)
		return false;

	if (t->major > tp->palm.size_threshold ||
	    t->minor > tp->palm.size_threshold) {
		if (t->palm.state != PALM_TOUCH_SIZE)
			evdev_log_debug(tp->device,
					"palm: touch %d size exceeded\n",
					t->index);
		t->palm.state = PALM_TOUCH_SIZE;
		return true;
	}

	return false;
}

static inline bool
tp_palm_detect_edge(struct tp_dispatch *tp,
		    struct tp_touch *t,
		    uint64_t time)
{
	if (t->palm.state == PALM_EDGE) {
		if (tp_palm_detect_multifinger(tp, t, time)) {
			t->palm.state = PALM_NONE;
			evdev_log_debug(tp->device,
				  "palm: touch %d released, multiple fingers\n",
				  t->index);

		/* If labelled a touch as palm, we unlabel as palm when
		   we move out of the palm edge zone within the timeout, provided
		   the direction is within 45 degrees of the horizontal.
		 */
		} else if (tp_palm_detect_move_out_of_edge(tp, t, time)) {
			t->palm.state = PALM_NONE;
			evdev_log_debug(tp->device,
				  "palm: touch %d released, out of edge zone\n",
				  t->index);
		}
		return false;
	}

	if (tp_palm_detect_multifinger(tp, t, time)) {
		return false;
	}

	/* palm must start in exclusion zone, it's ok to move into
	   the zone without being a palm */
	if (t->state != TOUCH_BEGIN || !tp_palm_in_edge(tp, t))
		return false;

	if (tp_touch_get_edge(tp, t) & EDGE_RIGHT)
		return false;

	t->palm.state = PALM_EDGE;
	t->palm.time = time;
	t->palm.first = t->point;

	return true;
}

static bool
tp_palm_detect_pressure_triggered(struct tp_dispatch *tp,
				  struct tp_touch *t,
				  uint64_t time)
{
	if (!tp->palm.use_pressure)
		return false;

	if (t->palm.state != PALM_NONE &&
	    t->palm.state != PALM_PRESSURE)
		return false;

	if (t->pressure > tp->palm.pressure_threshold)
		t->palm.state = PALM_PRESSURE;

	return t->palm.state == PALM_PRESSURE;
}

static bool
tp_palm_detect_arbitration_triggered(struct tp_dispatch *tp,
				     struct tp_touch *t,
				     uint64_t time)
{
	if (tp->arbitration.state == ARBITRATION_NOT_ACTIVE)
		return false;

	t->palm.state = PALM_ARBITRATION;

	return true;
}

static void
tp_palm_detect(struct tp_dispatch *tp, struct tp_touch *t, uint64_t time)
{
	const char *palm_state;
	enum touch_palm_state oldstate = t->palm.state;

	if (tp_palm_detect_pressure_triggered(tp, t, time))
		goto out;

	if (tp_palm_detect_arbitration_triggered(tp, t, time))
		goto out;

	if (tp_palm_detect_dwt_triggered(tp, t, time))
		goto out;

	if (tp_palm_detect_trackpoint_triggered(tp, t, time))
		goto out;

	if (tp_palm_detect_tool_triggered(tp, t, time))
		goto out;

	if (tp_palm_detect_touch_size_triggered(tp, t, time))
		goto out;

	if (tp_palm_detect_edge(tp, t, time))
		goto out;

	/* Pressure is highest priority because it cannot be released and
	 * overrides all other checks. So we check once before anything else
	 * in case pressure triggers on a non-palm touch. And again after
	 * everything in case one of the others released but we have a
	 * pressure trigger now.
	 */
	if (tp_palm_detect_pressure_triggered(tp, t, time))
		goto out;

	return;
out:

	if (oldstate == t->palm.state)
		return;

	switch (t->palm.state) {
	case PALM_EDGE:
		palm_state = "edge";
		break;
	case PALM_TYPING:
		palm_state = "typing";
		break;
	case PALM_TRACKPOINT:
		palm_state = "trackpoint";
		break;
	case PALM_TOOL_PALM:
		palm_state = "tool-palm";
		break;
	case PALM_PRESSURE:
		palm_state = "pressure";
		break;
	case PALM_TOUCH_SIZE:
		palm_state = "touch size";
		break;
	case PALM_ARBITRATION:
		palm_state = "arbitration";
		break;
	case PALM_NONE:
	default:
		abort();
		break;
	}
	evdev_log_debug(tp->device,
		  "palm: touch %d (%s), palm detected (%s)\n",
		  t->index,
		  touch_state_to_str(t->state),
		  palm_state);
}

static void
tp_unhover_pressure(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;
	int i;
	unsigned int nfake_touches;
	unsigned int real_fingers_down = 0;

	nfake_touches = tp_fake_finger_count(tp);
	if (nfake_touches == FAKE_FINGER_OVERFLOW)
		nfake_touches = 0;

	for (i = 0; i < (int)tp->num_slots; i++) {
		t = tp_get_touch(tp, i);

		if (t->state == TOUCH_NONE)
			continue;

		if (t->dirty) {
			if (t->state == TOUCH_HOVERING) {
				if (t->pressure >= tp->pressure.high) {
					evdev_log_debug(tp->device,
							"pressure: begin touch %d\n",
							t->index);
					/* avoid jumps when landing a finger */
					tp_motion_history_reset(t);
					tp_begin_touch(tp, t, time);
				}
			/* don't unhover for pressure if we have too many
			 * fake fingers down, see comment below. Except
			 * for single-finger touches where the real touch
			 * decides for the rest.
			 */
			} else if (nfake_touches <= tp->num_slots ||
				   tp->num_slots == 1) {
				if (t->pressure < tp->pressure.low) {
					evdev_log_debug(tp->device,
							"pressure: end touch %d\n",
							t->index);
					tp_maybe_end_touch(tp, t, time);
				}
			}
		}

		if (t->state == TOUCH_BEGIN ||
		    t->state == TOUCH_UPDATE)
			real_fingers_down++;
	}

	if (nfake_touches <= tp->num_slots ||
	    tp->nfingers_down == 0)
		return;

	/* if we have more fake fingers down than slots, we assume
	 * _all_ fingers have enough pressure, even if some of the slotted
	 * ones don't. Anything else gets insane quickly.
	 */
	if (real_fingers_down > 0) {
		tp_for_each_touch(tp, t) {
			if (t->state == TOUCH_HOVERING) {
				/* avoid jumps when landing a finger */
				tp_motion_history_reset(t);
				tp_begin_touch(tp, t, time);

				if (tp->nfingers_down >= nfake_touches)
					break;
			}
		}
	}

	if (tp->nfingers_down > nfake_touches ||
	    real_fingers_down == 0) {
		for (i = tp->ntouches - 1; i >= 0; i--) {
			t = tp_get_touch(tp, i);

			if (t->state == TOUCH_HOVERING ||
			    t->state == TOUCH_NONE ||
			    t->state == TOUCH_MAYBE_END)
				continue;

			tp_maybe_end_touch(tp, t, time);

			if (real_fingers_down > 0  &&
			    tp->nfingers_down == nfake_touches)
				break;
		}
	}
}

static void
tp_unhover_size(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;
	int low = tp->touch_size.low,
	    high = tp->touch_size.high;
	int i;

	/* We require 5 slots for size handling, so we don't need to care
	 * about fake touches here */

	for (i = 0; i < (int)tp->num_slots; i++) {
		t = tp_get_touch(tp, i);

		if (t->state == TOUCH_NONE)
			continue;

		if (!t->dirty)
			continue;

		if (t->state == TOUCH_HOVERING) {
			if ((t->major > high && t->minor > low) ||
			    (t->major > low && t->minor > high)) {
				evdev_log_debug(tp->device,
						"touch-size: begin touch %d\n",
						t->index);
				/* avoid jumps when landing a finger */
				tp_motion_history_reset(t);
				tp_begin_touch(tp, t, time);
			}
		} else {
			if (t->major < low || t->minor < low) {
				evdev_log_debug(tp->device,
						"touch-size: end touch %d\n",
						t->index);
				tp_maybe_end_touch(tp, t, time);
			}
		}
	}
}

static void
tp_unhover_fake_touches(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;
	unsigned int nfake_touches;
	int i;

	if (!tp->fake_touches && !tp->nfingers_down)
		return;

	nfake_touches = tp_fake_finger_count(tp);
	if (nfake_touches == FAKE_FINGER_OVERFLOW)
		return;

	if (tp->nfingers_down == nfake_touches &&
	    ((tp->nfingers_down == 0 && !tp_fake_finger_is_touching(tp)) ||
	     (tp->nfingers_down > 0 && tp_fake_finger_is_touching(tp))))
		return;

	/* if BTN_TOUCH is set and we have less fingers down than fake
	 * touches, switch each hovering touch to BEGIN
	 * until nfingers_down matches nfake_touches
	 */
	if (tp_fake_finger_is_touching(tp) &&
	    tp->nfingers_down < nfake_touches) {
		tp_for_each_touch(tp, t) {
			if (t->state == TOUCH_HOVERING) {
				tp_begin_touch(tp, t, time);

				if (tp->nfingers_down >= nfake_touches)
					break;
			}
		}
	}

	/* if BTN_TOUCH is unset end all touches, we're hovering now. If we
	 * have too many touches also end some of them. This is done in
	 * reverse order.
	 */
	if (tp->nfingers_down > nfake_touches ||
	    !tp_fake_finger_is_touching(tp)) {
		for (i = tp->ntouches - 1; i >= 0; i--) {
			t = tp_get_touch(tp, i);

			if (t->state == TOUCH_HOVERING ||
			    t->state == TOUCH_NONE)
				continue;

			tp_maybe_end_touch(tp, t, time);

			if (tp_fake_finger_is_touching(tp) &&
			    tp->nfingers_down == nfake_touches)
				break;
		}
	}
}

static void
tp_unhover_touches(struct tp_dispatch *tp, uint64_t time)
{
	if (tp->pressure.use_pressure)
		tp_unhover_pressure(tp, time);
	else if (tp->touch_size.use_touch_size)
		tp_unhover_size(tp, time);
	else
		tp_unhover_fake_touches(tp, time);

}

static inline void
tp_position_fake_touches(struct tp_dispatch *tp)
{
	struct tp_touch *t;
	struct tp_touch *topmost = NULL;
	unsigned int start, i;

	if (tp_fake_finger_count(tp) <= tp->num_slots ||
	    tp->nfingers_down == 0)
		return;

	/* We have at least one fake touch down. Find the top-most real
	 * touch and copy its coordinates over to to all fake touches.
	 * This is more reliable than just taking the first touch.
	 */
	for (i = 0; i < tp->num_slots; i++) {
		t = tp_get_touch(tp, i);
		if (t->state == TOUCH_END ||
		    t->state == TOUCH_NONE)
			continue;

		if (topmost == NULL || t->point.y < topmost->point.y)
			topmost = t;
	}

	if (!topmost) {
		evdev_log_bug_libinput(tp->device,
				       "Unable to find topmost touch\n");
		return;
	}

	start = tp->has_mt ? tp->num_slots : 1;
	for (i = start; i < tp->ntouches; i++) {
		t = tp_get_touch(tp, i);
		if (t->state == TOUCH_NONE)
			continue;

		t->point = topmost->point;
		t->pressure = topmost->pressure;
		if (!t->dirty)
			t->dirty = topmost->dirty;
	}
}

static inline bool
tp_need_motion_history_reset(struct tp_dispatch *tp)
{
	bool rc = false;

	/* Changing the numbers of fingers can cause a jump in the
	 * coordinates, always reset the motion history for all touches when
	 * that happens.
	 */
	if (tp->nfingers_down != tp->old_nfingers_down)
		return true;

	/* Quirk: if we had multiple events without x/y axis
	   information, the next x/y event is going to be a jump. So we
	   reset that touch to non-dirty effectively swallowing that event
	   and restarting with the next event again.
	 */
	if (tp->device->model_flags & EVDEV_MODEL_LENOVO_T450_TOUCHPAD) {
		if (tp->queued & TOUCHPAD_EVENT_MOTION) {
			if (tp->quirks.nonmotion_event_count > 10) {
				tp->queued &= ~TOUCHPAD_EVENT_MOTION;
				rc = true;
			}
			tp->quirks.nonmotion_event_count = 0;
		}

		if ((tp->queued & (TOUCHPAD_EVENT_OTHERAXIS|TOUCHPAD_EVENT_MOTION)) ==
		    TOUCHPAD_EVENT_OTHERAXIS)
			tp->quirks.nonmotion_event_count++;
	}

	return rc;
}

static bool
tp_detect_jumps(const struct tp_dispatch *tp,
		struct tp_touch *t,
		uint64_t time)
{
	struct device_coords delta;
	struct phys_coords mm;
	struct tp_history_point *last;
	double abs_distance, rel_distance;
	bool is_jump = false;
	uint64_t tdelta;
	/* Reference interval from the touchpad the various thresholds
	 * were measured from */
	unsigned int reference_interval = ms2us(12);

	/* On some touchpads the firmware does funky stuff and we cannot
	 * have our own jump detection, e.g. Lenovo Carbon X1 Gen 6 (see
	 * issue #506)
	 */
	if (tp->jump.detection_disabled)
		return false;

	/* We haven't seen pointer jumps on Wacom tablets yet, so exclude
	 * those.
	 */
	if (tp->device->model_flags & EVDEV_MODEL_WACOM_TOUCHPAD)
		return false;

	if (t->history.count == 0) {
		t->jumps.last_delta_mm = 0.0;
		return false;
	}

	/* called before tp_motion_history_push, so offset 0 is the most
	 * recent coordinate */
	last = tp_motion_history_offset(t, 0);
	tdelta = time - last->time;

	/* For test devices we always force the time delta to 12, at least
	   until the test suite actually does proper intervals. */
	if (tp->device->model_flags & EVDEV_MODEL_TEST_DEVICE)
		reference_interval = tdelta;

	/* If the last frame is more than 30ms ago, we have irregular
	 * frames, who knows what's a pointer jump here and what's
	 * legitimate movement.... */
	if (tdelta > 2.5 * reference_interval || tdelta == 0)
		return false;

	/* We historically expected ~12ms frame intervals, so the numbers
	   below are normalized to that (and that's also where the
	   measured data came from) */
	delta.x = abs(t->point.x - last->point.x);
	delta.y = abs(t->point.y - last->point.y);
	mm = evdev_device_unit_delta_to_mm(tp->device, &delta);
	abs_distance = hypot(mm.x, mm.y) * reference_interval/tdelta;
	rel_distance = abs_distance - t->jumps.last_delta_mm;

	/* Special case for the ALPS devices in the Lenovo ThinkPad E465,
	 * E550. These devices send occasional 4095/0 events on two fingers
	 * before snapping back to the correct position.
	 * https://gitlab.freedesktop.org/libinput/libinput/-/issues/492
	 * The specific values are hardcoded here, if this ever happens on
	 * any other device we can make it absmax/absmin instead.
	 */
	if (tp->device->model_flags & EVDEV_MODEL_ALPS_SERIAL_TOUCHPAD &&
	    t->point.x == 4095 && t->point.y == 0) {
		t->point = last->point;
		return true;
	}

	/* Cursor jump if:
	 * - current single-event delta is >20mm, or
	 * - we increased the delta by over 7mm within a 12ms frame.
	 *   (12ms simply because that's what I measured)
	 */
	is_jump = abs_distance > 20.0 || rel_distance > 7;
	t->jumps.last_delta_mm = abs_distance;

	return is_jump;
}

/**
 * Rewrite the motion history so that previous points' timestamps are the
 * current point's timestamp minus whatever MSC_TIMESTAMP gives us.
 *
 * This must be called before tp_motion_history_push()
 *
 * @param t The touch point
 * @param jumping_interval The large time interval in µs
 * @param normal_interval Normal hw interval in µs
 * @param time Current time in µs
 */
static inline void
tp_motion_history_fix_last(struct tp_dispatch *tp,
			   struct tp_touch *t,
			   unsigned int jumping_interval,
			   unsigned int normal_interval,
			   uint64_t time)
{
	if (t->state != TOUCH_UPDATE)
		return;

	/* We know the coordinates are correct because the touchpad should
	 * get that bit right. But the timestamps we got from the kernel are
	 * messed up, so we go back in the history and fix them.
	 *
	 * This way the next delta is huge but it's over a large time, so
	 * the pointer accel code should do the right thing.
	 */
	for (int i = 0; i < (int)t->history.count; i++) {
		struct tp_history_point *p;

		p = tp_motion_history_offset(t, i);
		p->time = time - jumping_interval - normal_interval * i;
	}
}

static void
tp_process_msc_timestamp(struct tp_dispatch *tp, uint64_t time)
{
	struct msc_timestamp *m = &tp->quirks.msc_timestamp;

	/* Pointer jump detection based on MSC_TIMESTAMP.

	   MSC_TIMESTAMP gets reset after a kernel timeout (1s) and on some
	   devices (Dell XPS) the i2c controller sleeps after a timeout. On
	   wakeup, some events are swallowed, triggering a cursor jump. The
	   event sequence after a sleep is always:

	   initial finger down:
		   ABS_X/Y	x/y
		   MSC_TIMESTAMP 0
		   SYN_REPORT +2500ms
	   second event:
		   ABS_X/Y	x+n/y+n        # normal movement
		   MSC_TIMESTAMP 7300          # the hw interval
		   SYN_REPORT +2ms
	   third event:
		   ABS_X/Y	x+lots/y+lots  # pointer jump!
		   MSC_TIMESTAMP 123456        # well above the hw interval
		   SYN_REPORT +2ms
	   fourth event:
		   ABS_X/Y	x+lots+n/y+lots+n  # all normal again
		   MSC_TIMESTAMP 123456 + 7300
		   SYN_REPORT +8ms

	   Our approach is to detect the 0 timestamp, check the interval on
	   the next event and then calculate the movement for one fictitious
	   event instead, swallowing all other movements. So if the time
	   delta is equivalent to 10 events and the movement is x, we
	   instead pretend there was movement of x/10.
	 */
	if (m->now == 0) {
		m->state = JUMP_STATE_EXPECT_FIRST;
		m->interval = 0;
		return;
	}

	switch(m->state) {
	case JUMP_STATE_EXPECT_FIRST:
		if (m->now > ms2us(20)) {
			m->state = JUMP_STATE_IGNORE;
		} else {
			m->state = JUMP_STATE_EXPECT_DELAY;
			m->interval = m->now;
		}
		break;
	case JUMP_STATE_EXPECT_DELAY:
		if (m->now > m->interval * 2) {
			uint32_t tdelta; /* µs */
			struct tp_touch *t;

			/* The current time is > 2 times the interval so we
			 * have a jump. Fix the motion history */
			tdelta = m->now - m->interval;

			tp_for_each_touch(tp, t) {
				tp_motion_history_fix_last(tp,
							   t,
							   tdelta,
							   m->interval,
							   time);
			}
			m->state = JUMP_STATE_IGNORE;

			/* We need to restart the acceleration filter to forget its history.
			 * The current point becomes the first point in the history there
			 * (including timestamp) and that accelerates correctly.
			 * This has a potential to be incorrect but since we only ever see
			 * those jumps over the first three events it doesn't matter.
			 */
			filter_restart(tp->device->pointer.filter, tp, time - tdelta);
		}
		break;
	case JUMP_STATE_IGNORE:
		break;
	}
}

static void
tp_pre_process_state(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;

	if (tp->queued & TOUCHPAD_EVENT_TIMESTAMP)
		tp_process_msc_timestamp(tp, time);

	tp_process_fake_touches(tp, time);
	tp_unhover_touches(tp, time);

	tp_for_each_touch(tp, t) {
		if (t->state == TOUCH_MAYBE_END)
			tp_end_touch(tp, t, time);

		/* Ignore motion when pressure/touch size fell below the
		 * threshold, thus ending the touch */
		if (t->state == TOUCH_END && t->history.count > 0)
			t->point = tp_motion_history_offset(t, 0)->point;
	}

}

static void
tp_process_state(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;
	bool restart_filter = false;
	bool want_motion_reset;
	bool have_new_touch = false;
	unsigned int speed_exceeded_count = 0;

	tp_position_fake_touches(tp);

	want_motion_reset = tp_need_motion_history_reset(tp);

	tp_for_each_touch(tp, t) {
		if (t->state == TOUCH_NONE)
			continue;

		if (want_motion_reset) {
			tp_motion_history_reset(t);
			t->quirks.reset_motion_history = true;
		} else if (t->quirks.reset_motion_history) {
			tp_motion_history_reset(t);
			t->quirks.reset_motion_history = false;
		}

		if (!t->dirty) {
			/* A non-dirty touch must be below the speed limit */
			if (t->speed.exceeded_count > 0)
				t->speed.exceeded_count--;

			speed_exceeded_count = max(speed_exceeded_count,
						   t->speed.exceeded_count);

			/* A touch that hasn't moved must be in the same
			 * position, so let's add this to the motion
			 * history.
			 */
			tp_motion_history_push(t, time);
			continue;
		}

		if (tp_detect_jumps(tp, t, time)) {
			if (!tp->semi_mt)
				evdev_log_bug_kernel_ratelimit(tp->device,
						&tp->jump.warning,
					        "Touch jump detected and discarded.\n"
					        "See %s/touchpad-jumping-cursors.html for details\n",
					        HTTP_DOC_LINK);
			tp_motion_history_reset(t);
		}

		tp_thumb_update_touch(tp, t, time);
		tp_palm_detect(tp, t, time);
		tp_detect_wobbling(tp, t, time);
		tp_motion_hysteresis(tp, t);
		tp_motion_history_push(t, time);

		/* Touch speed handling: if we'are above the threshold,
		 * count each event that we're over the threshold up to 10
		 * events. Count down when we are below the speed.
		 *
		 * Take the touch with the highest speed excess, if it is
		 * above a certain threshold (5, see below), assume a
		 * dropped finger is a thumb.
		 *
		 * Yes, this relies on the touchpad to keep sending us
		 * events even if the finger doesn't move, otherwise we
		 * never count down. Let's see how far we get with that.
		 */
		if (t->speed.last_speed > THUMB_IGNORE_SPEED_THRESHOLD) {
			if (t->speed.exceeded_count < 15)
				t->speed.exceeded_count++;
		} else if (t->speed.exceeded_count > 0) {
				t->speed.exceeded_count--;
		}

		speed_exceeded_count = max(speed_exceeded_count,
					   t->speed.exceeded_count);

		tp_calculate_motion_speed(tp, t, time);

		tp_unpin_finger(tp, t);

		if (t->state == TOUCH_BEGIN) {
			have_new_touch = true;
			restart_filter = true;
		}
	}

	if (tp->thumb.detect_thumbs &&
	    have_new_touch &&
	    tp->nfingers_down >= 2)
		tp_thumb_update_multifinger(tp);

	if (restart_filter)
		filter_restart(tp->device->pointer.filter, tp, time);

	tp_button_handle_state(tp, time);
	tp_edge_scroll_handle_state(tp, time);

	/*
	 * We have a physical button down event on a clickpad. To avoid
	 * spurious pointer moves by the clicking finger we pin all fingers.
	 * We unpin fingers when they move more then a certain threshold to
	 * to allow drag and drop.
	 */
	if ((tp->queued & TOUCHPAD_EVENT_BUTTON_PRESS) &&
	    tp->buttons.is_clickpad)
		tp_pin_fingers(tp);

	tp_gesture_handle_state(tp, time);
}

static void
tp_post_process_state(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;

	tp_for_each_touch(tp, t) {

		if (!t->dirty)
			continue;

		if (t->state == TOUCH_END) {
			if (t->has_ended)
				t->state = TOUCH_NONE;
			else
				t->state = TOUCH_HOVERING;
		} else if (t->state == TOUCH_BEGIN) {
			t->state = TOUCH_UPDATE;
		}

		t->dirty = false;
	}

	tp->old_nfingers_down = tp->nfingers_down;
	tp->buttons.old_state = tp->buttons.state;

	tp->queued = TOUCHPAD_EVENT_NONE;

	if (tp->nfingers_down == 0)
		tp_thumb_reset(tp);

	tp_tap_post_process_state(tp);
}

static void
tp_post_events(struct tp_dispatch *tp, uint64_t time)
{
	bool ignore_motion = false;

	/* Only post (top) button events while suspended */
	if (tp->device->is_suspended) {
		tp_post_button_events(tp, time);
		return;
	}

	ignore_motion |= tp_tap_handle_state(tp, time);
	ignore_motion |= tp_post_button_events(tp, time);

	if (tp->palm.trackpoint_active || tp->dwt.keyboard_active) {
		tp_edge_scroll_stop_events(tp, time);
		tp_gesture_cancel(tp, time);
		return;
	}

	if (ignore_motion) {
		tp_edge_scroll_stop_events(tp, time);
		tp_gesture_cancel_motion_gestures(tp, time);
		tp_gesture_post_events(tp, time, true);
		return;
	}

	if (tp_edge_scroll_post_events(tp, time) != 0)
		return;

	tp_gesture_post_events(tp, time, false);
}

static void
tp_apply_rotation(struct evdev_device *device)
{
	struct tp_dispatch *tp = (struct tp_dispatch *)device->dispatch;

	if (tp->left_handed.want_rotate == tp->left_handed.rotate)
		return;

	if (tp->nfingers_down)
		return;

	tp->left_handed.rotate = tp->left_handed.want_rotate;

	evdev_log_debug(device,
			"touchpad-rotation: rotation is %s\n",
			tp->left_handed.rotate ? "on" : "off");
}

static void
tp_handle_state(struct tp_dispatch *tp,
		uint64_t time)
{
	tp_pre_process_state(tp, time);
	tp_process_state(tp, time);
	tp_post_events(tp, time);
	tp_post_process_state(tp, time);

	tp_clickpad_middlebutton_apply_config(tp->device);
	tp_apply_rotation(tp->device);
}

LIBINPUT_UNUSED
static inline void
tp_debug_touch_state(struct tp_dispatch *tp,
		     struct evdev_device *device)
{
	char buf[1024] = {0};
	struct tp_touch *t;
	size_t i = 0;

	tp_for_each_touch(tp, t) {
		if (i >= tp->nfingers_down)
			break;
		sprintf(&buf[strlen(buf)],
			"slot %zd: %04d/%04d p%03d %s |",
			i++,
			t->point.x,
			t->point.y,
			t->pressure,
			tp_touch_active(tp, t) ? "" : "inactive");
	}
	if (buf[0] != '\0')
		evdev_log_debug(device, "touch state: %s\n", buf);
}

static void
tp_interface_process(struct evdev_dispatch *dispatch,
		     struct evdev_device *device,
		     struct input_event *e,
		     uint64_t time)
{
	struct tp_dispatch *tp = tp_dispatch(dispatch);

	switch (e->type) {
	case EV_ABS:
		if (tp->has_mt)
			tp_process_absolute(tp, e, time);
		else
			tp_process_absolute_st(tp, e, time);
		break;
	case EV_KEY:
		tp_process_key(tp, e, time);
		break;
	case EV_MSC:
		tp_process_msc(tp, e, time);
		break;
	case EV_SYN:
		tp_handle_state(tp, time);
#if 0
		tp_debug_touch_state(tp, device);
#endif
		break;
	}
}

static void
tp_remove_sendevents(struct tp_dispatch *tp)
{
	struct evdev_paired_keyboard *kbd;

	libinput_timer_cancel(&tp->palm.trackpoint_timer);
	libinput_timer_cancel(&tp->dwt.keyboard_timer);

	if (tp->buttons.trackpoint &&
	    tp->palm.monitor_trackpoint)
		libinput_device_remove_event_listener(
					&tp->palm.trackpoint_listener);

	list_for_each(kbd, &tp->dwt.paired_keyboard_list, link) {
		libinput_device_remove_event_listener(&kbd->listener);
	}

	if (tp->lid_switch.lid_switch)
		libinput_device_remove_event_listener(
					&tp->lid_switch.listener);

	if (tp->tablet_mode_switch.tablet_mode_switch)
		libinput_device_remove_event_listener(
					&tp->tablet_mode_switch.listener);
}

static void
tp_interface_remove(struct evdev_dispatch *dispatch)
{
	struct tp_dispatch *tp = tp_dispatch(dispatch);
	struct evdev_paired_keyboard *kbd;

	libinput_timer_cancel(&tp->arbitration.arbitration_timer);

	list_for_each_safe(kbd, &tp->dwt.paired_keyboard_list, link) {
		evdev_paired_keyboard_destroy(kbd);
	}
	tp->dwt.keyboard_active = false;

	tp_remove_tap(tp);
	tp_remove_buttons(tp);
	tp_remove_sendevents(tp);
	tp_remove_edge_scroll(tp);
	tp_remove_gesture(tp);
}

static void
tp_interface_destroy(struct evdev_dispatch *dispatch)
{
	struct tp_dispatch *tp = tp_dispatch(dispatch);

	libinput_timer_destroy(&tp->arbitration.arbitration_timer);
	libinput_timer_destroy(&tp->palm.trackpoint_timer);
	libinput_timer_destroy(&tp->dwt.keyboard_timer);
	libinput_timer_destroy(&tp->tap.timer);
	libinput_timer_destroy(&tp->gesture.finger_count_switch_timer);
	libinput_timer_destroy(&tp->gesture.hold_timer);
	free(tp->touches);
	free(tp);
}

static void
tp_release_fake_touches(struct tp_dispatch *tp)
{
	tp->fake_touches = 0;
}

static void
tp_clear_state(struct tp_dispatch *tp)
{
	uint64_t now = libinput_now(tp_libinput_context(tp));
	struct tp_touch *t;

	/* Unroll the touchpad state.
	 * Release buttons first. If tp is a clickpad, the button event
	 * must come before the touch up. If it isn't, the order doesn't
	 * matter anyway
	 *
	 * Then cancel all timeouts on the taps, triggering the last set
	 * of events.
	 *
	 * Then lift all touches so the touchpad is in a neutral state.
	 *
	 * Then reset thumb state.
	 *
	 */
	tp_release_all_buttons(tp, now);
	tp_release_all_taps(tp, now);

	tp_for_each_touch(tp, t) {
		tp_end_sequence(tp, t, now);
	}
	tp_release_fake_touches(tp);

	tp_thumb_reset(tp);

	tp_handle_state(tp, now);
}

static void
tp_suspend(struct tp_dispatch *tp,
	   struct evdev_device *device,
	   enum suspend_trigger trigger)
{
	if (tp->suspend_reason & trigger)
		return;

	if (tp->suspend_reason != 0)
		goto out;

	tp_clear_state(tp);

	/* On devices with top softwarebuttons we don't actually suspend the
	 * device, to keep the "trackpoint" buttons working. tp_post_events()
	 * will only send events for the trackpoint while suspended.
	 */
	if (tp->buttons.has_topbuttons) {
		evdev_notify_suspended_device(device);
		/* Enlarge topbutton area while suspended */
		tp_init_top_softbuttons(tp, device, 3.0);
	} else {
		evdev_device_suspend(device);
	}

out:
	tp->suspend_reason |= trigger;
}

static void
tp_interface_suspend(struct evdev_dispatch *dispatch,
		     struct evdev_device *device)
{
	struct tp_dispatch *tp = tp_dispatch(dispatch);

	tp_clear_state(tp);
}

static inline void
tp_sync_touch(struct tp_dispatch *tp,
	      struct evdev_device *device,
	      struct tp_touch *t,
	      int slot)
{
	struct libevdev *evdev = device->evdev;
	int tracking_id;

	if (!libevdev_fetch_slot_value(evdev,
				       slot,
				       ABS_MT_POSITION_X,
				       &t->point.x))
		t->point.x = libevdev_get_event_value(evdev, EV_ABS, ABS_X);
	if (!libevdev_fetch_slot_value(evdev,
				       slot,
				       ABS_MT_POSITION_Y,
				       &t->point.y))
		t->point.y = libevdev_get_event_value(evdev, EV_ABS, ABS_Y);

	if (!libevdev_fetch_slot_value(evdev,
				       slot,
				       ABS_MT_PRESSURE,
				       &t->pressure))
		t->pressure = libevdev_get_event_value(evdev,
						       EV_ABS,
						       ABS_PRESSURE);

	libevdev_fetch_slot_value(evdev,
				  slot,
				  ABS_MT_TOUCH_MAJOR,
				  &t->major);
	libevdev_fetch_slot_value(evdev,
				  slot,
				  ABS_MT_TOUCH_MINOR,
				  &t->minor);

	if (libevdev_fetch_slot_value(evdev,
				      slot,
				      ABS_MT_TRACKING_ID,
				      &tracking_id) &&
	    tracking_id != -1)
		tp->nactive_slots++;
}

static void
tp_sync_slots(struct tp_dispatch *tp,
	      struct evdev_device *device)
{
	/* Always sync the first touch so we get ABS_X/Y synced on
	 * single-touch touchpads */
	tp_sync_touch(tp, device, &tp->touches[0], 0);
	for (unsigned int i = 1; i < tp->num_slots; i++)
		tp_sync_touch(tp, device, &tp->touches[i], i);
}

static void
tp_resume(struct tp_dispatch *tp,
	  struct evdev_device *device,
	  enum suspend_trigger trigger)
{
	tp->suspend_reason &= ~trigger;
	if (tp->suspend_reason != 0)
		return;

	if (tp->buttons.has_topbuttons) {
		/* tap state-machine is offline while suspended, reset state */
		tp_clear_state(tp);
		/* restore original topbutton area size */
		tp_init_top_softbuttons(tp, device, 1.0);
		evdev_notify_resumed_device(device);
	} else {
		evdev_device_resume(device);
	}

	tp_sync_slots(tp, device);
}

static void
tp_trackpoint_timeout(uint64_t now, void *data)
{
	struct tp_dispatch *tp = data;

	if (tp->palm.trackpoint_active) {
		tp_tap_resume(tp, now);
		tp->palm.trackpoint_active = false;
	}
	tp->palm.trackpoint_event_count = 0;
}

static void
tp_trackpoint_event(uint64_t time, struct libinput_event *event, void *data)
{
	struct tp_dispatch *tp = data;

	if (!tp->palm.dwtp_enabled)
		return;

	/* Buttons do not count as trackpad activity, as people may use
	   the trackpoint buttons in combination with the touchpad. */
	if (event->type == LIBINPUT_EVENT_POINTER_BUTTON)
		return;

	tp->palm.trackpoint_last_event_time = time;
	tp->palm.trackpoint_event_count++;

	/* Require at least three events before enabling palm detection */
	if (tp->palm.trackpoint_event_count < 3) {
		libinput_timer_set(&tp->palm.trackpoint_timer,
				   time + DEFAULT_TRACKPOINT_EVENT_TIMEOUT);
		return;
	}

	if (!tp->palm.trackpoint_active) {
		tp_stop_actions(tp, time);
		tp->palm.trackpoint_active = true;
	}

	libinput_timer_set(&tp->palm.trackpoint_timer,
			   time + DEFAULT_TRACKPOINT_ACTIVITY_TIMEOUT);
}

static void
tp_keyboard_timeout(uint64_t now, void *data)
{
	struct tp_dispatch *tp = data;

	if (tp->dwt.dwt_enabled &&
	    long_any_bit_set(tp->dwt.key_mask,
			     ARRAY_LENGTH(tp->dwt.key_mask))) {
		libinput_timer_set(&tp->dwt.keyboard_timer,
				   now + DEFAULT_KEYBOARD_ACTIVITY_TIMEOUT_2);
		tp->dwt.keyboard_last_press_time = now;
		evdev_log_debug(tp->device, "palm: keyboard timeout refresh\n");
		return;
	}

	tp_tap_resume(tp, now);

	tp->dwt.keyboard_active = false;

	evdev_log_debug(tp->device, "palm: keyboard timeout\n");
}

static inline bool
tp_key_is_modifier(unsigned int keycode)
{
	switch (keycode) {
	/* Ignore modifiers to be responsive to ctrl-click, alt-tab, etc. */
	case KEY_LEFTCTRL:
	case KEY_RIGHTCTRL:
	case KEY_LEFTALT:
	case KEY_RIGHTALT:
	case KEY_LEFTSHIFT:
	case KEY_RIGHTSHIFT:
	case KEY_FN:
	case KEY_CAPSLOCK:
	case KEY_TAB:
	case KEY_COMPOSE:
	case KEY_RIGHTMETA:
	case KEY_LEFTMETA:
		return true;
	default:
		return false;
	}
}

static inline bool
tp_key_ignore_for_dwt(unsigned int keycode)
{
	/* Ignore keys not part of the "typewriter set", i.e. F-keys,
	 * multimedia keys, numpad, etc.
	 */

	if (tp_key_is_modifier(keycode))
		return false;

	switch (keycode) {
	case KEY_ESC:
	case KEY_KPASTERISK:
		return true;
	default:
		return keycode >= KEY_F1;
	}
}

static void
tp_keyboard_event(uint64_t time, struct libinput_event *event, void *data)
{
	struct tp_dispatch *tp = data;
	struct libinput_event_keyboard *kbdev;
	unsigned int timeout;
	unsigned int key;
	bool is_modifier;

	if (event->type != LIBINPUT_EVENT_KEYBOARD_KEY)
		return;

	kbdev = libinput_event_get_keyboard_event(event);
	key = libinput_event_keyboard_get_key(kbdev);

	/* Only trigger the timer on key down. */
	if (libinput_event_keyboard_get_key_state(kbdev) !=
	    LIBINPUT_KEY_STATE_PRESSED) {
		long_clear_bit(tp->dwt.key_mask, key);
		long_clear_bit(tp->dwt.mod_mask, key);
		return;
	}

	if (!tp->dwt.dwt_enabled)
		return;

	if (tp_key_ignore_for_dwt(key))
		return;

	/* modifier keys don't trigger disable-while-typing so things like
	 * ctrl+zoom or ctrl+click are possible */
	is_modifier = tp_key_is_modifier(key);
	if (is_modifier) {
		long_set_bit(tp->dwt.mod_mask, key);
		return;
	}

	if (!tp->dwt.keyboard_active) {
		/* This is the first non-modifier key press. Check if the
		 * modifier mask is set. If any modifier is down we don't
		 * trigger dwt because it's likely to be combination like
		 * Ctrl+S or similar */

		if (long_any_bit_set(tp->dwt.mod_mask,
				     ARRAY_LENGTH(tp->dwt.mod_mask)))
		    return;

		tp_stop_actions(tp, time);
		tp->dwt.keyboard_active = true;
		timeout = DEFAULT_KEYBOARD_ACTIVITY_TIMEOUT_1;
	} else {
		timeout = DEFAULT_KEYBOARD_ACTIVITY_TIMEOUT_2;
	}

	tp->dwt.keyboard_last_press_time = time;
	long_set_bit(tp->dwt.key_mask, key);
	libinput_timer_set(&tp->dwt.keyboard_timer,
			   time + timeout);
}

static bool
tp_want_dwt(struct evdev_device *touchpad,
	    struct evdev_device *keyboard)
{
	unsigned int vendor_tp = evdev_device_get_id_vendor(touchpad);
	unsigned int vendor_kbd = evdev_device_get_id_vendor(keyboard);
	unsigned int product_tp = evdev_device_get_id_product(touchpad);
	unsigned int product_kbd = evdev_device_get_id_product(keyboard);

	/* External touchpads with the same vid/pid as the keyboard are
	   considered a happy couple */
	if (touchpad->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD)
		return vendor_tp == vendor_kbd && product_tp == product_kbd;

	if (keyboard->tags & EVDEV_TAG_INTERNAL_KEYBOARD)
		return true;

	/* keyboard is not tagged as internal keyboard and it's not part of
	 * a combo */
	return false;
}

static void
tp_dwt_pair_keyboard(struct evdev_device *touchpad,
		     struct evdev_device *keyboard)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)touchpad->dispatch;
	struct evdev_paired_keyboard *kbd;
	size_t count = 0;

	if ((keyboard->tags & EVDEV_TAG_KEYBOARD) == 0)
		return;

	if (!tp_want_dwt(touchpad, keyboard))
		return;

	list_for_each(kbd, &tp->dwt.paired_keyboard_list, link) {
		count++;
		if (count > 3) {
			evdev_log_info(touchpad,
				       "too many internal keyboards for dwt\n");
			break;
		}
	}

	kbd = zalloc(sizeof(*kbd));
	kbd->device = keyboard;
	libinput_device_add_event_listener(&keyboard->base,
					   &kbd->listener,
					   tp_keyboard_event, tp);
	list_insert(&tp->dwt.paired_keyboard_list, &kbd->link);
	evdev_log_debug(touchpad,
			"palm: dwt activated with %s<->%s\n",
			touchpad->devname,
			keyboard->devname);
}

static void
tp_pair_trackpoint(struct evdev_device *touchpad,
			struct evdev_device *trackpoint)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)touchpad->dispatch;
	unsigned int bus_trp = libevdev_get_id_bustype(trackpoint->evdev);
	bool tp_is_internal, trp_is_internal;

	if ((trackpoint->tags & EVDEV_TAG_TRACKPOINT) == 0)
		return;

	tp_is_internal = !!(touchpad->tags & EVDEV_TAG_INTERNAL_TOUCHPAD);
	trp_is_internal = bus_trp != BUS_USB && bus_trp != BUS_BLUETOOTH;

	if (tp->buttons.trackpoint == NULL &&
	    tp_is_internal && trp_is_internal) {
		/* Don't send any pending releases to the new trackpoint */
		tp->buttons.active_is_topbutton = false;
		tp->buttons.trackpoint = trackpoint;
		if (tp->palm.monitor_trackpoint)
			libinput_device_add_event_listener(&trackpoint->base,
						&tp->palm.trackpoint_listener,
						tp_trackpoint_event, tp);
	}
}

static void
tp_lid_switch_event(uint64_t time, struct libinput_event *event, void *data)
{
	struct tp_dispatch *tp = data;
	struct libinput_event_switch *swev;

	if (libinput_event_get_type(event) != LIBINPUT_EVENT_SWITCH_TOGGLE)
		return;

	swev = libinput_event_get_switch_event(event);
	if (libinput_event_switch_get_switch(swev) != LIBINPUT_SWITCH_LID)
		return;

	switch (libinput_event_switch_get_switch_state(swev)) {
	case LIBINPUT_SWITCH_STATE_OFF:
		tp_resume(tp, tp->device, SUSPEND_LID);
		evdev_log_debug(tp->device, "lid: resume touchpad\n");
		break;
	case LIBINPUT_SWITCH_STATE_ON:
		tp_suspend(tp, tp->device, SUSPEND_LID);
		evdev_log_debug(tp->device, "lid: suspending touchpad\n");
		break;
	}
}

static void
tp_tablet_mode_switch_event(uint64_t time,
			    struct libinput_event *event,
			    void *data)
{
	struct tp_dispatch *tp = data;
	struct libinput_event_switch *swev;

	if (libinput_event_get_type(event) != LIBINPUT_EVENT_SWITCH_TOGGLE)
		return;

	swev = libinput_event_get_switch_event(event);
	if (libinput_event_switch_get_switch(swev) !=
	    LIBINPUT_SWITCH_TABLET_MODE)
		return;

	switch (libinput_event_switch_get_switch_state(swev)) {
	case LIBINPUT_SWITCH_STATE_OFF:
		tp_resume(tp, tp->device, SUSPEND_TABLET_MODE);
		evdev_log_debug(tp->device, "tablet-mode: resume touchpad\n");
		break;
	case LIBINPUT_SWITCH_STATE_ON:
		tp_suspend(tp, tp->device, SUSPEND_TABLET_MODE);
		evdev_log_debug(tp->device, "tablet-mode: suspending touchpad\n");
		break;
	}
}

static void
tp_pair_lid_switch(struct evdev_device *touchpad,
		   struct evdev_device *lid_switch)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)touchpad->dispatch;

	if ((lid_switch->tags & EVDEV_TAG_LID_SWITCH) == 0)
		return;

	if (touchpad->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD)
		return;

	if (tp->lid_switch.lid_switch == NULL) {
		evdev_log_debug(touchpad,
				"lid: activated for %s<->%s\n",
				touchpad->devname,
				lid_switch->devname);

		libinput_device_add_event_listener(&lid_switch->base,
						   &tp->lid_switch.listener,
						   tp_lid_switch_event, tp);
		tp->lid_switch.lid_switch = lid_switch;
	}
}

static void
tp_pair_tablet_mode_switch(struct evdev_device *touchpad,
			   struct evdev_device *tablet_mode_switch)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)touchpad->dispatch;

	if ((tablet_mode_switch->tags & EVDEV_TAG_TABLET_MODE_SWITCH) == 0)
		return;

	if (tp->tablet_mode_switch.tablet_mode_switch)
		return;

	if (touchpad->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD)
		return;

	if (evdev_device_has_model_quirk(touchpad,
					 QUIRK_MODEL_TABLET_MODE_NO_SUSPEND))
		return;

	evdev_log_debug(touchpad,
			"tablet-mode: activated for %s<->%s\n",
			touchpad->devname,
			tablet_mode_switch->devname);

	libinput_device_add_event_listener(&tablet_mode_switch->base,
				&tp->tablet_mode_switch.listener,
				tp_tablet_mode_switch_event, tp);
	tp->tablet_mode_switch.tablet_mode_switch = tablet_mode_switch;

	if (evdev_device_switch_get_state(tablet_mode_switch,
					  LIBINPUT_SWITCH_TABLET_MODE)
		    == LIBINPUT_SWITCH_STATE_ON) {
		tp_suspend(tp, touchpad, SUSPEND_TABLET_MODE);
	}
}

static void
tp_change_rotation(struct evdev_device *device, enum notify notify)
{
	struct tp_dispatch *tp = (struct tp_dispatch *)device->dispatch;
	struct evdev_device *tablet_device = tp->left_handed.tablet_device;
	bool tablet_is_left, touchpad_is_left;

	if (!tp->left_handed.must_rotate)
		return;

	touchpad_is_left = device->left_handed.enabled;
	tablet_is_left = tp->left_handed.tablet_left_handed_state;

	tp->left_handed.want_rotate = touchpad_is_left || tablet_is_left;

	tp_apply_rotation(device);

	if (notify == DO_NOTIFY && tablet_device) {
		struct evdev_dispatch *dispatch = tablet_device->dispatch;

		if (dispatch->interface->left_handed_toggle)
			dispatch->interface->left_handed_toggle(dispatch,
								tablet_device,
								tp->left_handed.want_rotate);
	}
}

static void
tp_pair_tablet(struct evdev_device *touchpad,
	       struct evdev_device *tablet)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)touchpad->dispatch;

	if (!tp->left_handed.must_rotate)
		return;

	if ((tablet->seat_caps & EVDEV_DEVICE_TABLET) == 0)
		return;

	if (libinput_device_get_device_group(&touchpad->base) !=
	    libinput_device_get_device_group(&tablet->base))
		return;

	tp->left_handed.tablet_device = tablet;

	evdev_log_debug(touchpad,
			"touchpad-rotation: %s will rotate %s\n",
			touchpad->devname,
			tablet->devname);

	if (libinput_device_config_left_handed_get(&tablet->base)) {
		tp->left_handed.want_rotate = true;
		tp->left_handed.tablet_left_handed_state = true;
		tp_change_rotation(touchpad, DONT_NOTIFY);
	}
}

static void
tp_interface_device_added(struct evdev_device *device,
			  struct evdev_device *added_device)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)device->dispatch;

	tp_pair_trackpoint(device, added_device);
	tp_dwt_pair_keyboard(device, added_device);
	tp_pair_lid_switch(device, added_device);
	tp_pair_tablet_mode_switch(device, added_device);
	tp_pair_tablet(device, added_device);

	if (tp->sendevents.current_mode !=
	    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE)
		return;

	if (added_device->tags & EVDEV_TAG_EXTERNAL_MOUSE)
		tp_suspend(tp, device, SUSPEND_EXTERNAL_MOUSE);
}

static void
tp_interface_device_removed(struct evdev_device *device,
			    struct evdev_device *removed_device)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)device->dispatch;
	struct evdev_paired_keyboard *kbd;

	if (removed_device == tp->buttons.trackpoint) {
		/* Clear any pending releases for the trackpoint */
		if (tp->buttons.active && tp->buttons.active_is_topbutton) {
			tp->buttons.active = 0;
			tp->buttons.active_is_topbutton = false;
		}
		if (tp->palm.monitor_trackpoint)
			libinput_device_remove_event_listener(
						&tp->palm.trackpoint_listener);
		tp->buttons.trackpoint = NULL;
	}

	list_for_each_safe(kbd, &tp->dwt.paired_keyboard_list, link) {
		if (kbd->device == removed_device) {
			evdev_paired_keyboard_destroy(kbd);
			tp->dwt.keyboard_active = false;
		}
	}

	if (removed_device == tp->lid_switch.lid_switch) {
		libinput_device_remove_event_listener(
					&tp->lid_switch.listener);
		tp->lid_switch.lid_switch = NULL;
		tp_resume(tp, device, SUSPEND_LID);
	}

	if (removed_device == tp->tablet_mode_switch.tablet_mode_switch) {
		libinput_device_remove_event_listener(
					&tp->tablet_mode_switch.listener);
		tp->tablet_mode_switch.tablet_mode_switch = NULL;
		tp_resume(tp, device, SUSPEND_TABLET_MODE);
	}

	if (tp->sendevents.current_mode ==
		    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE) {
		struct libinput_device *dev;
		bool found = false;

		list_for_each(dev, &device->base.seat->devices_list, link) {
			struct evdev_device *d = evdev_device(dev);
			if (d != removed_device &&
			    (d->tags & EVDEV_TAG_EXTERNAL_MOUSE)) {
				found = true;
				break;
			}
		}
		if (!found)
			tp_resume(tp, device, SUSPEND_EXTERNAL_MOUSE);
	}

	if (removed_device == tp->left_handed.tablet_device) {
		tp->left_handed.tablet_device = NULL;
		tp->left_handed.tablet_left_handed_state = false;

		/* Slight awkwardness: removing the tablet causes the
		 * touchpad to rotate back to normal if only the tablet was
		 * set to left-handed. Niche case, nothing to worry about
		 */
		tp_change_rotation(device, DO_NOTIFY);
	}
}

static inline void
evdev_tag_touchpad_internal(struct evdev_device *device)
{
	device->tags |= EVDEV_TAG_INTERNAL_TOUCHPAD;
	device->tags &= ~EVDEV_TAG_EXTERNAL_TOUCHPAD;
}

static inline void
evdev_tag_touchpad_external(struct evdev_device *device)
{
	device->tags |= EVDEV_TAG_EXTERNAL_TOUCHPAD;
	device->tags &= ~EVDEV_TAG_INTERNAL_TOUCHPAD;
}

static void
evdev_tag_touchpad(struct evdev_device *device,
		   struct udev_device *udev_device)
{
	int bustype, vendor;
	const char *prop;

	prop = udev_device_get_property_value(udev_device,
					      "ID_INPUT_TOUCHPAD_INTEGRATION");
	if (prop) {
		if (streq(prop, "internal")) {
			evdev_tag_touchpad_internal(device);
			return;
		}

		if (streq(prop, "external")) {
			evdev_tag_touchpad_external(device);
			return;
		}

		evdev_log_info(device,
			       "tagged with unknown value %s\n",
			       prop);
	}

	/* The hwdb is the authority on integration, these heuristics are
	 * the fallback only (they precede the hwdb too).
	 *
	 * Simple approach:
	 * Bluetooth touchpads are considered external, anything else is
	 * internal. Except the ones from some vendors that only make external
	 * touchpads.
	 */
	bustype = libevdev_get_id_bustype(device->evdev);
	vendor = libevdev_get_id_vendor(device->evdev);

	switch (bustype) {
	case BUS_BLUETOOTH:
		evdev_tag_touchpad_external(device);
		break;
	default:
		evdev_tag_touchpad_internal(device);
		break;
	}

	switch (vendor) {
	/* Logitech does not have internal touchpads */
	case VENDOR_ID_LOGITECH:
		evdev_tag_touchpad_external(device);
		break;
	}

	/* Wacom makes touchpads, but not internal ones */
	if (device->model_flags & EVDEV_MODEL_WACOM_TOUCHPAD)
		evdev_tag_touchpad_external(device);

	if ((device->tags &
	    (EVDEV_TAG_EXTERNAL_TOUCHPAD|EVDEV_TAG_INTERNAL_TOUCHPAD)) == 0) {
		evdev_log_bug_libinput(device,
				       "Internal or external? Please file a bug.\n");
		evdev_tag_touchpad_external(device);
	}
}

static void
tp_arbitration_timeout(uint64_t now, void *data)
{
	struct tp_dispatch *tp = data;

	if (tp->arbitration.state != ARBITRATION_NOT_ACTIVE)
		tp->arbitration.state = ARBITRATION_NOT_ACTIVE;
}

static void
tp_interface_toggle_touch(struct evdev_dispatch *dispatch,
			  struct evdev_device *device,
			  enum evdev_arbitration_state which,
			  const struct phys_rect *rect,
			  uint64_t time)
{
	struct tp_dispatch *tp = tp_dispatch(dispatch);

	if (which == tp->arbitration.state)
		return;

	switch (which) {
	case ARBITRATION_IGNORE_ALL:
	case ARBITRATION_IGNORE_RECT:
		libinput_timer_cancel(&tp->arbitration.arbitration_timer);
		tp_clear_state(tp);
		tp->arbitration.state = which;
		break;
	case ARBITRATION_NOT_ACTIVE:
		/* if in-kernel arbitration is in use and there is a touch
		 * and a pen in proximity, lifting the pen out of proximity
		 * causes a touch begin for the touch. On a hand-lift the
		 * proximity out precedes the touch up by a few ms, so we
		 * get what looks like a tap. Fix this by delaying
		 * arbitration by just a little bit so that any touch in
		 * event is caught as palm touch. */
		libinput_timer_set(&tp->arbitration.arbitration_timer,
				   time + ms2us(90));
		break;
	}
}

/* Called when the tablet toggles to left-handed */
static void
touchpad_left_handed_toggled(struct evdev_dispatch *dispatch,
			     struct evdev_device *device,
			     bool left_handed_enabled)
{
	struct tp_dispatch *tp = tp_dispatch(dispatch);

	if (!tp->left_handed.tablet_device)
		return;

	evdev_log_debug(device,
			"touchpad-rotation: tablet is %s\n",
			left_handed_enabled ? "left-handed" : "right-handed");

	/* Our left-handed config is independent even though rotation is
	 * locked. So we rotate when either device is left-handed. But it
	 * can only be actually changed when the device is in a neutral
	 * state, hence the want_rotate.
	 */
	tp->left_handed.tablet_left_handed_state = left_handed_enabled;
	tp_change_rotation(device, DONT_NOTIFY);
}

static struct evdev_dispatch_interface tp_interface = {
	.process = tp_interface_process,
	.suspend = tp_interface_suspend,
	.remove = tp_interface_remove,
	.destroy = tp_interface_destroy,
	.device_added = tp_interface_device_added,
	.device_removed = tp_interface_device_removed,
	.device_suspended = tp_interface_device_removed, /* treat as remove */
	.device_resumed = tp_interface_device_added,   /* treat as add */
	.post_added = NULL,
	.touch_arbitration_toggle = tp_interface_toggle_touch,
	.touch_arbitration_update_rect = NULL,
	.get_switch_state = NULL,
	.left_handed_toggle = touchpad_left_handed_toggled,
};

static void
tp_init_touch(struct tp_dispatch *tp,
	      struct tp_touch *t,
	      unsigned int index)
{
	t->tp = tp;
	t->has_ended = true;
	t->index = index;
}

static inline void
tp_disable_abs_mt(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;
	unsigned int code;

	for (code = ABS_MT_SLOT; code <= ABS_MAX; code++)
		libevdev_disable_event_code(evdev, EV_ABS, code);
}

static bool
tp_init_slots(struct tp_dispatch *tp,
	      struct evdev_device *device)
{
	const struct input_absinfo *absinfo;
	struct map {
		unsigned int code;
		int ntouches;
	} max_touches[] = {
		{ BTN_TOOL_QUINTTAP, 5 },
		{ BTN_TOOL_QUADTAP, 4 },
		{ BTN_TOOL_TRIPLETAP, 3 },
		{ BTN_TOOL_DOUBLETAP, 2 },
	};
	unsigned int i, n_btn_tool_touches = 1;

	absinfo = libevdev_get_abs_info(device->evdev, ABS_MT_SLOT);
	if (absinfo) {
		tp->num_slots = absinfo->maximum + 1;
		tp->slot = absinfo->value;
		tp->has_mt = true;
	} else {
		tp->num_slots = 1;
		tp->slot = 0;
		tp->has_mt = false;
	}

	tp->semi_mt = libevdev_has_property(device->evdev, INPUT_PROP_SEMI_MT);

	/* Semi-mt devices are not reliable for true multitouch data, so we
	 * simply pretend they're single touch touchpads with BTN_TOOL bits.
	 * Synaptics:
	 * Terrible resolution when two fingers are down,
	 * causing scroll jumps. The single-touch emulation ABS_X/Y is
	 * accurate but the ABS_MT_POSITION touchpoints report the bounding
	 * box and that causes jumps. See https://bugzilla.redhat.com/1235175
	 * Elantech:
	 * On three-finger taps/clicks, one slot doesn't get a coordinate
	 * assigned. See https://bugs.freedesktop.org/show_bug.cgi?id=93583
	 * Alps:
	 * If three fingers are set down in the same frame, one slot has the
	 * coordinates 0/0 and may not get updated for several frames.
	 * See https://bugzilla.redhat.com/show_bug.cgi?id=1295073
	 *
	 * The HP Pavilion DM4 touchpad has random jumps in slots, including
	 * for single-finger movement. See fdo bug 91135
	 */
	if (tp->semi_mt ||
	    evdev_device_has_model_quirk(tp->device,
					 QUIRK_MODEL_HP_PAVILION_DM4_TOUCHPAD)) {
		tp->num_slots = 1;
		tp->slot = 0;
		tp->has_mt = false;
	}

	if (!tp->has_mt)
		tp_disable_abs_mt(device);

	ARRAY_FOR_EACH(max_touches, m) {
		if (libevdev_has_event_code(device->evdev,
					    EV_KEY,
					    m->code)) {
			n_btn_tool_touches = m->ntouches;
			break;
		}
	}

	tp->ntouches = max(tp->num_slots, n_btn_tool_touches);
	tp->touches = zalloc(tp->ntouches * sizeof(struct tp_touch));

	for (i = 0; i < tp->ntouches; i++)
		tp_init_touch(tp, &tp->touches[i], i);

	tp_sync_slots(tp, device);

	/* Some touchpads don't reset BTN_TOOL_FINGER on touch up and only
	 * change to/from it when BTN_TOOL_DOUBLETAP is set. This causes us
	 * to ignore the first touches events until a two-finger gesture is
	 * performed.
	 */
	if (libevdev_get_event_value(device->evdev, EV_KEY, BTN_TOOL_FINGER))
		tp_fake_finger_set(tp, BTN_TOOL_FINGER, 1);

	return true;
}

static enum libinput_config_status
tp_accel_config_set_profile(struct libinput_device *libinput_device,
			    enum libinput_config_accel_profile profile);

static bool
tp_init_accel(struct tp_dispatch *tp, enum libinput_config_accel_profile which)
{
	struct evdev_device *device = tp->device;
	int res_x, res_y;
	struct motion_filter *filter;
	int dpi = device->dpi;
	bool use_v_avg = device->use_velocity_averaging;

	res_x = tp->device->abs.absinfo_x->resolution;
	res_y = tp->device->abs.absinfo_y->resolution;

	/*
	 * Not all touchpads report the same amount of units/mm (resolution).
	 * Normalize motion events to the default mouse DPI as base
	 * (unaccelerated) speed. This also evens out any differences in x
	 * and y resolution, so that a circle on the
	 * touchpad does not turn into an ellipse on the screen.
	 */
	tp->accel.x_scale_coeff = (DEFAULT_MOUSE_DPI/25.4) / res_x;
	tp->accel.y_scale_coeff = (DEFAULT_MOUSE_DPI/25.4) / res_y;
	tp->accel.xy_scale_coeff = 1.0 * res_x/res_y;

	if (which == LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT) {
		filter = create_pointer_accelerator_filter_touchpad_flat(dpi);
	} else if (which == LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM) {
		filter = create_custom_accelerator_filter();
	} else if (evdev_device_has_model_quirk(device, QUIRK_MODEL_LENOVO_X230) ||
		 tp->device->model_flags & EVDEV_MODEL_LENOVO_X220_TOUCHPAD_FW81) {
		filter = create_pointer_accelerator_filter_lenovo_x230(dpi, use_v_avg);
	} else {
		uint64_t eds_threshold = 0;
		uint64_t eds_value = 0;

		if (libevdev_get_id_bustype(device->evdev) == BUS_BLUETOOTH) {
			eds_threshold = ms2us(50);
			eds_value = ms2us(10);
		}
		filter = create_pointer_accelerator_filter_touchpad(dpi,
								    eds_threshold,
								    eds_value,
								    use_v_avg);
	}

	if (!filter)
		return false;

	evdev_device_init_pointer_acceleration(tp->device, filter);

	device->pointer.config.set_profile = tp_accel_config_set_profile;

	return true;
}

static enum libinput_config_status
tp_accel_config_set_speed(struct libinput_device *device, double speed)
{
	struct evdev_device *dev = evdev_device(device);

	if (!filter_set_speed(dev->pointer.filter, speed))
		return LIBINPUT_CONFIG_STATUS_INVALID;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_status
tp_accel_config_set_profile(struct libinput_device *libinput_device,
			    enum libinput_config_accel_profile profile)
{
	struct evdev_device *device = evdev_device(libinput_device);
	struct tp_dispatch *tp = tp_dispatch(device->dispatch);
	struct motion_filter *filter;
	double speed;

	filter = device->pointer.filter;
	if (filter_get_type(filter) == profile)
		return LIBINPUT_CONFIG_STATUS_SUCCESS;

	speed = filter_get_speed(filter);
	device->pointer.filter = NULL;

	if (tp_init_accel(tp, profile)) {
		tp_accel_config_set_speed(libinput_device, speed);
		filter_destroy(filter);
	} else {
		device->pointer.filter = filter;
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
	}

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static uint32_t
tp_scroll_get_methods(struct tp_dispatch *tp)
{
	uint32_t methods = LIBINPUT_CONFIG_SCROLL_EDGE;

	/* Any movement with more than one finger has random cursor
	 * jumps. Don't allow for 2fg scrolling on this device, see
	 * fdo bug 91135 */
	if (evdev_device_has_model_quirk(tp->device,
					 QUIRK_MODEL_HP_PAVILION_DM4_TOUCHPAD))
		return LIBINPUT_CONFIG_SCROLL_EDGE;

	if (tp->ntouches >= 2)
		methods |= LIBINPUT_CONFIG_SCROLL_2FG;

	return methods;
}

static uint32_t
tp_scroll_config_scroll_method_get_methods(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp_scroll_get_methods(tp);
}

static enum libinput_config_status
tp_scroll_config_scroll_method_set_method(struct libinput_device *device,
		        enum libinput_config_scroll_method method)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;
	uint64_t time = libinput_now(tp_libinput_context(tp));

	if (method == tp->scroll.method)
		return LIBINPUT_CONFIG_STATUS_SUCCESS;

	tp_edge_scroll_stop_events(tp, time);
	tp_gesture_stop_twofinger_scroll(tp, time);

	tp->scroll.method = method;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_scroll_method
tp_scroll_config_scroll_method_get_method(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp->scroll.method;
}

static enum libinput_config_scroll_method
tp_scroll_get_default_method(struct tp_dispatch *tp)
{
	uint32_t methods;
	enum libinput_config_scroll_method method;

	methods = tp_scroll_get_methods(tp);

	if (methods & LIBINPUT_CONFIG_SCROLL_2FG)
		method = LIBINPUT_CONFIG_SCROLL_2FG;
	else
		method = LIBINPUT_CONFIG_SCROLL_EDGE;

	if ((methods & method) == 0)
		evdev_log_bug_libinput(tp->device,
				       "invalid default scroll method %d\n",
				       method);
	return method;
}

static enum libinput_config_scroll_method
tp_scroll_config_scroll_method_get_default_method(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp_scroll_get_default_method(tp);
}

static int
tp_scroll_config_natural_get_default(struct libinput_device *device)
{
	struct evdev_device *dev = evdev_device(device);

	return (evdev_device_has_model_quirk(dev, QUIRK_MODEL_APPLE_TOUCHPAD) ||
		evdev_device_has_model_quirk(dev, QUIRK_MODEL_APPLE_TOUCHPAD_ONEBUTTON));
}

static void
tp_init_scroll(struct tp_dispatch *tp, struct evdev_device *device)
{
	tp_edge_scroll_init(tp, device);

	evdev_init_natural_scroll(device);
	/* Override natural scroll config for Apple touchpads */
	device->scroll.config_natural.get_default_enabled = tp_scroll_config_natural_get_default;
	device->scroll.natural_scrolling_enabled = tp_scroll_config_natural_get_default(&device->base);

	tp->scroll.config_method.get_methods = tp_scroll_config_scroll_method_get_methods;
	tp->scroll.config_method.set_method = tp_scroll_config_scroll_method_set_method;
	tp->scroll.config_method.get_method = tp_scroll_config_scroll_method_get_method;
	tp->scroll.config_method.get_default_method = tp_scroll_config_scroll_method_get_default_method;
	tp->scroll.method = tp_scroll_get_default_method(tp);
	tp->device->base.config.scroll_method = &tp->scroll.config_method;

	 /* In mm for touchpads with valid resolution, see tp_init_accel() */
	tp->device->scroll.threshold = 0.0;
	tp->device->scroll.direction_lock_threshold = 5.0;
}

static int
tp_dwt_config_is_available(struct libinput_device *device)
{
	return 1;
}

static enum libinput_config_status
tp_dwt_config_set(struct libinput_device *device,
	   enum libinput_config_dwt_state enable)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	switch(enable) {
	case LIBINPUT_CONFIG_DWT_ENABLED:
	case LIBINPUT_CONFIG_DWT_DISABLED:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	tp->dwt.dwt_enabled = (enable == LIBINPUT_CONFIG_DWT_ENABLED);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_dwt_state
tp_dwt_config_get(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp->dwt.dwt_enabled ?
		LIBINPUT_CONFIG_DWT_ENABLED :
		LIBINPUT_CONFIG_DWT_DISABLED;
}

static bool
tp_dwt_default_enabled(struct tp_dispatch *tp)
{
	return true;
}

static enum libinput_config_dwt_state
tp_dwt_config_get_default(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp_dwt_default_enabled(tp) ?
		LIBINPUT_CONFIG_DWT_ENABLED :
		LIBINPUT_CONFIG_DWT_DISABLED;
}

static int
tp_dwtp_config_is_available(struct libinput_device *device)
{
	return 1;
}

static enum libinput_config_status
tp_dwtp_config_set(struct libinput_device *device,
	   enum libinput_config_dwtp_state enable)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	switch(enable) {
	case LIBINPUT_CONFIG_DWTP_ENABLED:
	case LIBINPUT_CONFIG_DWTP_DISABLED:
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	tp->palm.dwtp_enabled = (enable == LIBINPUT_CONFIG_DWTP_ENABLED);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_dwtp_state
tp_dwtp_config_get(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp->palm.dwtp_enabled ?
		LIBINPUT_CONFIG_DWTP_ENABLED :
		LIBINPUT_CONFIG_DWTP_DISABLED;
}

static bool
tp_dwtp_default_enabled(struct tp_dispatch *tp)
{
	return true;
}

static enum libinput_config_dwtp_state
tp_dwtp_config_get_default(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp_dwtp_default_enabled(tp) ?
		LIBINPUT_CONFIG_DWTP_ENABLED :
		LIBINPUT_CONFIG_DWTP_DISABLED;
}

static inline bool
tp_is_tpkb_combo_below(struct evdev_device *device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	char *prop;
	enum tpkbcombo_layout layout = TPKBCOMBO_LAYOUT_UNKNOWN;
	int rc = false;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q)
		return false;

	if (quirks_get_string(q, QUIRK_ATTR_TPKBCOMBO_LAYOUT, &prop)) {
		rc = parse_tpkbcombo_layout_poperty(prop, &layout) &&
			layout == TPKBCOMBO_LAYOUT_BELOW;
	}

	quirks_unref(q);

	return rc;
}

static inline bool
tp_is_tablet(struct evdev_device *device)
{
	return device->tags & EVDEV_TAG_TABLET_TOUCHPAD;
}

static void
tp_init_dwt(struct tp_dispatch *tp,
	    struct evdev_device *device)
{
	if (device->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD &&
	    !tp_is_tpkb_combo_below(device))
		return;

	tp->dwt.config.is_available = tp_dwt_config_is_available;
	tp->dwt.config.set_enabled = tp_dwt_config_set;
	tp->dwt.config.get_enabled = tp_dwt_config_get;
	tp->dwt.config.get_default_enabled = tp_dwt_config_get_default;
	tp->dwt.dwt_enabled = tp_dwt_default_enabled(tp);
	device->base.config.dwt = &tp->dwt.config;
}

static void
tp_init_dwtp(struct tp_dispatch *tp,
	    struct evdev_device *device)
{
	tp->palm.dwtp_enabled = tp_dwtp_default_enabled(tp);

	if (device->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD)
		return;

	tp->palm.config.is_available = tp_dwtp_config_is_available;
	tp->palm.config.set_enabled = tp_dwtp_config_set;
	tp->palm.config.get_enabled = tp_dwtp_config_get;
	tp->palm.config.get_default_enabled = tp_dwtp_config_get_default;
	device->base.config.dwtp = &tp->palm.config;
}

static inline void
tp_init_palmdetect_edge(struct tp_dispatch *tp,
			struct evdev_device *device)
{
	double width, height;
	struct phys_coords mm = { 0.0, 0.0 };
	struct device_coords edges;

	if (device->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD &&
	    !tp_is_tpkb_combo_below(device))
		return;

	/* Edge palm detection hurts more than it helps on Apple touchpads. */
	if (evdev_device_has_model_quirk(device, QUIRK_MODEL_APPLE_TOUCHPAD))
		return;

	evdev_device_get_size(device, &width, &height);

	/* Enable edge palm detection on touchpads >= 70 mm. Anything
	   smaller probably won't need it, until we find out it does */
	if (width < 70.0)
		return;

	/* palm edges are 8% of the width on each side */
	mm.x = min(8, width * 0.08);
	edges = evdev_device_mm_to_units(device, &mm);
	tp->palm.left_edge = edges.x;

	mm.x = width - min(8, width * 0.08);
	edges = evdev_device_mm_to_units(device, &mm);
	tp->palm.right_edge = edges.x;

	if (!tp->buttons.has_topbuttons && height > 55) {
		/* top edge is 5% of the height */
		mm.y = height * 0.05;
		edges = evdev_device_mm_to_units(device, &mm);
		tp->palm.upper_edge = edges.y;
	}
}

static int
tp_read_palm_pressure_prop(struct tp_dispatch *tp,
			   const struct evdev_device *device)
{
	const int default_palm_threshold = 130;
	uint32_t threshold = default_palm_threshold;
	struct quirks_context *quirks;
	struct quirks *q;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q)
		return threshold;

	quirks_get_uint32(q, QUIRK_ATTR_PALM_PRESSURE_THRESHOLD, &threshold);
	quirks_unref(q);

	return threshold;
}

static inline void
tp_init_palmdetect_pressure(struct tp_dispatch *tp,
			    struct evdev_device *device)
{
	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_MT_PRESSURE)) {
		tp->palm.use_pressure = false;
		return;
	}

	tp->palm.pressure_threshold = tp_read_palm_pressure_prop(tp, device);
	if (tp->palm.pressure_threshold != 0) {
		tp->palm.use_pressure = true;

		evdev_log_debug(device,
				"palm: pressure threshold is %d\n",
				tp->palm.pressure_threshold);
	}
}

static inline void
tp_init_palmdetect_size(struct tp_dispatch *tp,
			struct evdev_device *device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	uint32_t threshold;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q)
		return;

	if (quirks_get_uint32(q, QUIRK_ATTR_PALM_SIZE_THRESHOLD, &threshold)) {
		if (threshold != 0) {
			tp->palm.use_size = true;
			tp->palm.size_threshold = threshold;
		}
	}
	quirks_unref(q);
}

static inline void
tp_init_palmdetect_arbitration(struct tp_dispatch *tp,
			       struct evdev_device *device)
{
	char timer_name[64];

	snprintf(timer_name,
		 sizeof(timer_name),
		  "%s arbitration",
		  evdev_device_get_sysname(device));
	libinput_timer_init(&tp->arbitration.arbitration_timer,
			    tp_libinput_context(tp),
			    timer_name,
			    tp_arbitration_timeout, tp);
	tp->arbitration.state = ARBITRATION_NOT_ACTIVE;
}

static void
tp_init_palmdetect(struct tp_dispatch *tp,
		   struct evdev_device *device)
{

	tp->palm.right_edge = INT_MAX;
	tp->palm.left_edge = INT_MIN;
	tp->palm.upper_edge = INT_MIN;

	tp_init_palmdetect_arbitration(tp, device);

	if (device->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD &&
	    !tp_is_tpkb_combo_below(device) &&
	    !tp_is_tablet(device))
		return;

	if (!tp_is_tablet(device))
		tp->palm.monitor_trackpoint = true;

	if (libevdev_has_event_code(device->evdev,
				    EV_ABS,
				    ABS_MT_TOOL_TYPE))
		tp->palm.use_mt_tool = true;

	if (!tp_is_tablet(device))
		tp_init_palmdetect_edge(tp, device);
	tp_init_palmdetect_pressure(tp, device);
	tp_init_palmdetect_size(tp, device);
}

static void
tp_init_sendevents(struct tp_dispatch *tp,
		   struct evdev_device *device)
{
	char timer_name[64];

	snprintf(timer_name,
		 sizeof(timer_name),
		  "%s trackpoint",
		  evdev_device_get_sysname(device));
	libinput_timer_init(&tp->palm.trackpoint_timer,
			    tp_libinput_context(tp),
			    timer_name,
			    tp_trackpoint_timeout, tp);

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s keyboard",
		 evdev_device_get_sysname(device));
	libinput_timer_init(&tp->dwt.keyboard_timer,
			    tp_libinput_context(tp),
			    timer_name,
			    tp_keyboard_timeout, tp);
}

static bool
tp_pass_sanity_check(struct tp_dispatch *tp,
		     struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;

	if (!libevdev_has_event_code(evdev, EV_ABS, ABS_X))
		goto error;

	if (!libevdev_has_event_code(evdev, EV_KEY, BTN_TOUCH))
		goto error;

	if (!libevdev_has_event_code(evdev, EV_KEY, BTN_TOOL_FINGER))
		goto error;

	return true;

error:
	evdev_log_bug_kernel(device,
			     "device failed touchpad sanity checks\n");
	return false;
}

static void
tp_init_default_resolution(struct tp_dispatch *tp,
			   struct evdev_device *device)
{
	const int touchpad_width_mm = 69, /* 1 under palm detection */
		  touchpad_height_mm = 50;
	int xres, yres;

	if (!device->abs.is_fake_resolution)
		return;

	/* we only get here if
	 * - the touchpad provides no resolution
	 * - the udev hwdb didn't override the resolution
	 * - no ATTR_SIZE_HINT is set
	 *
	 * The majority of touchpads that triggers all these conditions
	 * are old ones, so let's assume a small touchpad size and assume
	 * that.
	 */
	evdev_log_info(device,
		       "no resolution or size hints, assuming a size of %dx%dmm\n",
		       touchpad_width_mm,
		       touchpad_height_mm);

	xres = device->abs.dimensions.x/touchpad_width_mm;
	yres = device->abs.dimensions.y/touchpad_height_mm;
	libevdev_set_abs_resolution(device->evdev, ABS_X, xres);
	libevdev_set_abs_resolution(device->evdev, ABS_Y, yres);
	libevdev_set_abs_resolution(device->evdev, ABS_MT_POSITION_X, xres);
	libevdev_set_abs_resolution(device->evdev, ABS_MT_POSITION_Y, yres);
	device->abs.is_fake_resolution = false;
}

static inline void
tp_init_hysteresis(struct tp_dispatch *tp)
{
	int xmargin, ymargin;
	const struct input_absinfo *ax = tp->device->abs.absinfo_x,
				   *ay = tp->device->abs.absinfo_y;

	if (ax->fuzz)
		xmargin = ax->fuzz;
	else
		xmargin = ax->resolution/4;

	if (ay->fuzz)
		ymargin = ay->fuzz;
	else
		ymargin = ay->resolution/4;

	tp->hysteresis.margin.x = xmargin;
	tp->hysteresis.margin.y = ymargin;
	tp->hysteresis.enabled = (ax->fuzz || ay->fuzz);
	if (tp->hysteresis.enabled)
		evdev_log_debug(tp->device,
				"hysteresis enabled. "
				"See %s/touchpad-jitter.html for details\n",
				HTTP_DOC_LINK);
}

static void
tp_init_pressure(struct tp_dispatch *tp,
		 struct evdev_device *device)
{
	const struct input_absinfo *abs;
	unsigned int code;
	struct quirks_context *quirks;
	struct quirks *q;
	struct quirk_range r;
	int hi, lo;

	code = tp->has_mt ? ABS_MT_PRESSURE : ABS_PRESSURE;
	if (!libevdev_has_event_code(device->evdev, EV_ABS, code)) {
		tp->pressure.use_pressure = false;
		return;
	}

	abs = libevdev_get_abs_info(device->evdev, code);
	assert(abs);

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (q && quirks_get_range(q, QUIRK_ATTR_PRESSURE_RANGE, &r)) {
		hi = r.upper;
		lo = r.lower;

		if (hi == 0 && lo == 0) {
			evdev_log_info(device,
			       "pressure-based touch detection disabled\n");
			goto out;
		}
	} else {
		double range = absinfo_range(abs);

		/* Approximately the synaptics defaults */
		hi = abs->minimum + 0.12 * range;
		lo = abs->minimum + 0.10 * range;
	}

	if (hi > abs->maximum || hi < abs->minimum ||
	    lo > abs->maximum || lo < abs->minimum) {
		evdev_log_bug_libinput(device,
			       "discarding out-of-bounds pressure range %d:%d\n",
			       hi, lo);
		goto out;
	}

	tp->pressure.use_pressure = true;
	tp->pressure.high = hi;
	tp->pressure.low = lo;

	evdev_log_debug(device,
			"using pressure-based touch detection (%d:%d)\n",
			lo,
			hi);
out:
	quirks_unref(q);
}

static bool
tp_init_touch_size(struct tp_dispatch *tp,
		   struct evdev_device *device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	struct quirk_range r;
	int lo, hi;
	int rc = false;

	if (!libevdev_has_event_code(device->evdev,
				     EV_ABS,
				     ABS_MT_TOUCH_MAJOR)) {
		return false;
	}

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (q && quirks_get_range(q, QUIRK_ATTR_TOUCH_SIZE_RANGE, &r)) {
		hi = r.upper;
		lo = r.lower;
	} else {
		goto out;
	}

	if (libevdev_get_num_slots(device->evdev) < 5) {
		evdev_log_bug_libinput(device,
			       "Expected 5+ slots for touch size detection\n");
		goto out;
	}

	if (hi == 0 && lo == 0) {
		evdev_log_info(device,
			       "touch size based touch detection disabled\n");
		goto out;
	}

	/* Thresholds apply for both major or minor */
	tp->touch_size.low = lo;
	tp->touch_size.high = hi;
	tp->touch_size.use_touch_size = true;

	evdev_log_debug(device,
			"using size-based touch detection (%d:%d)\n",
			hi, lo);

	rc = true;
out:
	quirks_unref(q);
	return rc;
}

static void
tp_init_pressurepad(struct tp_dispatch *tp,
		    struct evdev_device *device)
{
	/* On traditional touchpads, the pressure value equals contact
	 * size. On PressurePads, pressure is a real physical axis for the
	 * force down. So we disable it here because we don't do anything
	 * with it anyway and using it for touch size messes things up.
	 *
	 * The kernel/udev set the resolution to non-zero on those devices
	 * to indicate that the value is in a known axis space.
	 *
	 * See also #562
	 */
	if (libevdev_get_abs_resolution(device->evdev, ABS_MT_PRESSURE) != 0 ||
	    evdev_device_has_model_quirk(device, QUIRK_MODEL_PRESSURE_PAD)) {
		libevdev_disable_event_code(device->evdev, EV_ABS, ABS_MT_PRESSURE);
		libevdev_disable_event_code(device->evdev, EV_ABS, ABS_PRESSURE);
	}
}

static int
tp_init(struct tp_dispatch *tp,
	struct evdev_device *device)
{
	bool use_touch_size = false;

	tp->base.dispatch_type = DISPATCH_TOUCHPAD;
	tp->base.interface = &tp_interface;
	tp->device = device;
	list_init(&tp->dwt.paired_keyboard_list);

	if (!tp_pass_sanity_check(tp, device))
		return false;

	tp_init_default_resolution(tp, device);
	tp_init_pressurepad(tp, device);

	if (!tp_init_slots(tp, device))
		return false;

	evdev_device_init_abs_range_warnings(device);
	use_touch_size = tp_init_touch_size(tp, device);

	if (!use_touch_size)
		tp_init_pressure(tp, device);

	/* 5 warnings per 24 hours should be enough */
	ratelimit_init(&tp->jump.warning, h2us(24), 5);

	/* Set the dpi to that of the x axis, because that's what we normalize
	   to when needed*/
	device->dpi = device->abs.absinfo_x->resolution * 25.4;

	tp_init_hysteresis(tp);

	if (!tp_init_accel(tp, LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE))
		return false;

	tp_init_tap(tp);
	tp_init_buttons(tp, device);
	tp_init_dwt(tp, device);
	tp_init_dwtp(tp, device);
	tp_init_palmdetect(tp, device);
	tp_init_sendevents(tp, device);
	tp_init_scroll(tp, device);
	tp_init_gesture(tp);
	tp_init_thumb(tp);

	/* Lenovo X1 Gen6 buffers the events in a weird way, making jump
	 * detection impossible. See
	 * https://gitlab.freedesktop.org/libinput/libinput/-/issues/506
	 */
	if (evdev_device_has_model_quirk(device,
					 QUIRK_MODEL_LENOVO_X1GEN6_TOUCHPAD))
		tp->jump.detection_disabled = true;

	device->seat_caps |= EVDEV_DEVICE_POINTER;
	if (tp->gesture.enabled)
		device->seat_caps |= EVDEV_DEVICE_GESTURE;

	return true;
}

static uint32_t
tp_sendevents_get_modes(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	uint32_t modes = LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;

	if (evdev->tags & EVDEV_TAG_INTERNAL_TOUCHPAD)
		modes |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;

	return modes;
}

static void
tp_suspend_conditional(struct tp_dispatch *tp,
		       struct evdev_device *device)
{
	struct libinput_device *dev;

	list_for_each(dev, &device->base.seat->devices_list, link) {
		struct evdev_device *d = evdev_device(dev);
		if (d->tags & EVDEV_TAG_EXTERNAL_MOUSE) {
			tp_suspend(tp, device, SUSPEND_EXTERNAL_MOUSE);
			break;
		}
	}
}

static enum libinput_config_status
tp_sendevents_set_mode(struct libinput_device *device,
		       enum libinput_config_send_events_mode mode)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	/* DISABLED overrides any DISABLED_ON_ */
	if ((mode & LIBINPUT_CONFIG_SEND_EVENTS_DISABLED) &&
	    (mode & LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE))
	    mode &= ~LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;

	if (mode == tp->sendevents.current_mode)
		return LIBINPUT_CONFIG_STATUS_SUCCESS;

	switch(mode) {
	case LIBINPUT_CONFIG_SEND_EVENTS_ENABLED:
		tp_resume(tp, evdev, SUSPEND_SENDEVENTS);
		tp_resume(tp, evdev, SUSPEND_EXTERNAL_MOUSE);
		break;
	case LIBINPUT_CONFIG_SEND_EVENTS_DISABLED:
		tp_suspend(tp, evdev, SUSPEND_SENDEVENTS);
		tp_resume(tp, evdev, SUSPEND_EXTERNAL_MOUSE);
		break;
	case LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE:
		tp_suspend_conditional(tp, evdev);
		tp_resume(tp, evdev, SUSPEND_SENDEVENTS);
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
	}

	tp->sendevents.current_mode = mode;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_send_events_mode
tp_sendevents_get_mode(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *dispatch = (struct tp_dispatch*)evdev->dispatch;

	return dispatch->sendevents.current_mode;
}

static enum libinput_config_send_events_mode
tp_sendevents_get_default_mode(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
}

static void
tp_change_to_left_handed(struct evdev_device *device)
{
	struct tp_dispatch *tp = (struct tp_dispatch *)device->dispatch;

	if (device->left_handed.want_enabled == device->left_handed.enabled)
		return;

	if (tp->buttons.state & 0x3) /* BTN_LEFT|BTN_RIGHT */
		return;

	/* tapping and clickfinger aren't affected by left-handed config,
	 * so checking physical buttons is enough */

	device->left_handed.enabled = device->left_handed.want_enabled;
	tp_change_rotation(device, DO_NOTIFY);
}

static bool
tp_requires_rotation(struct tp_dispatch *tp, struct evdev_device *device)
{
	bool rotate = false;
#if HAVE_LIBWACOM
	struct libinput *li = tp_libinput_context(tp);
	WacomDeviceDatabase *db = NULL;
	WacomDevice **devices = NULL,
		    **d;
	WacomDevice *dev;
	uint32_t vid = evdev_device_get_id_vendor(device),
		 pid = evdev_device_get_id_product(device);

	if ((device->tags & EVDEV_TAG_TABLET_TOUCHPAD) == 0)
		goto out;

	db = libinput_libwacom_ref(li);
	if (!db)
		goto out;

	/* Check if we have a device with the same vid/pid. If not,
	   we need to loop through all devices and check their paired
	   device. */
	dev = libwacom_new_from_usbid(db, vid, pid, NULL);
	if (dev) {
		rotate = libwacom_is_reversible(dev);
		libwacom_destroy(dev);
		goto out;
	}

	devices = libwacom_list_devices_from_database(db, NULL);
	if (!devices)
		goto out;
	d = devices;
	while(*d) {
		const WacomMatch *paired;

		paired = libwacom_get_paired_device(*d);
		if (paired &&
		    libwacom_match_get_vendor_id(paired) == vid &&
		    libwacom_match_get_product_id(paired) == pid) {
			rotate = libwacom_is_reversible(dev);
			break;
		}
		d++;
	}

	free(devices);

out:
	/* We don't need to keep it around for the touchpad, we're done with
	 * it until the device dies. */
	if (db)
		libinput_libwacom_unref(li);
#endif

	return rotate;
}

static void
tp_init_left_handed(struct tp_dispatch *tp,
		    struct evdev_device *device)
{
	bool want_left_handed = true;

	tp->left_handed.must_rotate = tp_requires_rotation(tp, device);

	if (device->model_flags & EVDEV_MODEL_APPLE_TOUCHPAD_ONEBUTTON)
		want_left_handed = false;
	if (want_left_handed)
		evdev_init_left_handed(device, tp_change_to_left_handed);

}

struct evdev_dispatch *
evdev_mt_touchpad_create(struct evdev_device *device)
{
	struct tp_dispatch *tp;

	evdev_tag_touchpad(device, device->udev_device);

	tp = zalloc(sizeof *tp);

	if (!tp_init(tp, device)) {
		tp_interface_destroy(&tp->base);
		return NULL;
	}

	device->base.config.sendevents = &tp->sendevents.config;

	tp->sendevents.current_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
	tp->sendevents.config.get_modes = tp_sendevents_get_modes;
	tp->sendevents.config.set_mode = tp_sendevents_set_mode;
	tp->sendevents.config.get_mode = tp_sendevents_get_mode;
	tp->sendevents.config.get_default_mode = tp_sendevents_get_default_mode;

	tp_init_left_handed(tp, device);

	return &tp->base;
}
