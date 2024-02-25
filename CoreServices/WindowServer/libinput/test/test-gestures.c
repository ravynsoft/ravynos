/*
 * Copyright © 2015 Red Hat, Inc.
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

#include <config.h>

#include <check.h>
#include <libinput.h>
#include <valgrind/valgrind.h>

#include "libinput-util.h"
#include "litest.h"

enum cardinal {
	N, NE, E, SE, S, SW, W, NW, NCARDINALS
};

enum hold_gesture_behaviour {
   HOLD_GESTURE_IGNORE,
   HOLD_GESTURE_REQUIRE,
};

static void
test_gesture_swipe_3fg(int cardinal, enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 3)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_touch_down(dev, 2, 60, 40);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	litest_touch_move_three_touches(dev, 40, 40, 50, 40, 60, 40, dir_x,
					dir_y, 10);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					    3);
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_END,
					    3);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 3);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 3);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}

static void
test_gesture_swipe_4fg(int cardinal, enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 3 },
		{ 3, 3 },
		{ 3, 0 },
		{ 3, -3 },
		{ 0, -3 },
		{ -3, -3 },
		{ -3, 0 },
		{ -3, 3 },
	};
	int i;

	if (litest_slot_count(dev) < 4)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_touch_down(dev, 2, 60, 40);
	litest_touch_down(dev, 3, 70, 40);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);

		dir_x += cardinals[cardinal][0];
		dir_y += cardinals[cardinal][1];

		litest_touch_move(dev,
				  0,
				  40 + dir_x,
				  40 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 + dir_x,
				  40 + dir_y);
		litest_touch_move(dev,
				  2,
				  60 + dir_x,
				  40 + dir_y);
		litest_touch_move(dev,
				  3,
				  70 + dir_x,
				  40 + dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					    4);
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_END,
					    4);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 4);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 4);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	litest_touch_up(dev, 3);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 4);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}

static void
test_gesture_pinch_2fg(int cardinal, enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 2 ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	/* If the device is too small to provide a finger spread wide enough
	 * to avoid the scroll bias, skip the test */
	if (cardinal == E || cardinal == W) {
		double w = 0, h = 0;
		libinput_device_get_size(dev->libinput_device, &w, &h);
		/* 0.6 because the code below gives us points like 20/y and
		 * 80/y. 45 because the threshold in the code is 40mm */
		if (w * 0.6 < 45)
			return;
	}

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x -= 2;
		else if (dir_x < 0.0)
			dir_x += 2;
		if (dir_y > 0.0)
			dir_y -= 2;
		else if (dir_y < 0.0)
			dir_y += 2;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					2);
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_END,
					2);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 2);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 2);

		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);

		ck_assert(scale < oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 2);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}

static void
test_gesture_pinch_3fg(int cardinal, enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 3)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	litest_touch_down(dev, 2, 51 - dir_x, 51 - dir_y);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x -= 2;
		else if (dir_x < 0.0)
			dir_x += 2;
		if (dir_y > 0.0)
			dir_y -= 2;
		else if (dir_y < 0.0)
			dir_y += 2;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_touch_move(dev,
				  2,
				  51 - dir_x,
				  51 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					3);
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_END,
					3);
	}
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 3);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 3);

		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);

		ck_assert(scale < oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}

static void
test_gesture_pinch_4fg(int cardinal, enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 4)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	litest_touch_down(dev, 2, 51 - dir_x, 51 - dir_y);
	litest_touch_down(dev, 3, 52 - dir_x, 52 - dir_y);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	for (i = 0; i < 7; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x -= 2;
		else if (dir_x < 0.0)
			dir_x += 2;
		if (dir_y > 0.0)
			dir_y -= 2;
		else if (dir_y < 0.0)
			dir_y += 2;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_touch_move(dev,
				  2,
				  51 - dir_x,
				  51 - dir_y);
		litest_touch_move(dev,
				  3,
				  52 - dir_x,
				  52 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					4);
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_END,
					4);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 4);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 4);

		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);

		ck_assert(scale < oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	litest_touch_up(dev, 3);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 4);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}

static void
test_gesture_spread(int cardinal, enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 2 ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	/* If the device is too small to provide a finger spread wide enough
	 * to avoid the scroll bias, skip the test */
	if (cardinal == E || cardinal == W) {
		double w = 0, h = 0;
		libinput_device_get_size(dev->libinput_device, &w, &h);
		/* 0.6 because the code below gives us points like 20/y and
		 * 80/y. 45 because the threshold in the code is 40mm */
		if (w * 0.6 < 45)
			return;
	}

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	for (i = 0; i < 15; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x += 1;
		else if (dir_x < 0.0)
			dir_x -= 1;
		if (dir_y > 0.0)
			dir_y += 1;
		else if (dir_y < 0.0)
			dir_y -= 1;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					2);
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_END,
					2);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 2);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 2);
		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);
		ck_assert(scale > oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 2);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}

static void
test_gesture_3fg_buttonarea_scroll(enum hold_gesture_behaviour hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_buttonareas(dev);
	litest_enable_2fg_scroll(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 20);
	litest_touch_down(dev, 1, 30, 20);
	/* third finger in btnarea */
	litest_touch_down(dev, 2, 50, 99);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE)
		litest_timeout_gesture_hold();

	litest_touch_move_two_touches(dev, 40, 20, 30, 20, 0, 40, 10);

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);

	if (hold == HOLD_GESTURE_REQUIRE) {
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					2);
		litest_assert_gesture_event(li,
					LIBINPUT_EVENT_GESTURE_HOLD_END,
					2);
	}

	litest_assert_scroll(li,
			     LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			     LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
			     4);
}

static void
test_gesture_hold(int nfingers)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < nfingers)
		return;

	litest_drain_events(li);

	switch (nfingers) {
	case 4:
		litest_touch_down(dev, 3, 70, 30);
		_fallthrough_;
	case 3:
		litest_touch_down(dev, 2, 60, 30);
		_fallthrough_;
	case 2:
		litest_touch_down(dev, 1, 50, 30);
		_fallthrough_;
	case 1:
		litest_touch_down(dev, 0, 40, 30);
		break;
	}

	libinput_dispatch(li);
	litest_timeout_gesture_hold();

	if (libinput_device_has_capability(dev->libinput_device,
					   LIBINPUT_DEVICE_CAP_GESTURE)) {
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					    nfingers);
	} else {
		litest_assert_empty_queue(li);
	}

	switch (nfingers) {
	case 4:
		litest_touch_up(dev, 3);
		_fallthrough_;
	case 3:
		litest_touch_up(dev, 2);
		_fallthrough_;
	case 2:
		litest_touch_up(dev, 1);
		_fallthrough_;
	case 1:
		litest_touch_up(dev, 0);
		break;
	}

	libinput_dispatch(li);
	if (libinput_device_has_capability(dev->libinput_device,
					   LIBINPUT_DEVICE_CAP_GESTURE)) {
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_END,
					    nfingers);
	}

	litest_assert_empty_queue(li);
}

static void
test_gesture_hold_cancel(int nfingers)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int last_finger = (nfingers - 1);

	if (litest_slot_count(dev) < nfingers)
		return;

	litest_drain_events(li);

	switch (nfingers) {
	case 4:
		litest_touch_down(dev, 3, 70, 30);
		_fallthrough_;
	case 3:
		litest_touch_down(dev, 2, 60, 30);
		_fallthrough_;
	case 2:
		litest_touch_down(dev, 1, 50, 30);
		_fallthrough_;
	case 1:
		litest_touch_down(dev, 0, 40, 30);
		break;
	}

	libinput_dispatch(li);
	litest_timeout_gesture_hold();

	litest_touch_up(dev, last_finger);

	if (libinput_device_has_capability(dev->libinput_device,
					   LIBINPUT_DEVICE_CAP_GESTURE)) {
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					    nfingers);
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_END,
					    nfingers);
	}

	litest_assert_empty_queue(li);
}

START_TEST(gestures_cap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	if (libevdev_has_property(dev->evdev, INPUT_PROP_SEMI_MT))
		ck_assert(!libinput_device_has_capability(device,
					  LIBINPUT_DEVICE_CAP_GESTURE));
	else
		ck_assert(libinput_device_has_capability(device,
					 LIBINPUT_DEVICE_CAP_GESTURE));
}
END_TEST

START_TEST(gestures_nocap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert(!libinput_device_has_capability(device,
						  LIBINPUT_DEVICE_CAP_GESTURE));
}
END_TEST

START_TEST(gestures_swipe_3fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_swipe_3fg(cardinal, HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_swipe_3fg_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP) ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_touch_move_two_touches(dev, 40, 40, 50, 40, dir_x, dir_y, 10);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 3);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 3);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_swipe_3fg_btntool_pinch_like)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP) ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	litest_drain_events(li);

	/* Technically a pinch position + pinch movement, but expect swipe
	 * for nfingers > nslots */
	litest_touch_down(dev, 0, 20, 60);
	litest_touch_down(dev, 1, 50, 20);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_touch_move_to(dev, 0, 20, 60, 10, 80, 20);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_gesture_event(event, LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN, 3);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		litest_is_gesture_event(event,
					LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
					3);
		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_swipe_4fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_swipe_4fg(cardinal, HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_swipe_4fg_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_QUADTAP) ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_touch_move_two_touches(dev, 40, 40, 50, 40, dir_x, dir_y, 10);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 4);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 4);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 4);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_pinch)
{
	int cardinal = _i; /* ranged test */
	test_gesture_pinch_2fg(cardinal, HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_pinch_3fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_pinch_3fg(cardinal, HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_pinch_4fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_pinch_4fg(cardinal, HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_spread)
{
	int cardinal = _i; /* ranged test */
	test_gesture_spread(cardinal, HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_time_usec)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	uint64_t time_usec;

	if (litest_slot_count(dev) < 3)
		return;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_touch_down(dev, 2, 60, 40);
	libinput_dispatch(li);
	litest_touch_move_three_touches(dev, 40, 40, 50, 40, 60, 40, 0, 30,
					30);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 3);
	time_usec = libinput_event_gesture_get_time_usec(gevent);
	ck_assert_int_eq(libinput_event_gesture_get_time(gevent),
			 (uint32_t) (time_usec / 1000));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_3fg_buttonarea_scroll)
{
	test_gesture_3fg_buttonarea_scroll(HOLD_GESTURE_IGNORE);
}
END_TEST

START_TEST(gestures_3fg_buttonarea_scroll_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) > 2)
		return;

	litest_enable_buttonareas(dev);
	litest_enable_2fg_scroll(dev);
	litest_drain_events(li);

	/* first finger in btnarea */
	litest_touch_down(dev, 0, 20, 99);
	litest_touch_down(dev, 1, 30, 20);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_touch_move_to(dev, 1, 30, 20, 30, 70, 10);

	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	litest_assert_scroll(li,
			     LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
			     LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
			     4);
}
END_TEST

START_TEST(gestures_swipe_3fg_unaccel)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	double reference_ux = 0, reference_uy = 0;

	/**
	 * This magic number is an artifact of the acceleration code.
	 * The maximum factor in the touchpad accel profile is 4.8 times the
	 * speed setting (1.000875 at default setting 0). The factor
	 * applied to the const acceleration is the 0.9 baseline.
	 * So our two sets of coordinates are:
	 * accel = 4.8 * delta * normalize_magic
	 * unaccel = 0.9 * delta * normalize_magic
	 *
	 * Since delta and the normalization magic are the same for both,
	 * our accelerated deltas can be a maximum of 4.8/0.9 bigger than
	 * the unaccelerated deltas.
	 *
	 * If any of the accel methods numbers change, this will have to
	 * change here too.
	 */
	const double max_factor = 5.34;

	if (litest_slot_count(dev) < 3)
		return;

	litest_drain_events(li);
	litest_touch_down(dev, 0, 40, 20);
	litest_touch_down(dev, 1, 50, 20);
	litest_touch_down(dev, 2, 60, 20);
	libinput_dispatch(li);
	litest_touch_move_three_touches(dev,
					40, 20,
					50, 20,
					60, 20,
					30, 40,
					10);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_gesture_event(event,
				LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
				3);
	libinput_event_destroy(event);
	event = libinput_get_event(li);
	do {
		struct libinput_event_gesture *gevent;
		double dx, dy;
		double ux, uy;

		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 3);
		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		ux = libinput_event_gesture_get_dx_unaccelerated(gevent);
		uy = libinput_event_gesture_get_dy_unaccelerated(gevent);

		ck_assert_double_ne(ux, 0.0);
		ck_assert_double_ne(uy, 0.0);

		if (!reference_ux)
			reference_ux = ux;
		if (!reference_uy)
			reference_uy = uy;

		/* The unaccelerated delta should be the same for every
		 * event, but we have rounding errors since we only control
		 * input data as percentage of the touchpad size.
		 * so we just eyeball it */
		ck_assert_double_gt(ux, reference_ux - 2);
		ck_assert_double_lt(ux, reference_ux + 2);
		ck_assert_double_gt(uy, reference_uy - 2);
		ck_assert_double_lt(uy, reference_uy + 2);

		/* All our touchpads are large enough to make this is a fast
		 * swipe, we don't expect deceleration, unaccel should
		 * always be less than accel delta */
		ck_assert_double_lt(ux, dx);
		ck_assert_double_lt(ux, dx);

		/* Check our accelerated delta is within the expected
		 * maximum. */
		ck_assert_double_lt(dx, ux * max_factor);
		ck_assert_double_lt(dy, uy * max_factor);

		libinput_event_destroy(event);
	} while ((event = libinput_get_event(li)));

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
}
END_TEST

START_TEST(gestures_hold_config_default_disabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert_int_eq(libinput_device_config_gesture_hold_is_available(device),
			 0);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_default_enabled(device),
			 LIBINPUT_CONFIG_HOLD_DISABLED);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_default_enabled(device),
			 LIBINPUT_CONFIG_HOLD_DISABLED);
}
END_TEST

START_TEST(gestures_hold_config_default_enabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert_int_eq(libinput_device_config_gesture_hold_is_available(device),
			 1);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_default_enabled(device),
			 LIBINPUT_CONFIG_HOLD_ENABLED);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_enabled(device),
			 LIBINPUT_CONFIG_HOLD_ENABLED);
}
END_TEST

START_TEST(gestures_hold_config_set_invalid)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert_int_eq(libinput_device_config_gesture_set_hold_enabled(device, -1),
			 LIBINPUT_CONFIG_STATUS_INVALID);
	ck_assert_int_eq(libinput_device_config_gesture_set_hold_enabled(device, 2),
			 LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(gestures_hold_config_is_available)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert_int_eq(libinput_device_config_gesture_hold_is_available(device),
			 1);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_enabled(device),
			 LIBINPUT_CONFIG_HOLD_ENABLED);
	ck_assert_int_eq(libinput_device_config_gesture_set_hold_enabled(device, LIBINPUT_CONFIG_HOLD_DISABLED),
			 LIBINPUT_CONFIG_STATUS_SUCCESS);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_enabled(device),
			 LIBINPUT_CONFIG_HOLD_DISABLED);
}
END_TEST

START_TEST(gestures_hold_config_is_not_available)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert_int_eq(libinput_device_config_gesture_hold_is_available(device),
			 0);
	ck_assert_int_eq(libinput_device_config_gesture_get_hold_enabled(device),
			 LIBINPUT_CONFIG_HOLD_DISABLED);
	ck_assert_int_eq(libinput_device_config_gesture_set_hold_enabled(device, LIBINPUT_CONFIG_HOLD_ENABLED),
			 LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	ck_assert_int_eq(libinput_device_config_gesture_set_hold_enabled(device, LIBINPUT_CONFIG_HOLD_DISABLED),
			 LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

START_TEST(gestures_hold)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */

	litest_disable_tap(dev->libinput_device);
	litest_drain_events(li);

	test_gesture_hold(nfingers);
}
END_TEST

START_TEST(gestures_hold_tap_enabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	test_gesture_hold(nfingers);
}
END_TEST

START_TEST(gestures_hold_cancel)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */

	litest_disable_tap(dev->libinput_device);
	litest_drain_events(li);

	test_gesture_hold_cancel(nfingers);
}
END_TEST

START_TEST(gestures_hold_cancel_tap_enabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	test_gesture_hold_cancel(nfingers);
}
END_TEST

START_TEST(gestures_hold_then_swipe_3fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_swipe_3fg(cardinal, HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_then_swipe_4fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_swipe_4fg(cardinal, HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_then_pinch_2fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_pinch_2fg(cardinal, HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_then_pinch_3fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_pinch_3fg(cardinal, HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_then_pinch_4fg)
{
	int cardinal = _i; /* ranged test */
	test_gesture_pinch_4fg(cardinal, HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_then_spread)
{
	int cardinal = _i; /* ranged test */
	test_gesture_spread(cardinal, HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_then_3fg_buttonarea_scroll)
{
	test_gesture_3fg_buttonarea_scroll(HOLD_GESTURE_REQUIRE);
}
END_TEST

START_TEST(gestures_hold_once_on_double_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (!libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* First tap, a hold gesture must be generated */
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_timeout_gesture_quick_hold();
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_gesture_event(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
				    1);
	litest_assert_gesture_event(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_END,
				    1);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_empty_queue(li);

	/* Double tap, don't generate an extra hold gesture */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);
	litest_timeout_gesture_quick_hold();

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(gestures_hold_once_tap_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */
	unsigned int button = 0;

	if (nfingers > litest_slot_count(dev))
		return;

	if (!libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_disable_drag_lock(dev->libinput_device);
	litest_drain_events(li);

	switch (nfingers) {
	case 1:
		button = BTN_LEFT;
		break;
	case 2:
		button = BTN_RIGHT;
		break;
	case 3:
		button = BTN_MIDDLE;
		break;
	default:
		abort();
	}

	switch (nfingers) {
	case 3:
		litest_touch_down(dev, 2, 60, 30);
		_fallthrough_;
	case 2:
		litest_touch_down(dev, 1, 50, 30);
		_fallthrough_;
	case 1:
		litest_touch_down(dev, 0, 40, 30);
		break;
	}
	libinput_dispatch(li);
	litest_timeout_gesture_quick_hold();

	switch (nfingers) {
	case 3:
		litest_touch_up(dev, 2);
		_fallthrough_;
	case 2:
		litest_touch_up(dev, 1);
		_fallthrough_;
	case 1:
		litest_touch_up(dev, 0);
		break;
	}
	libinput_dispatch(li);

	/* "Quick" hold gestures are only generated when using 1 or 2 fingers */
	if (nfingers == 1 || nfingers == 2) {
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
					    nfingers);
		litest_assert_gesture_event(li,
					    LIBINPUT_EVENT_GESTURE_HOLD_END,
					    nfingers);
	}

	/* Tap and drag, don't generate an extra hold gesture */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);
	libinput_dispatch(li);

	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(gestures_hold_and_motion_before_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (!libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);

	litest_touch_move_to(dev, 0, 50, 50, 51, 51, 1);
	litest_touch_move_to(dev, 0, 51, 51, 50, 50, 1);
	libinput_dispatch(li);

	litest_timeout_gesture_quick_hold();

	litest_drain_events_of_type(li, LIBINPUT_EVENT_POINTER_MOTION, -1);

	litest_assert_gesture_event(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
				    1);

	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_gesture_event(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_END,
				    1);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(gestures_hold_and_motion_after_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (!libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_timeout_gesture_quick_hold();

	litest_assert_gesture_event(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
				    1);

	litest_touch_move_to(dev, 0, 50, 50, 51, 51, 1);
	litest_touch_move_to(dev, 0, 51, 51, 50, 50, 1);
	libinput_dispatch(li);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_gesture_event(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_END,
				    1);
	litest_assert_empty_queue(li);
}
END_TEST

TEST_COLLECTION(gestures)
{
	struct range cardinals = { N, N + NCARDINALS };
	struct range range_hold = { 1, 5 };
	struct range range_multifinger_tap = {1, 4};

	litest_add(gestures_cap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add(gestures_nocap, LITEST_ANY, LITEST_TOUCHPAD);

	litest_add_ranged(gestures_swipe_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_swipe_3fg_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add(gestures_swipe_3fg_btntool_pinch_like, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add_ranged(gestures_swipe_4fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_swipe_4fg_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_pinch, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_pinch_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_pinch_4fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_spread, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);

	litest_add(gestures_3fg_buttonarea_scroll, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH);
	litest_add(gestures_3fg_buttonarea_scroll_btntool, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH);

	litest_add(gestures_time_usec, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);

	litest_add(gestures_hold_config_default_disabled, LITEST_TOUCHPAD|LITEST_SEMI_MT, LITEST_ANY);
	litest_add(gestures_hold_config_default_enabled, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add(gestures_hold_config_set_invalid, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add(gestures_hold_config_is_available, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add(gestures_hold_config_is_not_available, LITEST_TOUCHPAD|LITEST_SEMI_MT, LITEST_ANY);

	litest_add_ranged(gestures_hold, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_hold);
	litest_add_ranged(gestures_hold_tap_enabled, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_hold);
	litest_add_ranged(gestures_hold_cancel, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_hold);
	litest_add_ranged(gestures_hold_cancel_tap_enabled, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_hold);
	litest_add_ranged(gestures_hold_then_swipe_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_hold_then_swipe_4fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_hold_then_pinch_2fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_hold_then_pinch_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_hold_then_pinch_4fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged(gestures_hold_then_spread, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add(gestures_hold_then_3fg_buttonarea_scroll, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH);

	litest_add(gestures_hold_once_on_double_tap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add_ranged(gestures_hold_once_tap_n_drag, LITEST_TOUCHPAD, LITEST_ANY, &range_multifinger_tap);

	litest_add(gestures_hold_and_motion_before_timeout, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add(gestures_hold_and_motion_after_timeout, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);

	/* Timing-sensitive test, valgrind is too slow */
	if (!RUNNING_ON_VALGRIND)
		litest_add(gestures_swipe_3fg_unaccel, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
}
