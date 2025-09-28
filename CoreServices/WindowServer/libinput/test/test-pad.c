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
#include <stdbool.h>

#if HAVE_LIBWACOM
#include <libwacom/libwacom.h>
#endif

#include "libinput-util.h"
#include "litest.h"

START_TEST(pad_cap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert(libinput_device_has_capability(device,
						 LIBINPUT_DEVICE_CAP_TABLET_PAD));

}
END_TEST

START_TEST(pad_no_cap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert(!libinput_device_has_capability(device,
						  LIBINPUT_DEVICE_CAP_TABLET_PAD));
}
END_TEST

START_TEST(pad_time)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	unsigned int code;
	uint64_t time, time_usec, oldtime;
	bool has_buttons = false;

	litest_drain_events(li);

	for (code = BTN_0; code < BTN_DIGI; code++) {
		if (!libevdev_has_event_code(dev->evdev, EV_KEY, code))
			continue;

		has_buttons = true;

		litest_button_click(dev, code, 1);
		litest_button_click(dev, code, 0);
		libinput_dispatch(li);

		break;
	}

	if (!has_buttons)
		return;

	ev = libinput_get_event(li);
	ck_assert_notnull(ev);
	ck_assert_int_eq(libinput_event_get_type(ev),
			 LIBINPUT_EVENT_TABLET_PAD_BUTTON);
	pev = libinput_event_get_tablet_pad_event(ev);
	time = libinput_event_tablet_pad_get_time(pev);
	time_usec = libinput_event_tablet_pad_get_time_usec(pev);

	ck_assert(time != 0);
	ck_assert(time == time_usec/1000);

	libinput_event_destroy(ev);

	litest_drain_events(li);
	msleep(10);

	litest_button_click(dev, code, 1);
	litest_button_click(dev, code, 0);
	libinput_dispatch(li);

	ev = libinput_get_event(li);
	ck_assert_int_eq(libinput_event_get_type(ev),
			 LIBINPUT_EVENT_TABLET_PAD_BUTTON);
	pev = libinput_event_get_tablet_pad_event(ev);

	oldtime = time;
	time = libinput_event_tablet_pad_get_time(pev);
	time_usec = libinput_event_tablet_pad_get_time_usec(pev);

	ck_assert(time > oldtime);
	ck_assert(time != 0);
	ck_assert(time == time_usec/1000);

	libinput_event_destroy(ev);
}
END_TEST

START_TEST(pad_num_buttons_libwacom)
{
#if HAVE_LIBWACOM
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	WacomDeviceDatabase *db = NULL;
	WacomDevice *wacom = NULL;
	unsigned int nb_lw, nb;

	db = libwacom_database_new();
	ck_assert_notnull(db);

	wacom = libwacom_new_from_usbid(db,
					libevdev_get_id_vendor(dev->evdev),
					libevdev_get_id_product(dev->evdev),
					NULL);
	ck_assert_notnull(wacom);

	nb_lw = libwacom_get_num_buttons(wacom);
	nb = libinput_device_tablet_pad_get_num_buttons(device);

	ck_assert_int_eq(nb, nb_lw);

	libwacom_destroy(wacom);
	libwacom_database_destroy(db);
#endif
}
END_TEST

START_TEST(pad_num_buttons)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	unsigned int code;
	unsigned int nbuttons = 0;

	for (code = BTN_0; code < KEY_OK; code++) {
		/* BTN_STYLUS is set for compatibility reasons but not
		 * actually hooked up */
		if (code == BTN_STYLUS)
			continue;

		if (libevdev_has_event_code(dev->evdev, EV_KEY, code))
			nbuttons++;
	}

	ck_assert_int_eq(libinput_device_tablet_pad_get_num_buttons(device),
			 nbuttons);
}
END_TEST

START_TEST(pad_button_intuos)
{
#if !HAVE_LIBWACOM
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	unsigned int code;
	unsigned int expected_number = 0;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	unsigned int count = 0;

	/* Intuos button mapping is sequential up from BTN_0 and continues
	 * with BTN_A */
	if (!libevdev_has_event_code(dev->evdev, EV_KEY, BTN_0))
		return;

	litest_drain_events(li);

	for (code = BTN_0; code < BTN_DIGI; code++) {
		/* Skip over the BTN_MOUSE and BTN_JOYSTICK range */
		if ((code >= BTN_MOUSE && code < BTN_JOYSTICK) ||
		    (code >= BTN_DIGI)) {
			ck_assert(!libevdev_has_event_code(dev->evdev,
							   EV_KEY, code));
			continue;
		}

		if (!libevdev_has_event_code(dev->evdev, EV_KEY, code))
			continue;

		litest_button_click(dev, code, 1);
		litest_button_click(dev, code, 0);
		libinput_dispatch(li);

		count++;

		ev = libinput_get_event(li);
		pev = litest_is_pad_button_event(ev,
						 expected_number,
						 LIBINPUT_BUTTON_STATE_PRESSED);
		ev = libinput_event_tablet_pad_get_base_event(pev);
		libinput_event_destroy(ev);

		ev = libinput_get_event(li);
		pev = litest_is_pad_button_event(ev,
						 expected_number,
						 LIBINPUT_BUTTON_STATE_RELEASED);
		ev = libinput_event_tablet_pad_get_base_event(pev);
		libinput_event_destroy(ev);

		expected_number++;
	}

	litest_assert_empty_queue(li);

	ck_assert_int_gt(count, 3);
#endif
}
END_TEST

START_TEST(pad_button_bamboo)
{
#if !HAVE_LIBWACOM
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	unsigned int code;
	unsigned int expected_number = 0;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	unsigned int count = 0;

	if (!libevdev_has_event_code(dev->evdev, EV_KEY, BTN_LEFT))
		return;

	litest_drain_events(li);

	for (code = BTN_LEFT; code < BTN_JOYSTICK; code++) {
		if (!libevdev_has_event_code(dev->evdev, EV_KEY, code))
			continue;

		litest_button_click(dev, code, 1);
		litest_button_click(dev, code, 0);
		libinput_dispatch(li);

		count++;

		ev = libinput_get_event(li);
		pev = litest_is_pad_button_event(ev,
						 expected_number,
						 LIBINPUT_BUTTON_STATE_PRESSED);
		ev = libinput_event_tablet_pad_get_base_event(pev);
		libinput_event_destroy(ev);

		ev = libinput_get_event(li);
		pev = litest_is_pad_button_event(ev,
						 expected_number,
						 LIBINPUT_BUTTON_STATE_RELEASED);
		ev = libinput_event_tablet_pad_get_base_event(pev);
		libinput_event_destroy(ev);

		expected_number++;
	}

	litest_assert_empty_queue(li);

	ck_assert_int_gt(count, 3);
#endif
}
END_TEST

START_TEST(pad_button_libwacom)
{
#if HAVE_LIBWACOM
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	WacomDeviceDatabase *db = NULL;
	WacomDevice *wacom = NULL;

	db = libwacom_database_new();
	assert(db);

	wacom = libwacom_new_from_usbid(db,
					libevdev_get_id_vendor(dev->evdev),
					libevdev_get_id_product(dev->evdev),
					NULL);
	assert(wacom);

	litest_drain_events(li);

	for (int i = 0; i < libwacom_get_num_buttons(wacom); i++) {
		unsigned int code;

		code = libwacom_get_button_evdev_code(wacom, 'A' + i);

		litest_button_click(dev, code, 1);
		litest_button_click(dev, code, 0);
		libinput_dispatch(li);

		litest_assert_pad_button_event(li,
					       i,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		litest_assert_pad_button_event(li,
					       i,
					       LIBINPUT_BUTTON_STATE_RELEASED);
	}

	libwacom_destroy(wacom);
	libwacom_database_destroy(db);
#endif
}
END_TEST

START_TEST(pad_button_mode_groups)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	unsigned int code;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;

	litest_drain_events(li);

	for (code = BTN_0; code < KEY_OK; code++) {
		unsigned int mode, index;
		struct libinput_tablet_pad_mode_group *group;

		if (!libevdev_has_event_code(dev->evdev, EV_KEY, code))
			continue;

		litest_button_click(dev, code, 1);
		litest_button_click(dev, code, 0);
		libinput_dispatch(li);

		switch (code) {
		case BTN_STYLUS:
			litest_assert_empty_queue(li);
			continue;
		default:
			break;
		}

		ev = libinput_get_event(li);
		ck_assert_int_eq(libinput_event_get_type(ev),
				 LIBINPUT_EVENT_TABLET_PAD_BUTTON);
		pev = libinput_event_get_tablet_pad_event(ev);

		/* litest virtual devices don't have modes */
		mode = libinput_event_tablet_pad_get_mode(pev);
		ck_assert_int_eq(mode, 0);
		group = libinput_event_tablet_pad_get_mode_group(pev);
		index = libinput_tablet_pad_mode_group_get_index(group);
		ck_assert_int_eq(index, 0);

		libinput_event_destroy(ev);

		ev = libinput_get_event(li);
		ck_assert_int_eq(libinput_event_get_type(ev),
				 LIBINPUT_EVENT_TABLET_PAD_BUTTON);
		pev = libinput_event_get_tablet_pad_event(ev);

		mode = libinput_event_tablet_pad_get_mode(pev);
		ck_assert_int_eq(mode, 0);
		group = libinput_event_tablet_pad_get_mode_group(pev);
		index = libinput_tablet_pad_mode_group_get_index(group);
		ck_assert_int_eq(index, 0);
		libinput_event_destroy(ev);
	}

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(pad_has_ring)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	int nrings;

	nrings = libinput_device_tablet_pad_get_num_rings(device);
	ck_assert_int_ge(nrings, 1);
}
END_TEST

START_TEST(pad_ring)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	int val;
	double degrees, expected;
	int min, max;
	int step_size;
	int nevents = 0;

	litest_pad_ring_start(dev, 10);

	litest_drain_events(li);

	/* Wacom's 0 value is at 275 degrees */
	expected = 270;

	min = libevdev_get_abs_minimum(dev->evdev, ABS_WHEEL);
	max = libevdev_get_abs_maximum(dev->evdev, ABS_WHEEL);
	step_size = 360/(max - min + 1);

	/* This is a bit strange because we rely on kernel filtering here.
	   The litest_*() functions take a percentage, but mapping this to
	   the pads 72 or 36 range pad ranges is lossy and a bit
	   unpredictable. So instead we increase by a small percentage,
	   expecting *most* events to be filtered by the kernel because they
	   resolve to the same integer value as the previous event. Whenever
	   an event gets through, we expect that to be the next integer
	   value in the range and thus the next step on the circle.
	 */
	for (val = 0; val < 100.0; val += 1) {
		litest_pad_ring_change(dev, val);
		libinput_dispatch(li);

		ev = libinput_get_event(li);
		if (!ev)
			continue;

		nevents++;
		pev = litest_is_pad_ring_event(ev,
					       0,
					       LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER);

		degrees = libinput_event_tablet_pad_get_ring_position(pev);
		ck_assert_double_ge(degrees, 0.0);
		ck_assert_double_lt(degrees, 360.0);

		ck_assert_double_eq(degrees, expected);

		libinput_event_destroy(ev);
		expected = fmod(degrees + step_size, 360);
	}

	ck_assert_int_eq(nevents, 360/step_size - 1);

	litest_pad_ring_end(dev);
}
END_TEST

START_TEST(pad_ring_finger_up)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	double degrees;

	litest_pad_ring_start(dev, 10);

	litest_drain_events(li);

	litest_pad_ring_end(dev);
	libinput_dispatch(li);

	ev = libinput_get_event(li);
	pev = litest_is_pad_ring_event(ev,
				       0,
				       LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER);

	degrees = libinput_event_tablet_pad_get_ring_position(pev);
	ck_assert_double_eq(degrees, -1.0);
	libinput_event_destroy(ev);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(pad_has_strip)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	int nstrips;

	nstrips = libinput_device_tablet_pad_get_num_strips(device);
	ck_assert_int_ge(nstrips, 1);
}
END_TEST

START_TEST(pad_strip)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	int val;
	double pos, expected;

	litest_pad_strip_start(dev, 10);

	litest_drain_events(li);

	expected = 0;

	/* 9.5 works with the generic axis scaling without jumping over a
	 * value. */
	for (val = 0; val < 100; val += 9.5) {
		litest_pad_strip_change(dev, val);
		libinput_dispatch(li);

		ev = libinput_get_event(li);
		pev = litest_is_pad_strip_event(ev,
						0,
						LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER);

		pos = libinput_event_tablet_pad_get_strip_position(pev);
		ck_assert_double_ge(pos, 0.0);
		ck_assert_double_lt(pos, 1.0);

		/* rounding errors, mostly caused by small physical range */
		ck_assert_double_ge(pos, expected - 0.02);
		ck_assert_double_le(pos, expected + 0.02);

		libinput_event_destroy(ev);

		expected = pos + 0.08;
	}

	litest_pad_strip_change(dev, 100);
	libinput_dispatch(li);

	ev = libinput_get_event(li);
	pev = litest_is_pad_strip_event(ev,
					   0,
					   LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER);
	pos = libinput_event_tablet_pad_get_strip_position(pev);
	ck_assert_double_eq(pos, 1.0);
	libinput_event_destroy(ev);

	litest_pad_strip_end(dev);
}
END_TEST

START_TEST(pad_strip_finger_up)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	double pos;

	litest_pad_strip_start(dev, 10);
	litest_drain_events(li);

	litest_pad_strip_end(dev);
	libinput_dispatch(li);

	ev = libinput_get_event(li);
	pev = litest_is_pad_strip_event(ev,
					0,
					LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER);

	pos = libinput_event_tablet_pad_get_strip_position(pev);
	ck_assert_double_eq(pos, -1.0);
	libinput_event_destroy(ev);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(pad_left_handed_default)
{
#if HAVE_LIBWACOM
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;

	ck_assert(libinput_device_config_left_handed_is_available(device));

	ck_assert_int_eq(libinput_device_config_left_handed_get_default(device),
			 0);
	ck_assert_int_eq(libinput_device_config_left_handed_get(device),
			 0);

	status = libinput_device_config_left_handed_set(dev->libinput_device, 1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	ck_assert_int_eq(libinput_device_config_left_handed_get(device),
			 1);
	ck_assert_int_eq(libinput_device_config_left_handed_get_default(device),
			 0);

	status = libinput_device_config_left_handed_set(dev->libinput_device, 0);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	ck_assert_int_eq(libinput_device_config_left_handed_get(device),
			 0);
	ck_assert_int_eq(libinput_device_config_left_handed_get_default(device),
			 0);

#endif
}
END_TEST

START_TEST(pad_no_left_handed)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;

	/* Without libwacom we default to left-handed being available */
#if HAVE_LIBWACOM
	ck_assert(!libinput_device_config_left_handed_is_available(device));
#else
	ck_assert(libinput_device_config_left_handed_is_available(device));
#endif

	ck_assert_int_eq(libinput_device_config_left_handed_get_default(device),
			 0);
	ck_assert_int_eq(libinput_device_config_left_handed_get(device),
			 0);

#if HAVE_LIBWACOM
	status = libinput_device_config_left_handed_set(dev->libinput_device, 1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);

	ck_assert_int_eq(libinput_device_config_left_handed_get(device),
			 0);
	ck_assert_int_eq(libinput_device_config_left_handed_get_default(device),
			 0);

	status = libinput_device_config_left_handed_set(dev->libinput_device, 0);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);

	ck_assert_int_eq(libinput_device_config_left_handed_get(device),
			 0);
	ck_assert_int_eq(libinput_device_config_left_handed_get_default(device),
			 0);
#endif
}
END_TEST

START_TEST(pad_left_handed_ring)
{
#if HAVE_LIBWACOM
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_tablet_pad *pev;
	int val;
	double degrees, expected;

	libinput_device_config_left_handed_set(dev->libinput_device, 1);

	litest_pad_ring_start(dev, 10);

	litest_drain_events(li);

	/* Wacom's 0 value is at 275 degrees -> 90 in left-handed mode*/
	expected = 90;

	for (val = 0; val < 100; val += 10) {
		litest_pad_ring_change(dev, val);
		libinput_dispatch(li);

		ev = libinput_get_event(li);
		pev = litest_is_pad_ring_event(ev,
					       0,
					       LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER);

		degrees = libinput_event_tablet_pad_get_ring_position(pev);
		ck_assert_double_ge(degrees, 0.0);
		ck_assert_double_lt(degrees, 360.0);

		/* rounding errors, mostly caused by small physical range */
		ck_assert_double_ge(degrees, expected - 2);
		ck_assert_double_le(degrees, expected + 2);

		libinput_event_destroy(ev);

		expected = fmod(degrees + 36, 360);
	}

	litest_pad_ring_end(dev);
#endif
}
END_TEST

START_TEST(pad_mode_groups)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group *group;
	int ngroups;
	int i;

	ngroups = libinput_device_tablet_pad_get_num_mode_groups(device);
	ck_assert_int_eq(ngroups, 1);

	for (i = 0; i < ngroups; i++) {
		group = libinput_device_tablet_pad_get_mode_group(device, i);
		ck_assert_notnull(group);
		ck_assert_int_eq(libinput_tablet_pad_mode_group_get_index(group),
				 i);
	}

	group = libinput_device_tablet_pad_get_mode_group(device, ngroups);
	ck_assert(group == NULL);
	group = libinput_device_tablet_pad_get_mode_group(device, ngroups + 1);
	ck_assert(group == NULL);
}
END_TEST

START_TEST(pad_mode_groups_userdata)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group *group;
	int rc;
	void *userdata = &rc;

	group = libinput_device_tablet_pad_get_mode_group(device, 0);
	ck_assert(libinput_tablet_pad_mode_group_get_user_data(group) ==
		  NULL);
	libinput_tablet_pad_mode_group_set_user_data(group, userdata);
	ck_assert(libinput_tablet_pad_mode_group_get_user_data(group) ==
		  &rc);

	libinput_tablet_pad_mode_group_set_user_data(group, NULL);
	ck_assert(libinput_tablet_pad_mode_group_get_user_data(group) ==
		  NULL);
}
END_TEST

START_TEST(pad_mode_groups_ref)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group *group, *g;

	group = libinput_device_tablet_pad_get_mode_group(device, 0);
	g = libinput_tablet_pad_mode_group_ref(group);
	ck_assert_ptr_eq(g, group);

	/* We don't expect this to be freed. Any leaks should be caught by
	 * valgrind. */
	g = libinput_tablet_pad_mode_group_unref(group);
	ck_assert_ptr_eq(g, group);
}
END_TEST

START_TEST(pad_mode_group_mode)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group *group;
	int ngroups;
	unsigned int nmodes, mode;

	ngroups = libinput_device_tablet_pad_get_num_mode_groups(device);
	ck_assert_int_ge(ngroups, 1);

	group = libinput_device_tablet_pad_get_mode_group(device, 0);

	nmodes = libinput_tablet_pad_mode_group_get_num_modes(group);
	ck_assert_int_eq(nmodes, 1);

	mode = libinput_tablet_pad_mode_group_get_mode(group);
	ck_assert_int_lt(mode, nmodes);
}
END_TEST

START_TEST(pad_mode_group_has)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group *group;
	int ngroups, nbuttons, nrings, nstrips;
	int i, b, r, s;

	ngroups = libinput_device_tablet_pad_get_num_mode_groups(device);
	ck_assert_int_ge(ngroups, 1);

	nbuttons = libinput_device_tablet_pad_get_num_buttons(device);
	nrings = libinput_device_tablet_pad_get_num_rings(device);
	nstrips = libinput_device_tablet_pad_get_num_strips(device);

	for (b = 0; b < nbuttons; b++) {
		bool found = false;
		for (i = 0; i < ngroups; i++) {
			group = libinput_device_tablet_pad_get_mode_group(device,
									  i);
			if (libinput_tablet_pad_mode_group_has_button(group,
								      b)) {
				ck_assert(!found);
				found = true;
			}
		}
		ck_assert(found);
	}

	for (s = 0; s < nstrips; s++) {
		bool found = false;
		for (i = 0; i < ngroups; i++) {
			group = libinput_device_tablet_pad_get_mode_group(device,
									  i);
			if (libinput_tablet_pad_mode_group_has_strip(group,
								     s)) {
				ck_assert(!found);
				found = true;
			}
		}
		ck_assert(found);
	}

	for (r = 0; r < nrings; r++) {
		bool found = false;
		for (i = 0; i < ngroups; i++) {
			group = libinput_device_tablet_pad_get_mode_group(device,
									  i);
			if (libinput_tablet_pad_mode_group_has_ring(group,
								    r)) {
				ck_assert(!found);
				found = true;
			}
		}
		ck_assert(found);
	}
}
END_TEST

START_TEST(pad_mode_group_has_invalid)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group* group;
	int ngroups, nbuttons, nrings, nstrips;
	int i;
	int rc;

	ngroups = libinput_device_tablet_pad_get_num_mode_groups(device);
	ck_assert_int_ge(ngroups, 1);

	nbuttons = libinput_device_tablet_pad_get_num_buttons(device);
	nrings = libinput_device_tablet_pad_get_num_rings(device);
	nstrips = libinput_device_tablet_pad_get_num_strips(device);

	for (i = 0; i < ngroups; i++) {
		group = libinput_device_tablet_pad_get_mode_group(device, i);
		rc = libinput_tablet_pad_mode_group_has_button(group,
							       nbuttons);
		ck_assert_int_eq(rc, 0);
		rc = libinput_tablet_pad_mode_group_has_button(group,
							       nbuttons + 1);
		ck_assert_int_eq(rc, 0);
		rc = libinput_tablet_pad_mode_group_has_button(group,
							       0x1000000);
		ck_assert_int_eq(rc, 0);
	}

	for (i = 0; i < ngroups; i++) {
		group = libinput_device_tablet_pad_get_mode_group(device, i);
		rc = libinput_tablet_pad_mode_group_has_strip(group,
							      nstrips);
		ck_assert_int_eq(rc, 0);
		rc = libinput_tablet_pad_mode_group_has_strip(group,
							       nstrips + 1);
		ck_assert_int_eq(rc, 0);
		rc = libinput_tablet_pad_mode_group_has_strip(group,
							       0x1000000);
		ck_assert_int_eq(rc, 0);
	}

	for (i = 0; i < ngroups; i++) {
		group = libinput_device_tablet_pad_get_mode_group(device, i);
		rc = libinput_tablet_pad_mode_group_has_ring(group,
							     nrings);
		ck_assert_int_eq(rc, 0);
		rc = libinput_tablet_pad_mode_group_has_ring(group,
							     nrings + 1);
		ck_assert_int_eq(rc, 0);
		rc = libinput_tablet_pad_mode_group_has_ring(group,
							     0x1000000);
		ck_assert_int_eq(rc, 0);
	}
}
END_TEST

START_TEST(pad_mode_group_has_no_toggle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_tablet_pad_mode_group* group;
	int ngroups, nbuttons;
	int i, b;

	ngroups = libinput_device_tablet_pad_get_num_mode_groups(device);
	ck_assert_int_ge(ngroups, 1);

	/* Button must not be toggle buttons */
	nbuttons = libinput_device_tablet_pad_get_num_buttons(device);
	for (i = 0; i < ngroups; i++) {
		group = libinput_device_tablet_pad_get_mode_group(device, i);
		for (b = 0; b < nbuttons; b++) {
			ck_assert(!libinput_tablet_pad_mode_group_button_is_toggle(
								    group,
								    b));
		}
	}
}
END_TEST

static bool
pad_has_keys(struct litest_device *dev)
{
	struct libevdev *evdev = dev->evdev;

	return (libevdev_has_event_code(evdev, EV_KEY, KEY_BUTTONCONFIG) ||
		libevdev_has_event_code(evdev, EV_KEY, KEY_ONSCREEN_KEYBOARD) ||
	        libevdev_has_event_code(evdev, EV_KEY, KEY_CONTROLPANEL));
}

static void
pad_key_down(struct litest_device *dev, unsigned int which)
{
	litest_event(dev, EV_ABS, ABS_MISC, 15);
	litest_event(dev, EV_KEY, which, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
}

static void
pad_key_up(struct litest_device *dev, unsigned int which)
{
	litest_event(dev, EV_ABS, ABS_MISC, 0);
	litest_event(dev, EV_KEY, which, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
}

START_TEST(pad_keys)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	unsigned int key;

	if (!pad_has_keys(dev))
		return;

	litest_drain_events(li);

	key = KEY_BUTTONCONFIG;
	pad_key_down(dev, key);
	libinput_dispatch(li);
	litest_assert_pad_key_event(li, key, LIBINPUT_KEY_STATE_PRESSED);

	pad_key_up(dev, key);
	libinput_dispatch(li);
	litest_assert_pad_key_event(li, key, LIBINPUT_KEY_STATE_RELEASED);

	key = KEY_ONSCREEN_KEYBOARD;
	pad_key_down(dev, key);
	libinput_dispatch(li);
	litest_assert_pad_key_event(li, key, LIBINPUT_KEY_STATE_PRESSED);

	pad_key_up(dev, key);
	libinput_dispatch(li);
	litest_assert_pad_key_event(li, key, LIBINPUT_KEY_STATE_RELEASED);

	key = KEY_CONTROLPANEL;
	pad_key_down(dev, key);
	libinput_dispatch(li);
	litest_assert_pad_key_event(li, key, LIBINPUT_KEY_STATE_PRESSED);

	pad_key_up(dev, key);
	libinput_dispatch(li);
	litest_assert_pad_key_event(li, key, LIBINPUT_KEY_STATE_RELEASED);
}
END_TEST

TEST_COLLECTION(tablet_pad)
{
	litest_add(pad_cap, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_no_cap, LITEST_ANY, LITEST_TABLET_PAD);

	litest_add(pad_time, LITEST_TABLET_PAD, LITEST_ANY);

	litest_add(pad_num_buttons, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_num_buttons_libwacom, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_button_intuos, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_button_bamboo, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_button_libwacom, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_button_mode_groups, LITEST_TABLET_PAD, LITEST_ANY);

	litest_add(pad_has_ring, LITEST_RING, LITEST_ANY);
	litest_add(pad_ring, LITEST_RING, LITEST_ANY);
	litest_add(pad_ring_finger_up, LITEST_RING, LITEST_ANY);

	litest_add(pad_has_strip, LITEST_STRIP, LITEST_ANY);
	litest_add(pad_strip, LITEST_STRIP, LITEST_ANY);
	litest_add(pad_strip_finger_up, LITEST_STRIP, LITEST_ANY);

	litest_add_for_device(pad_left_handed_default, LITEST_WACOM_INTUOS5_PAD);
	litest_add_for_device(pad_no_left_handed, LITEST_WACOM_INTUOS3_PAD);
	litest_add_for_device(pad_left_handed_ring, LITEST_WACOM_INTUOS5_PAD);
	/* None of the current strip tablets are left-handed */

	litest_add(pad_mode_groups, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_mode_groups_userdata, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_mode_groups_ref, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_mode_group_mode, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_mode_group_has, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_mode_group_has_invalid, LITEST_TABLET_PAD, LITEST_ANY);
	litest_add(pad_mode_group_has_no_toggle, LITEST_TABLET_PAD, LITEST_ANY);

	litest_add(pad_keys, LITEST_TABLET_PAD, LITEST_ANY);
}
