/*
 * Copyright © 2019 Matt Mayfield
 * Copyright © 2019 Red Hat, Inc.
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
#include "evdev-mt-touchpad.h"

/* distance between fingers to assume it is not a scroll */
#define SCROLL_MM_X 35
#define SCROLL_MM_Y 25
#define THUMB_TIMEOUT ms2us(100)

static inline const char*
thumb_state_to_str(enum tp_thumb_state state)
{
	switch(state){
	CASE_RETURN_STRING(THUMB_STATE_FINGER);
	CASE_RETURN_STRING(THUMB_STATE_JAILED);
	CASE_RETURN_STRING(THUMB_STATE_PINCH);
	CASE_RETURN_STRING(THUMB_STATE_SUPPRESSED);
	CASE_RETURN_STRING(THUMB_STATE_REVIVED);
	CASE_RETURN_STRING(THUMB_STATE_REVIVED_JAILED);
	CASE_RETURN_STRING(THUMB_STATE_DEAD);
	}

	return NULL;
}

static void
tp_thumb_set_state(struct tp_dispatch *tp,
		   struct tp_touch *t,
		   enum tp_thumb_state state)
{
	unsigned int index = t ? t->index : UINT_MAX;

	if (tp->thumb.state == state && tp->thumb.index == index)
		return;

	evdev_log_debug(tp->device,
			"thumb: touch %d, %s → %s\n",
			(int)index,
			thumb_state_to_str(tp->thumb.state),
			thumb_state_to_str(state));

	tp->thumb.state = state;
	tp->thumb.index = index;
}

void
tp_thumb_reset(struct tp_dispatch *tp)
{
	tp->thumb.state = THUMB_STATE_FINGER;
	tp->thumb.index = UINT_MAX;
	tp->thumb.pinch_eligible = true;
}

static void
tp_thumb_lift(struct tp_dispatch *tp)
{
	tp->thumb.state = THUMB_STATE_FINGER;
	tp->thumb.index = UINT_MAX;
}

static bool
tp_thumb_in_exclusion_area(const struct tp_dispatch *tp,
			   const struct tp_touch *t)
{
	return (t->point.y > tp->thumb.lower_thumb_line &&
		tp->scroll.method != LIBINPUT_CONFIG_SCROLL_EDGE);

}

static bool
tp_thumb_detect_pressure_size(const struct tp_dispatch *tp,
			      const struct tp_touch *t)
{
	bool is_thumb = false;

	if (tp->thumb.use_pressure &&
	    t->pressure > tp->thumb.pressure_threshold &&
	    tp_thumb_in_exclusion_area(tp, t)) {
		is_thumb = true;
	}

	if (tp->thumb.use_size &&
	    (t->major > tp->thumb.size_threshold) &&
	    (t->minor < (tp->thumb.size_threshold * 0.6))) {
		is_thumb = true;
	}

	return is_thumb;
}

static bool
tp_thumb_needs_jail(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	if (t->point.y < tp->thumb.upper_thumb_line ||
	    tp->scroll.method == LIBINPUT_CONFIG_SCROLL_EDGE)
		return false;

	if (!tp_thumb_in_exclusion_area(tp, t) &&
           (tp->thumb.use_size || tp->thumb.use_pressure) &&
	    !tp_thumb_detect_pressure_size(tp, t))
		return false;

	if (t->speed.exceeded_count >= 10)
		return false;

	return true;
}

bool
tp_thumb_ignored(const struct tp_dispatch *tp, const struct tp_touch *t)
{
	return (tp->thumb.detect_thumbs &&
		tp->thumb.index == t->index &&
		(tp->thumb.state == THUMB_STATE_JAILED ||
		 tp->thumb.state == THUMB_STATE_PINCH ||
		 tp->thumb.state == THUMB_STATE_SUPPRESSED ||
		 tp->thumb.state == THUMB_STATE_REVIVED_JAILED ||
		 tp->thumb.state == THUMB_STATE_DEAD));
}

bool
tp_thumb_ignored_for_tap(const struct tp_dispatch *tp,
			 const struct tp_touch *t)
{
	return (tp->thumb.detect_thumbs &&
		tp->thumb.index == t->index &&
		(tp->thumb.state == THUMB_STATE_PINCH ||
		 tp->thumb.state == THUMB_STATE_SUPPRESSED ||
		 tp->thumb.state == THUMB_STATE_DEAD));
}

bool
tp_thumb_ignored_for_gesture(const struct tp_dispatch *tp,
			     const struct tp_touch *t)
{
	return (tp->thumb.detect_thumbs &&
		tp->thumb.index == t->index &&
		(tp->thumb.state == THUMB_STATE_JAILED ||
		 tp->thumb.state == THUMB_STATE_SUPPRESSED ||
		 tp->thumb.state == THUMB_STATE_REVIVED_JAILED ||
		 tp->thumb.state == THUMB_STATE_DEAD));
}

void
tp_thumb_suppress(struct tp_dispatch *tp, struct tp_touch *t)
{
	if(tp->thumb.state == THUMB_STATE_FINGER ||
	   tp->thumb.state == THUMB_STATE_JAILED ||
	   tp->thumb.state == THUMB_STATE_PINCH ||
	   tp->thumb.index != t->index) {
		tp_thumb_set_state(tp, t, THUMB_STATE_SUPPRESSED);
		return;
	}

	tp_thumb_set_state(tp, t, THUMB_STATE_DEAD);
}

static void
tp_thumb_pinch(struct tp_dispatch *tp, struct tp_touch *t)
{
	if(tp->thumb.state == THUMB_STATE_FINGER ||
	   tp->thumb.state == THUMB_STATE_JAILED ||
	   tp->thumb.index != t->index)
		tp_thumb_set_state(tp, t, THUMB_STATE_PINCH);
	else if (tp->thumb.state != THUMB_STATE_PINCH)
		tp_thumb_suppress(tp, t);
}

static void
tp_thumb_revive(struct tp_dispatch *tp, struct tp_touch *t)
{
	if((tp->thumb.state != THUMB_STATE_SUPPRESSED &&
	    tp->thumb.state != THUMB_STATE_PINCH) ||
	   (tp->thumb.index != t->index))
		return;

	if(tp_thumb_needs_jail(tp, t))
		tp_thumb_set_state(tp, t, THUMB_STATE_REVIVED_JAILED);
	else
		tp_thumb_set_state(tp, t, THUMB_STATE_REVIVED);
}

void
tp_thumb_update_touch(struct tp_dispatch *tp,
		      struct tp_touch *t,
		      uint64_t time)
{
	if (!tp->thumb.detect_thumbs)
		return;

	/* Once any active touch exceeds the speed threshold, don't
	 * try to detect pinches until all touches lift.
	 */
	if (t->speed.exceeded_count >= 10 &&
	    tp->thumb.pinch_eligible &&
	    tp->gesture.state == GESTURE_STATE_NONE) {
		tp->thumb.pinch_eligible = false;
		if(tp->thumb.state == THUMB_STATE_PINCH) {
			struct tp_touch *thumb;
			tp_for_each_touch(tp, thumb) {
				if (thumb->index != tp->thumb.index)
					continue;

				tp_thumb_set_state(tp, thumb, THUMB_STATE_SUPPRESSED);
				break;
			}
		}
	}

	/* Handle the thumb lifting off the touchpad */
	if (t->state == TOUCH_END && t->index == tp->thumb.index) {
		tp_thumb_lift(tp);
		return;
	}

	/* If this touch is not the only one, thumb updates happen by context
	 * instead of here
	 */
	if (tp->nfingers_down > 1)
		return;

	/* If we arrived here by other fingers lifting off, revive current touch
	 * if appropriate
	 */
	tp_thumb_revive(tp, t);

	/* First new touch below the lower_thumb_line, or below the upper_thumb_
	 * line if hardware can't verify it's a finger, starts as JAILED.
	 */
	if (t->state == TOUCH_BEGIN && tp_thumb_needs_jail(tp, t)) {
		tp_thumb_set_state(tp, t, THUMB_STATE_JAILED);
		return;
	}

	/* If a touch breaks the speed threshold, or leaves the thumb area
	 * (upper or lower, depending on HW detection), it "escapes" jail.
	 */
	if (tp->thumb.state == THUMB_STATE_JAILED &&
	    !(tp_thumb_needs_jail(tp, t)))
		tp_thumb_set_state(tp, t, THUMB_STATE_FINGER);
	if (tp->thumb.state == THUMB_STATE_REVIVED_JAILED &&
	    !(tp_thumb_needs_jail(tp, t)))
		tp_thumb_set_state(tp, t, THUMB_STATE_REVIVED);
}

void
tp_thumb_update_multifinger(struct tp_dispatch *tp)
{
	struct tp_touch *t;
	struct tp_touch *first = NULL,
			*second = NULL,
			*newest = NULL,
			*oldest = NULL;
	struct device_coords distance;
	struct phys_coords mm;

	unsigned int speed_exceeded_count = 0;

	/* Get the first and second bottom-most touches, the max speed exceeded
	 * count overall, and the newest and oldest touches.
	 */
	tp_for_each_touch(tp, t) {
		if (t->state == TOUCH_NONE ||
		    t->state == TOUCH_HOVERING)
			continue;

		if (t->state == TOUCH_BEGIN)
			newest = t;

		speed_exceeded_count = max(speed_exceeded_count,
		                           t->speed.exceeded_count);

		if (!oldest || t->initial_time < oldest->initial_time) {
			oldest = t;
		}

		if (!first) {
			first = t;
			continue;
		}

		if (t->point.y > first->point.y) {
			second = first;
			first = t;
			continue;
		}

		if (!second || t->point.y > second->point.y ) {
			second = t;
		}
	}

	if (!first || !second)
		return;

	distance.x = abs(first->point.x - second->point.x);
	distance.y = abs(first->point.y - second->point.y);
	mm = evdev_device_unit_delta_to_mm(tp->device, &distance);

	/* Speed-based thumb detection: if an existing finger is moving, and
	 * a new touch arrives, mark it as a thumb if it doesn't qualify as a
	 * 2-finger scroll. Also account for a thumb dropping onto the touchpad
	 * while scrolling or swiping.
	 */
	if (newest &&
	    tp->thumb.state == THUMB_STATE_FINGER &&
	    tp->nfingers_down >= 2 &&
	    speed_exceeded_count > 5 &&
	    (tp->scroll.method != LIBINPUT_CONFIG_SCROLL_2FG ||
	     (mm.x > SCROLL_MM_X || mm.y > SCROLL_MM_Y))) {
		evdev_log_debug(tp->device,
				"touch %d is speed-based thumb\n",
				newest->index);
		tp_thumb_suppress(tp, newest);
		return;
	}

	/* Contextual thumb detection: When a new touch arrives, check the
	 * timing and position of the two lowest touches.
	 *
	 * If both touches are very close, regardless of timing, and no matter
	 * their absolute position on the touchpad, count them both as live
	 * to support responsive two-finger scrolling.
	 */

	if (mm.x < SCROLL_MM_X && mm.y < SCROLL_MM_Y) {
		tp_thumb_lift(tp);
		return;
	}

	/* If all the touches arrived within a very short time, and all of them
	 * are above the lower_thumb_line, assume the touches are all live to
	 * enable double, triple, and quadruple taps, clicks, and gestures. (If
	 * there is an actual resting thumb, it will be detected later based on
	 * the behavior of the other touches.)
	 */

	if (newest &&
	    (newest->initial_time - oldest->initial_time) < THUMB_TIMEOUT &&
	    first->point.y < tp->thumb.lower_thumb_line) {
		tp_thumb_lift(tp);
		return;
	}

	/* If we're past the THUMB_TIMEOUT, and the touches are relatively far
	 * apart, then the new touch is unlikely to be a tap or clickfinger.
	 * Proceed with pre-1.14.901 thumb detection.
	*/

	if (mm.y > SCROLL_MM_Y) {
		if (tp->thumb.pinch_eligible)
			tp_thumb_pinch(tp, first);
		else
			tp_thumb_suppress(tp, first);
	} else {
		tp_thumb_lift(tp);
	}
}

void
tp_init_thumb(struct tp_dispatch *tp)
{
	struct evdev_device *device = tp->device;
	double w = 0.0, h = 0.0;
	struct device_coords edges;
	struct phys_coords mm = { 0.0, 0.0 };
	uint32_t threshold;
	struct quirks_context *quirks;
	struct quirks *q;

	tp->thumb.detect_thumbs = false;

	if (!tp->buttons.is_clickpad)
		return;

	/* if the touchpad is less than 50mm high, skip thumb detection.
	 * it's too small to meaningfully interact with a thumb on the
	 * touchpad */
	evdev_device_get_size(device, &w, &h);
	if (h < 50)
		return;

	tp->thumb.detect_thumbs = true;
	tp->thumb.use_pressure = false;
	tp->thumb.pressure_threshold = INT_MAX;

	/* detect thumbs by pressure in the bottom 15mm, detect thumbs by
	 * lingering in the bottom 8mm */
	mm.y = h * 0.85;
	edges = evdev_device_mm_to_units(device, &mm);
	tp->thumb.upper_thumb_line = edges.y;

	mm.y = h * 0.92;
	edges = evdev_device_mm_to_units(device, &mm);
	tp->thumb.lower_thumb_line = edges.y;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);

	if (libevdev_has_event_code(device->evdev, EV_ABS, ABS_MT_PRESSURE)) {
		if (quirks_get_uint32(q,
				      QUIRK_ATTR_THUMB_PRESSURE_THRESHOLD,
				      &threshold)) {
			tp->thumb.use_pressure = true;
			tp->thumb.pressure_threshold = threshold;
		}
	}

	if (libevdev_has_event_code(device->evdev, EV_ABS, ABS_MT_TOUCH_MAJOR)) {
		if (quirks_get_uint32(q,
				      QUIRK_ATTR_THUMB_SIZE_THRESHOLD,
				      &threshold)) {
			tp->thumb.use_size = true;
			tp->thumb.size_threshold = threshold;
		}
	}

	tp_thumb_reset(tp);

	quirks_unref(q);

	evdev_log_debug(device,
			"thumb: enabled thumb detection (area%s%s)\n",
			tp->thumb.use_pressure ? ", pressure" : "",
			tp->thumb.use_size ? ", size" : "");
}

struct tp_touch*
tp_thumb_get_touch(struct tp_dispatch *tp)
{
	struct tp_touch *thumb;

	if (tp->thumb.index == UINT_MAX)
		return NULL;

	tp_for_each_touch(tp, thumb) {
		if (thumb->index == tp->thumb.index)
			return thumb;
	}

	return NULL;
}
