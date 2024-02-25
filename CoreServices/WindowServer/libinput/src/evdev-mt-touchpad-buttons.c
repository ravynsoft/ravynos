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

#include <limits.h>
#include <math.h>
#include <string.h>
#include "linux/input.h"

#include "util-input-event.h"
#include "evdev-mt-touchpad.h"

#define DEFAULT_BUTTON_ENTER_TIMEOUT ms2us(100)
#define DEFAULT_BUTTON_LEAVE_TIMEOUT ms2us(300)

/*****************************************
 * BEFORE YOU EDIT THIS FILE, look at the state diagram in
 * doc/touchpad-softbutton-state-machine.svg (generated with
 * https://www.diagrams.net).
 * Any changes in this file must be represented in the diagram.
 *
 * The state machine only affects the soft button area code.
 */

static inline const char*
button_state_to_str(enum button_state state)
{
	switch(state) {
	CASE_RETURN_STRING(BUTTON_STATE_NONE);
	CASE_RETURN_STRING(BUTTON_STATE_AREA);
	CASE_RETURN_STRING(BUTTON_STATE_BOTTOM);
	CASE_RETURN_STRING(BUTTON_STATE_TOP);
	CASE_RETURN_STRING(BUTTON_STATE_TOP_NEW);
	CASE_RETURN_STRING(BUTTON_STATE_TOP_TO_IGNORE);
	CASE_RETURN_STRING(BUTTON_STATE_IGNORE);
	}
	return NULL;
}

static inline const char*
button_event_to_str(enum button_event event)
{
	switch(event) {
	CASE_RETURN_STRING(BUTTON_EVENT_IN_BOTTOM_R);
	CASE_RETURN_STRING(BUTTON_EVENT_IN_BOTTOM_M);
	CASE_RETURN_STRING(BUTTON_EVENT_IN_BOTTOM_L);
	CASE_RETURN_STRING(BUTTON_EVENT_IN_TOP_R);
	CASE_RETURN_STRING(BUTTON_EVENT_IN_TOP_M);
	CASE_RETURN_STRING(BUTTON_EVENT_IN_TOP_L);
	CASE_RETURN_STRING(BUTTON_EVENT_IN_AREA);
	CASE_RETURN_STRING(BUTTON_EVENT_UP);
	CASE_RETURN_STRING(BUTTON_EVENT_PRESS);
	CASE_RETURN_STRING(BUTTON_EVENT_RELEASE);
	CASE_RETURN_STRING(BUTTON_EVENT_TIMEOUT);
	}
	return NULL;
}

static inline bool
is_inside_bottom_button_area(const struct tp_dispatch *tp,
			     const struct tp_touch *t)
{
	return t->point.y >= tp->buttons.bottom_area.top_edge;
}

static inline bool
is_inside_bottom_right_area(const struct tp_dispatch *tp,
			    const struct tp_touch *t)
{
	return is_inside_bottom_button_area(tp, t) &&
	       t->point.x > tp->buttons.bottom_area.rightbutton_left_edge;
}

static inline bool
is_inside_bottom_middle_area(const struct tp_dispatch *tp,
			   const struct tp_touch *t)
{
	return is_inside_bottom_button_area(tp, t) &&
	       !is_inside_bottom_right_area(tp, t) &&
	       t->point.x > tp->buttons.bottom_area.middlebutton_left_edge;
}

static inline bool
is_inside_top_button_area(const struct tp_dispatch *tp,
			  const struct tp_touch *t)
{
	return t->point.y <= tp->buttons.top_area.bottom_edge;
}

static inline bool
is_inside_top_right_area(const struct tp_dispatch *tp,
			 const struct tp_touch *t)
{
	return is_inside_top_button_area(tp, t) &&
	       t->point.x > tp->buttons.top_area.rightbutton_left_edge;
}

static inline bool
is_inside_top_middle_area(const struct tp_dispatch *tp,
			  const struct tp_touch *t)
{
	return is_inside_top_button_area(tp, t) &&
	       t->point.x >= tp->buttons.top_area.leftbutton_right_edge &&
	       t->point.x <= tp->buttons.top_area.rightbutton_left_edge;
}

static void
tp_button_set_enter_timer(struct tp_dispatch *tp,
			  struct tp_touch *t,
			  uint64_t time)
{
	libinput_timer_set(&t->button.timer,
			   time + DEFAULT_BUTTON_ENTER_TIMEOUT);
}

static void
tp_button_set_leave_timer(struct tp_dispatch *tp,
			  struct tp_touch *t,
			  uint64_t time)
{
	libinput_timer_set(&t->button.timer,
			   time + DEFAULT_BUTTON_LEAVE_TIMEOUT);
}

/*
 * tp_button_set_state, change state and implement on-entry behavior
 * as described in the state machine diagram.
 */
static void
tp_button_set_state(struct tp_dispatch *tp,
		    struct tp_touch *t,
		    enum button_state new_state,
		    enum button_event event,
		    uint64_t time)
{
	libinput_timer_cancel(&t->button.timer);

	t->button.state = new_state;

	switch (t->button.state) {
	case BUTTON_STATE_NONE:
		t->button.current = 0;
		break;
	case BUTTON_STATE_AREA:
		t->button.current = BUTTON_EVENT_IN_AREA;
		break;
	case BUTTON_STATE_BOTTOM:
		t->button.current = event;
		break;
	case BUTTON_STATE_TOP:
		break;
	case BUTTON_STATE_TOP_NEW:
		t->button.current = event;
		tp_button_set_enter_timer(tp, t, time);
		break;
	case BUTTON_STATE_TOP_TO_IGNORE:
		tp_button_set_leave_timer(tp, t, time);
		break;
	case BUTTON_STATE_IGNORE:
		t->button.current = 0;
		break;
	}
}

static void
tp_button_none_handle_event(struct tp_dispatch *tp,
			    struct tp_touch *t,
			    enum button_event event,
			    uint64_t time)
{
	switch (event) {
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
		tp_button_set_state(tp, t, BUTTON_STATE_BOTTOM, event, time);
		break;
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
		tp_button_set_state(tp, t, BUTTON_STATE_TOP_NEW, event, time);
		break;
	case BUTTON_EVENT_IN_AREA:
		tp_button_set_state(tp, t, BUTTON_STATE_AREA, event, time);
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
	case BUTTON_EVENT_RELEASE:
	case BUTTON_EVENT_TIMEOUT:
		break;
	}
}

static void
tp_button_area_handle_event(struct tp_dispatch *tp,
			    struct tp_touch *t,
			    enum button_event event,
			    uint64_t time)
{
	switch (event) {
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
	case BUTTON_EVENT_IN_AREA:
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
	case BUTTON_EVENT_RELEASE:
	case BUTTON_EVENT_TIMEOUT:
		break;
	}
}

/**
 * Release any button in the bottom area, provided it started within a
 * threshold around start_time (i.e. simultaneously with the other touch
 * that triggered this call).
 */
static inline void
tp_button_release_other_bottom_touches(struct tp_dispatch *tp,
				       uint64_t other_start_time)
{
	struct tp_touch *t;

	tp_for_each_touch(tp, t) {
		uint64_t tdelta;

		if (t->button.state != BUTTON_STATE_BOTTOM ||
		    t->button.has_moved)
			continue;

		if (other_start_time > t->button.initial_time)
			tdelta = other_start_time - t->button.initial_time;
		else
			tdelta = t->button.initial_time - other_start_time;

		if (tdelta > ms2us(80))
			continue;

		t->button.has_moved = true;
	}
}

static void
tp_button_bottom_handle_event(struct tp_dispatch *tp,
			      struct tp_touch *t,
			      enum button_event event,
			      uint64_t time)
{
	switch (event) {
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
		if (event != t->button.current)
			tp_button_set_state(tp,
					    t,
					    BUTTON_STATE_BOTTOM,
					    event,
					    time);
		break;
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
	case BUTTON_EVENT_IN_AREA:
		tp_button_set_state(tp, t, BUTTON_STATE_AREA, event, time);

		/* We just transitioned one finger from BOTTOM to AREA,
		 * if there are other fingers in BOTTOM that started
		 * simultaneously with this finger, release those fingers
		 * because they're part of a gesture.
		 */
		tp_button_release_other_bottom_touches(tp,
						       t->button.initial_time);
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
	case BUTTON_EVENT_RELEASE:
	case BUTTON_EVENT_TIMEOUT:
		break;
	}
}

static void
tp_button_top_handle_event(struct tp_dispatch *tp,
			   struct tp_touch *t,
			   enum button_event event,
			   uint64_t time)
{
	switch (event) {
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
		tp_button_set_state(tp, t, BUTTON_STATE_TOP_TO_IGNORE, event, time);
		break;
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
		if (event != t->button.current)
			tp_button_set_state(tp,
					    t,
					    BUTTON_STATE_TOP_NEW,
					    event,
					    time);
		break;
	case BUTTON_EVENT_IN_AREA:
		tp_button_set_state(tp, t, BUTTON_STATE_TOP_TO_IGNORE, event, time);
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
	case BUTTON_EVENT_RELEASE:
	case BUTTON_EVENT_TIMEOUT:
		break;
	}
}

static void
tp_button_top_new_handle_event(struct tp_dispatch *tp,
			       struct tp_touch *t,
			       enum button_event event,
			       uint64_t time)
{
	switch(event) {
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
		tp_button_set_state(tp, t, BUTTON_STATE_AREA, event, time);
		break;
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
		if (event != t->button.current)
			tp_button_set_state(tp,
					    t,
					    BUTTON_STATE_TOP_NEW,
					    event,
					    time);
		break;
	case BUTTON_EVENT_IN_AREA:
		tp_button_set_state(tp, t, BUTTON_STATE_AREA, event, time);
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
		tp_button_set_state(tp, t, BUTTON_STATE_TOP, event, time);
		break;
	case BUTTON_EVENT_RELEASE:
		break;
	case BUTTON_EVENT_TIMEOUT:
		tp_button_set_state(tp, t, BUTTON_STATE_TOP, event, time);
		break;
	}
}

static void
tp_button_top_to_ignore_handle_event(struct tp_dispatch *tp,
				     struct tp_touch *t,
				     enum button_event event,
				     uint64_t time)
{
	switch(event) {
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
		if (event == t->button.current)
			tp_button_set_state(tp,
					    t,
					    BUTTON_STATE_TOP,
					    event,
					    time);
		else
			tp_button_set_state(tp,
					    t,
					    BUTTON_STATE_TOP_NEW,
					    event,
					    time);
		break;
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
	case BUTTON_EVENT_IN_AREA:
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
	case BUTTON_EVENT_RELEASE:
		break;
	case BUTTON_EVENT_TIMEOUT:
		tp_button_set_state(tp, t, BUTTON_STATE_IGNORE, event, time);
		break;
	}
}

static void
tp_button_ignore_handle_event(struct tp_dispatch *tp,
			      struct tp_touch *t,
			      enum button_event event,
			      uint64_t time)
{
	switch (event) {
	case BUTTON_EVENT_IN_BOTTOM_R:
	case BUTTON_EVENT_IN_BOTTOM_M:
	case BUTTON_EVENT_IN_BOTTOM_L:
	case BUTTON_EVENT_IN_TOP_R:
	case BUTTON_EVENT_IN_TOP_M:
	case BUTTON_EVENT_IN_TOP_L:
	case BUTTON_EVENT_IN_AREA:
		break;
	case BUTTON_EVENT_UP:
		tp_button_set_state(tp, t, BUTTON_STATE_NONE, event, time);
		break;
	case BUTTON_EVENT_PRESS:
		t->button.current = BUTTON_EVENT_IN_AREA;
		break;
	case BUTTON_EVENT_RELEASE:
		break;
	case BUTTON_EVENT_TIMEOUT:
		break;
	}
}

static void
tp_button_handle_event(struct tp_dispatch *tp,
		       struct tp_touch *t,
		       enum button_event event,
		       uint64_t time)
{
	enum button_state current = t->button.state;

	switch(t->button.state) {
	case BUTTON_STATE_NONE:
		tp_button_none_handle_event(tp, t, event, time);
		break;
	case BUTTON_STATE_AREA:
		tp_button_area_handle_event(tp, t, event, time);
		break;
	case BUTTON_STATE_BOTTOM:
		tp_button_bottom_handle_event(tp, t, event, time);
		break;
	case BUTTON_STATE_TOP:
		tp_button_top_handle_event(tp, t, event, time);
		break;
	case BUTTON_STATE_TOP_NEW:
		tp_button_top_new_handle_event(tp, t, event, time);
		break;
	case BUTTON_STATE_TOP_TO_IGNORE:
		tp_button_top_to_ignore_handle_event(tp, t, event, time);
		break;
	case BUTTON_STATE_IGNORE:
		tp_button_ignore_handle_event(tp, t, event, time);
		break;
	}

	if (current != t->button.state)
		evdev_log_debug(tp->device,
				"button state: touch %d from %-20s event %-24s to %-20s\n",
				t->index,
				button_state_to_str(current),
				button_event_to_str(event),
				button_state_to_str(t->button.state));
}

static inline void
tp_button_check_for_movement(struct tp_dispatch *tp, struct tp_touch *t)
{
	struct device_coords delta;
	struct phys_coords mm;
	double vector_length;

	if (t->button.has_moved)
		return;

	switch (t->button.state) {
	case BUTTON_STATE_NONE:
	case BUTTON_STATE_AREA:
	case BUTTON_STATE_TOP:
	case BUTTON_STATE_TOP_NEW:
	case BUTTON_STATE_TOP_TO_IGNORE:
	case BUTTON_STATE_IGNORE:
		/* No point calculating if we're not going to use it */
		return;
	case BUTTON_STATE_BOTTOM:
		break;
	}

	delta.x = t->point.x - t->button.initial.x;
	delta.y = t->point.y - t->button.initial.y;
	mm = evdev_device_unit_delta_to_mm(tp->device, &delta);
	vector_length = hypot(mm.x, mm.y);

	if (vector_length > 5.0 /* mm */) {
		t->button.has_moved = true;

		tp_button_release_other_bottom_touches(tp,
						       t->button.initial_time);
	}
}

void
tp_button_handle_state(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;

	tp_for_each_touch(tp, t) {
		if (t->state == TOUCH_NONE || t->state == TOUCH_HOVERING)
			continue;

		if (t->state == TOUCH_BEGIN) {
			t->button.initial = t->point;
			t->button.initial_time = time;
			t->button.has_moved = false;
		}

		if (t->state == TOUCH_END) {
			tp_button_handle_event(tp, t, BUTTON_EVENT_UP, time);
		} else if (t->dirty) {
			enum button_event event;

			if (is_inside_bottom_button_area(tp, t)) {
				if (is_inside_bottom_right_area(tp, t))
					event = BUTTON_EVENT_IN_BOTTOM_R;
				else if (is_inside_bottom_middle_area(tp, t))
					event = BUTTON_EVENT_IN_BOTTOM_M;
				else
					event = BUTTON_EVENT_IN_BOTTOM_L;

				/* In the bottom area we check for movement
				 * within the area. Top area - meh */
				tp_button_check_for_movement(tp, t);
			} else if (is_inside_top_button_area(tp, t)) {
				if (is_inside_top_right_area(tp, t))
					event = BUTTON_EVENT_IN_TOP_R;
				else if (is_inside_top_middle_area(tp, t))
					event = BUTTON_EVENT_IN_TOP_M;
				else
					event = BUTTON_EVENT_IN_TOP_L;
			} else {
				event = BUTTON_EVENT_IN_AREA;
			}

			tp_button_handle_event(tp, t, event, time);
		}
		if (tp->queued & TOUCHPAD_EVENT_BUTTON_RELEASE)
			tp_button_handle_event(tp, t, BUTTON_EVENT_RELEASE, time);
		if (tp->queued & TOUCHPAD_EVENT_BUTTON_PRESS)
			tp_button_handle_event(tp, t, BUTTON_EVENT_PRESS, time);
	}
}

static void
tp_button_handle_timeout(uint64_t now, void *data)
{
	struct tp_touch *t = data;

	tp_button_handle_event(t->tp, t, BUTTON_EVENT_TIMEOUT, now);
}

void
tp_process_button(struct tp_dispatch *tp,
		  const struct input_event *e,
		  uint64_t time)
{
	uint32_t mask = bit(e->code - BTN_LEFT);

	/* Ignore other buttons on clickpads */
	if (tp->buttons.is_clickpad && e->code != BTN_LEFT) {
		evdev_log_bug_kernel(tp->device,
				     "received %s button event on a clickpad\n",
				     libevdev_event_code_get_name(EV_KEY, e->code));
		return;
	}

	if (e->value) {
		tp->buttons.state |= mask;
		tp->queued |= TOUCHPAD_EVENT_BUTTON_PRESS;
	} else {
		tp->buttons.state &= ~mask;
		tp->queued |= TOUCHPAD_EVENT_BUTTON_RELEASE;
	}
}

void
tp_release_all_buttons(struct tp_dispatch *tp,
		       uint64_t time)
{
	if (tp->buttons.state) {
		tp->buttons.state = 0;
		tp->queued |= TOUCHPAD_EVENT_BUTTON_RELEASE;
	}
}

static void
tp_init_softbuttons(struct tp_dispatch *tp,
		    struct evdev_device *device)
{
	double width, height;
	struct device_coords edges;
	int mb_le, mb_re; /* middle button left/right edge */
	struct phys_coords mm = { 0.0, 0.0 };

	evdev_device_get_size(device, &width, &height);

	/* button height: 10mm or 15% or the touchpad height,
	   whichever is smaller */
	if (height * 0.15 > 10)
		mm.y = height - 10;
	else
		mm.y = height * 0.85;

	mm.x = width * 0.5;
	edges = evdev_device_mm_to_units(device, &mm);
	tp->buttons.bottom_area.top_edge = edges.y;
	tp->buttons.bottom_area.rightbutton_left_edge = edges.x;

	tp->buttons.bottom_area.middlebutton_left_edge = INT_MAX;

	/* if middlebutton emulation is enabled, don't init a software area */
	if (device->middlebutton.want_enabled)
		return;

	/* The middle button is 25% of the touchpad and centered. Many
	 * touchpads don't have markings for the middle button at all so we
	 * need to make it big enough to reliably hit it but not too big so
	 * it takes away all the space.
	 *
	 * On touchpads with visible markings we reduce the size of the
	 * middle button since users have a visual guide.
	 */
	if (evdev_device_has_model_quirk(device,
					 QUIRK_MODEL_TOUCHPAD_VISIBLE_MARKER)) {
		mm.x = width/2 - 5; /* 10mm wide */
		edges = evdev_device_mm_to_units(device, &mm);
		mb_le = edges.x;

		mm.x = width/2 + 5; /* 10mm wide */
		edges = evdev_device_mm_to_units(device, &mm);
		mb_re = edges.x;
	} else {
		mm.x = width * 0.375;
		edges = evdev_device_mm_to_units(device, &mm);
		mb_le = edges.x;

		mm.x = width * 0.625;
		edges = evdev_device_mm_to_units(device, &mm);
		mb_re = edges.x;
	}

	tp->buttons.bottom_area.middlebutton_left_edge = mb_le;
	tp->buttons.bottom_area.rightbutton_left_edge = mb_re;
}

void
tp_init_top_softbuttons(struct tp_dispatch *tp,
			struct evdev_device *device,
			double topbutton_size_mult)
{
	struct device_coords edges;

	if (tp->buttons.has_topbuttons) {
		/* T440s has the top button line 5mm from the top, event
		   analysis has shown events to start down to ~10mm from the
		   top - which maps to 15%.  We allow the caller to enlarge the
		   area using a multiplier for the touchpad disabled case. */
		double topsize_mm = 10 * topbutton_size_mult;
		struct phys_coords mm;
		double width, height;

		evdev_device_get_size(device, &width, &height);

		mm.x = width * 0.60;
		mm.y = topsize_mm;
		edges = evdev_device_mm_to_units(device, &mm);
		tp->buttons.top_area.bottom_edge = edges.y;
		tp->buttons.top_area.rightbutton_left_edge = edges.x;

		mm.x = width * 0.40;
		edges = evdev_device_mm_to_units(device, &mm);
		tp->buttons.top_area.leftbutton_right_edge = edges.x;
	} else {
		tp->buttons.top_area.bottom_edge = INT_MIN;
	}
}

static inline uint32_t
tp_button_config_click_get_methods(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;
	uint32_t methods = LIBINPUT_CONFIG_CLICK_METHOD_NONE;

	if (tp->buttons.is_clickpad) {
		methods |= LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;
		if (tp->has_mt)
			methods |= LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER;
	}

	if (evdev->model_flags & EVDEV_MODEL_APPLE_TOUCHPAD_ONEBUTTON)
		methods |= LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER;

	return methods;
}

static void
tp_switch_click_method(struct tp_dispatch *tp)
{
	/*
	 * All we need to do when switching click methods is to change the
	 * bottom_area.top_edge so that when in clickfinger mode the bottom
	 * touchpad area is not dead wrt finger movement starting there.
	 *
	 * We do not need to take any state into account, fingers which are
	 * already down will simply keep the state / area they have assigned
	 * until they are released, and the post_button_events path is state
	 * agnostic.
	 */

	switch (tp->buttons.click_method) {
	case LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS:
		tp_init_softbuttons(tp, tp->device);
		break;
	case LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER:
	case LIBINPUT_CONFIG_CLICK_METHOD_NONE:
		tp->buttons.bottom_area.top_edge = INT_MAX;
		break;
	}
}

static enum libinput_config_status
tp_button_config_click_set_method(struct libinput_device *device,
				  enum libinput_config_click_method method)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	tp->buttons.click_method = method;
	tp_switch_click_method(tp);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_click_method
tp_button_config_click_get_method(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp->buttons.click_method;
}

static enum libinput_config_click_method
tp_click_get_default_method(struct tp_dispatch *tp)
{
	struct evdev_device *device = tp->device;

	if (evdev_device_has_model_quirk(device, QUIRK_MODEL_CHROMEBOOK) ||
	    evdev_device_has_model_quirk(device, QUIRK_MODEL_SYSTEM76_BONOBO) ||
	    evdev_device_has_model_quirk(device, QUIRK_MODEL_SYSTEM76_GALAGO) ||
	    evdev_device_has_model_quirk(device, QUIRK_MODEL_SYSTEM76_KUDU) ||
	    evdev_device_has_model_quirk(device, QUIRK_MODEL_CLEVO_W740SU) ||
	    evdev_device_has_model_quirk(device, QUIRK_MODEL_APPLE_TOUCHPAD_ONEBUTTON))
		return LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER;

	if (!tp->buttons.is_clickpad)
		return LIBINPUT_CONFIG_CLICK_METHOD_NONE;

	if (evdev_device_has_model_quirk(device, QUIRK_MODEL_APPLE_TOUCHPAD))
		return LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER;

	return LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;
}

static enum libinput_config_click_method
tp_button_config_click_get_default_method(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct tp_dispatch *tp = (struct tp_dispatch*)evdev->dispatch;

	return tp_click_get_default_method(tp);
}

void
tp_clickpad_middlebutton_apply_config(struct evdev_device *device)
{
	struct tp_dispatch *tp = (struct tp_dispatch*)device->dispatch;

	if (!tp->buttons.is_clickpad ||
	    tp->buttons.state != 0)
		return;

	if (device->middlebutton.want_enabled ==
	    device->middlebutton.enabled)
		return;

	device->middlebutton.enabled = device->middlebutton.want_enabled;
	if (tp->buttons.click_method ==
	    LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS)
		tp_init_softbuttons(tp, device);
}

static int
tp_clickpad_middlebutton_is_available(struct libinput_device *device)
{
	return evdev_middlebutton_is_available(device);
}

static enum libinput_config_status
tp_clickpad_middlebutton_set(struct libinput_device *device,
		     enum libinput_config_middle_emulation_state enable)
{
	struct evdev_device *evdev = evdev_device(device);

	switch (enable) {
	case LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED:
		evdev->middlebutton.want_enabled = true;
		break;
	case LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED:
		evdev->middlebutton.want_enabled = false;
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	tp_clickpad_middlebutton_apply_config(evdev);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_middle_emulation_state
tp_clickpad_middlebutton_get(struct libinput_device *device)
{
	return evdev_middlebutton_get(device);
}

static enum libinput_config_middle_emulation_state
tp_clickpad_middlebutton_get_default(struct libinput_device *device)
{
	return evdev_middlebutton_get_default(device);
}

static inline void
tp_init_clickpad_middlebutton_emulation(struct tp_dispatch *tp,
					struct evdev_device *device)
{
	device->middlebutton.enabled_default = false;
	device->middlebutton.want_enabled = false;
	device->middlebutton.enabled = false;

	device->middlebutton.config.available = tp_clickpad_middlebutton_is_available;
	device->middlebutton.config.set = tp_clickpad_middlebutton_set;
	device->middlebutton.config.get = tp_clickpad_middlebutton_get;
	device->middlebutton.config.get_default = tp_clickpad_middlebutton_get_default;
	device->base.config.middle_emulation = &device->middlebutton.config;
}

static inline void
tp_init_middlebutton_emulation(struct tp_dispatch *tp,
			       struct evdev_device *device)
{
	bool enable_by_default,
	     want_config_option;

	/* On clickpads we provide the config option but disable by default.
	   When enabled, the middle software button disappears */
	if (tp->buttons.is_clickpad) {
		tp_init_clickpad_middlebutton_emulation(tp, device);
		return;
	}

	/* init middle button emulation on non-clickpads, but only if we
	 * don't have a middle button. Exception: ALPS touchpads don't know
	 * if they have a middle button, so we always want the option there
	 * and enabled by default.
	 */
	if (!libevdev_has_event_code(device->evdev, EV_KEY, BTN_MIDDLE)) {
		enable_by_default = true;
		want_config_option = false;
	} else if (evdev_device_has_model_quirk(device,
						QUIRK_MODEL_ALPS_SERIAL_TOUCHPAD)) {
		enable_by_default = true;
		want_config_option = true;
	} else
		return;

	evdev_init_middlebutton(tp->device,
				enable_by_default,
				want_config_option);
}

static bool
tp_guess_clickpad(const struct tp_dispatch *tp, struct evdev_device *device)
{
	bool is_clickpad;
	bool has_left = libevdev_has_event_code(device->evdev, EV_KEY, BTN_LEFT),
	     has_middle = libevdev_has_event_code(device->evdev, EV_KEY, BTN_MIDDLE),
	     has_right = libevdev_has_event_code(device->evdev, EV_KEY, BTN_RIGHT);

	is_clickpad = libevdev_has_property(device->evdev, INPUT_PROP_BUTTONPAD);

	/* A non-clickpad without a right button is a clickpad, assume the
	 * kernel is wrong.
	 * Exceptions here:
	 * - The one-button Apple touchpad (discontinued in 2008) has a
	 *   single physical button
	 * - Wacom touch devices have neither left nor right buttons
	 */
	if (!is_clickpad && has_left && !has_right &&
	    (tp->device->model_flags & EVDEV_MODEL_APPLE_TOUCHPAD_ONEBUTTON) == 0) {
		evdev_log_bug_kernel(device,
				     "missing right button, assuming it is a clickpad.\n");
		is_clickpad = true;
	}

	if (has_middle || has_right) {
		if (is_clickpad)
			evdev_log_bug_kernel(device,
					     "clickpad advertising right button. "
					     "See %s/clickpad-with-right-button.html for details\n",
					     HTTP_DOC_LINK);
	} else if (has_left &
		   !is_clickpad &&
		   libevdev_get_id_vendor(device->evdev) != VENDOR_ID_APPLE) {
			evdev_log_bug_kernel(device,
					     "non clickpad without right button?\n");
	}

	return is_clickpad;
}

void
tp_init_buttons(struct tp_dispatch *tp,
		struct evdev_device *device)
{
	struct tp_touch *t;
	const struct input_absinfo *absinfo_x, *absinfo_y;
	int i;

	tp->buttons.is_clickpad = tp_guess_clickpad(tp, device);

	tp->buttons.has_topbuttons = libevdev_has_property(device->evdev,
						        INPUT_PROP_TOPBUTTONPAD);

	absinfo_x = device->abs.absinfo_x;
	absinfo_y = device->abs.absinfo_y;

	/* pinned-finger motion threshold, see tp_unpin_finger. */
	tp->buttons.motion_dist.x_scale_coeff = 1.0/absinfo_x->resolution;
	tp->buttons.motion_dist.y_scale_coeff = 1.0/absinfo_y->resolution;

	tp->buttons.config_method.get_methods = tp_button_config_click_get_methods;
	tp->buttons.config_method.set_method = tp_button_config_click_set_method;
	tp->buttons.config_method.get_method = tp_button_config_click_get_method;
	tp->buttons.config_method.get_default_method = tp_button_config_click_get_default_method;
	tp->device->base.config.click_method = &tp->buttons.config_method;

	tp->buttons.click_method = tp_click_get_default_method(tp);
	tp_switch_click_method(tp);

	tp_init_top_softbuttons(tp, device, 1.0);

	tp_init_middlebutton_emulation(tp, device);

	i = 0;
	tp_for_each_touch(tp, t) {
		char timer_name[64];
		i++;

		snprintf(timer_name,
			 sizeof(timer_name),
			 "%s (%d) button",
			 evdev_device_get_sysname(device),
			 i);
		t->button.state = BUTTON_STATE_NONE;
		libinput_timer_init(&t->button.timer,
				    tp_libinput_context(tp),
				    timer_name,
				    tp_button_handle_timeout, t);
	}
}

void
tp_remove_buttons(struct tp_dispatch *tp)
{
	struct tp_touch *t;

	tp_for_each_touch(tp, t) {
		libinput_timer_cancel(&t->button.timer);
		libinput_timer_destroy(&t->button.timer);
	}
}

static int
tp_post_physical_buttons(struct tp_dispatch *tp, uint64_t time)
{
	uint32_t current, old, button;

	current = tp->buttons.state;
	old = tp->buttons.old_state;
	button = BTN_LEFT;

	while (current || old) {
		enum libinput_button_state state;

		if ((current & 0x1) ^ (old & 0x1)) {
			uint32_t b;

			if (!!(current & 0x1))
				state = LIBINPUT_BUTTON_STATE_PRESSED;
			else
				state = LIBINPUT_BUTTON_STATE_RELEASED;

			b = evdev_to_left_handed(tp->device, button);
			evdev_pointer_notify_physical_button(tp->device,
							     time,
							     b,
							     state);
		}

		button++;
		current >>= 1;
		old >>= 1;
	}

	return 0;
}

static inline bool
tp_clickfinger_within_distance(struct tp_dispatch *tp,
			       struct tp_touch *t1,
			       struct tp_touch *t2)
{
	double x, y;
	bool within_distance = false;
	int xres, yres;
	int bottom_threshold;

	if (!t1 || !t2)
		return 0;

	if (tp_thumb_ignored(tp, t1) || tp_thumb_ignored(tp, t2))
		return 0;

	x = abs(t1->point.x - t2->point.x);
	y = abs(t1->point.y - t2->point.y);

	xres = tp->device->abs.absinfo_x->resolution;
	yres = tp->device->abs.absinfo_y->resolution;
	x /= xres;
	y /= yres;

	/* maximum horiz spread is 40mm horiz, 30mm vert, anything wider
	 * than that is probably a gesture. */
	if (x > 40 || y > 30)
		goto out;

	within_distance = true;

	/* if y spread is <= 20mm, they're definitely together. */
	if (y <= 20)
		goto out;

	/* if they're vertically spread between 20-40mm, they're not
	 * together if:
	 * - the touchpad's vertical size is >50mm, anything smaller is
	 *   unlikely to have a thumb resting on it
	 * - and one of the touches is in the bottom 20mm of the touchpad
	 *   and the other one isn't
	 */

	if (tp->device->abs.dimensions.y/yres < 50)
		goto out;

	bottom_threshold = tp->device->abs.absinfo_y->maximum - 20 * yres;
	if ((t1->point.y > bottom_threshold) !=
		    (t2->point.y > bottom_threshold))
		within_distance = 0;

out:
	return within_distance;
}

static uint32_t
tp_clickfinger_set_button(struct tp_dispatch *tp)
{
	uint32_t button;
	unsigned int nfingers = 0;
	struct tp_touch *t;
	struct tp_touch *first = NULL,
			*second = NULL;

	tp_for_each_touch(tp, t) {
		if (t->state != TOUCH_BEGIN && t->state != TOUCH_UPDATE)
			continue;

		if (tp_thumb_ignored(tp, t))
			continue;

		if (t->palm.state != PALM_NONE)
			continue;

		nfingers++;

		if (!first)
			first = t;
		else if (!second)
			second = t;
	}

	/* Only check for finger distance when there are 2 fingers on the
	 * touchpad */
	if (nfingers != 2)
		goto out;

	if (tp_clickfinger_within_distance(tp, first, second))
		nfingers = 2;
	else
		nfingers = 1;

out:
	switch (nfingers) {
	case 0:
	case 1: button = BTN_LEFT; break;
	case 2: button = BTN_RIGHT; break;
	case 3: button = BTN_MIDDLE; break;
	default:
		button = 0;
		break;
	}

	return button;
}

static int
tp_notify_clickpadbutton(struct tp_dispatch *tp,
			 uint64_t time,
			 uint32_t button,
			 uint32_t is_topbutton,
			 enum libinput_button_state state)
{
	/* If we've a trackpoint, send top buttons through the trackpoint */
	if (tp->buttons.trackpoint) {
		if (is_topbutton) {
			struct evdev_dispatch *dispatch = tp->buttons.trackpoint->dispatch;
			struct input_event event, syn_report;
			int value;

			value = (state == LIBINPUT_BUTTON_STATE_PRESSED) ? 1 : 0;
			event = input_event_init(time, EV_KEY, button, value);
			syn_report = input_event_init(time, EV_SYN, SYN_REPORT, 0);
			dispatch->interface->process(dispatch,
						     tp->buttons.trackpoint,
						     &event,
						     time);
			dispatch->interface->process(dispatch,
						     tp->buttons.trackpoint,
						     &syn_report,
						     time);
			return 1;
		}
		/* Ignore button events not for the trackpoint while suspended */
		if (tp->device->is_suspended)
			return 0;
	}

	/* A button click always terminates edge scrolling, even if we
	 * don't end up sending a button event. */
	tp_edge_scroll_stop_events(tp, time);

	/*
	 * If the user has requested clickfinger replace the button chosen
	 * by the softbutton code with one based on the number of fingers.
	 */
	if (tp->buttons.click_method == LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER &&
	    state == LIBINPUT_BUTTON_STATE_PRESSED) {
		button = tp_clickfinger_set_button(tp);
		tp->buttons.active = button;

		if (!button)
			return 0;
	}

	evdev_pointer_notify_button(tp->device, time, button, state);
	return 1;
}

static int
tp_post_clickpadbutton_buttons(struct tp_dispatch *tp, uint64_t time)
{
	uint32_t current, old, button, is_top;
	enum libinput_button_state state;
	enum { AREA = 0x01, LEFT = 0x02, MIDDLE = 0x04, RIGHT = 0x08 };
	bool want_left_handed = true;

	current = tp->buttons.state;
	old = tp->buttons.old_state;
	is_top = 0;

	if (!tp->buttons.click_pending && current == old)
		return 0;

	if (current) {
		struct tp_touch *t;
		uint32_t area = 0;

		if (evdev_device_has_model_quirk(tp->device,
						 QUIRK_MODEL_TOUCHPAD_PHANTOM_CLICKS) &&
		    tp->nactive_slots == 0) {
			/* Some touchpads, notably those on the Dell XPS 15 9500,
			 * are prone to registering touchpad clicks when the
			 * case is sufficiently flexed. Ignore these by
			 * disregarding any clicks that are registered without
			 * touchpad touch. */
			tp->buttons.click_pending = true;
			return 0;
		}

		tp_for_each_touch(tp, t) {
			switch (t->button.current) {
			case BUTTON_EVENT_IN_AREA:
				area |= AREA;
				break;
			case BUTTON_EVENT_IN_TOP_L:
				is_top = 1;
				_fallthrough_;
			case BUTTON_EVENT_IN_BOTTOM_L:
				area |= LEFT;
				break;
			case BUTTON_EVENT_IN_TOP_M:
				is_top = 1;
				_fallthrough_;
			case BUTTON_EVENT_IN_BOTTOM_M:
				area |= MIDDLE;
				break;
			case BUTTON_EVENT_IN_TOP_R:
				is_top = 1;
				_fallthrough_;
			case BUTTON_EVENT_IN_BOTTOM_R:
				area |= RIGHT;
				break;
			default:
				break;
			}
		}

		if (area == 0 &&
		    tp->buttons.click_method != LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER) {
			/* No touches, wait for a touch before processing */
			tp->buttons.click_pending = true;
			return 0;
		}

		if ((tp->device->middlebutton.enabled || is_top) &&
		    (area & LEFT) && (area & RIGHT)) {
			button = BTN_MIDDLE;
		} else if (area & MIDDLE) {
			button = BTN_MIDDLE;
		} else if (area & RIGHT) {
			button = BTN_RIGHT;
		} else if (area & LEFT) {
			button = BTN_LEFT;
		} else { /* main or no area (for clickfinger) is always BTN_LEFT */
			button = BTN_LEFT;
			want_left_handed = false;
		}

		if (is_top)
			want_left_handed = false;

		if (want_left_handed)
			button = evdev_to_left_handed(tp->device, button);

		tp->buttons.active = button;
		tp->buttons.active_is_topbutton = is_top;
		state = LIBINPUT_BUTTON_STATE_PRESSED;
	} else {
		button = tp->buttons.active;
		is_top = tp->buttons.active_is_topbutton;
		tp->buttons.active = 0;
		tp->buttons.active_is_topbutton = 0;
		state = LIBINPUT_BUTTON_STATE_RELEASED;
	}

	tp->buttons.click_pending = false;

	if (button)
		return tp_notify_clickpadbutton(tp,
						time,
						button,
						is_top,
						state);
	return 0;
}

int
tp_post_button_events(struct tp_dispatch *tp, uint64_t time)
{
	if (tp->buttons.is_clickpad ||
	    tp->device->model_flags & EVDEV_MODEL_APPLE_TOUCHPAD_ONEBUTTON)
		return tp_post_clickpadbutton_buttons(tp, time);
	return tp_post_physical_buttons(tp, time);
}

bool
tp_button_touch_active(const struct tp_dispatch *tp,
		       const struct tp_touch *t)
{
	return t->button.state == BUTTON_STATE_AREA || t->button.has_moved;
}

bool
tp_button_is_inside_softbutton_area(const struct tp_dispatch *tp,
				    const struct tp_touch *t)
{
	return is_inside_top_button_area(tp, t) ||
	       is_inside_bottom_button_area(tp, t);
}
