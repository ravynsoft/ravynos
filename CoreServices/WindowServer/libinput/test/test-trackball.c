/*
 * Copyright © 2016 Red Hat, Inc.
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
#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <unistd.h>

#include "libinput-util.h"
#include "litest.h"

START_TEST(trackball_rotation_config_defaults)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	int angle;

	ck_assert(libinput_device_config_rotation_is_available(device));

	angle = libinput_device_config_rotation_get_angle(device);
	ck_assert_int_eq(angle, 0);
	angle = libinput_device_config_rotation_get_default_angle(device);
	ck_assert_int_eq(angle, 0);
}
END_TEST

START_TEST(trackball_rotation_config_invalid_range)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;

	status = libinput_device_config_rotation_set_angle(device, 360);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
	status = libinput_device_config_rotation_set_angle(device, 361);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
	status = libinput_device_config_rotation_set_angle(device, -1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(trackball_rotation_config_no_rotation)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;
	int angle;

	ck_assert(!libinput_device_config_rotation_is_available(device));

	angle = libinput_device_config_rotation_get_angle(device);
	ck_assert_int_eq(angle, 0);
	angle = libinput_device_config_rotation_get_default_angle(device);
	ck_assert_int_eq(angle, 0);

	/* 0 always succeeds */
	status = libinput_device_config_rotation_set_angle(device, 0);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	for (angle = 1; angle < 360; angle++) {
		status = libinput_device_config_rotation_set_angle(device,
								   angle);
		ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	}
}
END_TEST

START_TEST(trackball_rotation_config_right_angle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;
	int angle;

	ck_assert(libinput_device_config_rotation_is_available(device));

	for (angle = 0; angle < 360; angle += 90) {
		status = libinput_device_config_rotation_set_angle(device,
								   angle);
		ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	}
}
END_TEST

START_TEST(trackball_rotation_config_odd_angle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;
	int angle;

	ck_assert(libinput_device_config_rotation_is_available(device));

	for (angle = 0; angle < 360; angle++) {
		status = libinput_device_config_rotation_set_angle(device,
								   angle);
		ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	}
}
END_TEST

START_TEST(trackball_rotation_x)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device = dev->libinput_device;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	int angle;
	double dx, dy;

	litest_drain_events(li);

	for (angle = 0; angle < 360; angle++) {
		libinput_device_config_rotation_set_angle(device, angle);

		litest_event(dev, EV_REL, REL_X, 1);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		ptrev = litest_is_motion_event(event);

		/* Test unaccelerated because pointer accel may mangle the
		   other coords */
		dx = libinput_event_pointer_get_dx_unaccelerated(ptrev);
		dy = libinput_event_pointer_get_dy_unaccelerated(ptrev);

		switch (angle) {
		case 0:
			ck_assert_double_eq(dx, 1.0);
			ck_assert_double_eq(dy, 0.0);
			break;
		case 90:
			ck_assert_double_eq(dx, 0.0);
			ck_assert_double_eq(dy, 1.0);
			break;
		case 180:
			ck_assert_double_eq(dx, -1.0);
			ck_assert_double_eq(dy, 0.0);
			break;
		case 270:
			ck_assert_double_eq(dx, 0.0);
			ck_assert_double_eq(dy, -1.0);
			break;
		}
		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(trackball_rotation_y)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device = dev->libinput_device;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	int angle;
	double dx, dy;

	litest_drain_events(li);

	for (angle = 0; angle < 360; angle++) {
		libinput_device_config_rotation_set_angle(device, angle);

		litest_event(dev, EV_REL, REL_Y, 1);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		ptrev = litest_is_motion_event(event);

		/* Test unaccelerated because pointer accel may mangle the
		   other coords */
		dx = libinput_event_pointer_get_dx_unaccelerated(ptrev);
		dy = libinput_event_pointer_get_dy_unaccelerated(ptrev);

		switch (angle) {
		case 0:
			ck_assert_double_eq(dx, 0.0);
			ck_assert_double_eq(dy, 1.0);
			break;
		case 90:
			ck_assert_double_eq(dx, -1.0);
			ck_assert_double_eq(dy, 0.0);
			break;
		case 180:
			ck_assert_double_eq(dx, 0.0);
			ck_assert_double_eq(dy, -1.0);
			break;
		case 270:
			ck_assert_double_eq(dx, 1.0);
			ck_assert_double_eq(dy, 0.0);
			break;
		}
		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(trackball_rotation_accel)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device = dev->libinput_device;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	double dx, dy;

	litest_drain_events(li);

	/* Pointer accel mangles the coordinates, so we only test one angle
	 * and rely on the unaccelerated tests above to warn us when
	 * something's off */
	libinput_device_config_rotation_set_angle(device, 90);

	litest_event(dev, EV_REL, REL_Y, 1);
	litest_event(dev, EV_REL, REL_X, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	ptrev = litest_is_motion_event(event);

	dx = libinput_event_pointer_get_dx(ptrev);
	dy = libinput_event_pointer_get_dy(ptrev);

	ck_assert_double_lt(dx, 0.0);
	ck_assert_double_gt(dy, 0.0);
	libinput_event_destroy(event);
}
END_TEST

TEST_COLLECTION(trackball)
{
	litest_add(trackball_rotation_config_defaults, LITEST_TRACKBALL, LITEST_ANY);
	litest_add(trackball_rotation_config_invalid_range, LITEST_TRACKBALL, LITEST_ANY);
	litest_add(trackball_rotation_config_no_rotation, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackball_rotation_config_right_angle, LITEST_TRACKBALL, LITEST_ANY);
	litest_add(trackball_rotation_config_odd_angle, LITEST_TRACKBALL, LITEST_ANY);
	litest_add(trackball_rotation_x, LITEST_TRACKBALL, LITEST_ANY);
	litest_add(trackball_rotation_y, LITEST_TRACKBALL, LITEST_ANY);
	litest_add(trackball_rotation_accel, LITEST_TRACKBALL, LITEST_ANY);
}
