/*
 * Copyright © 2013 Red Hat, Inc.
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

#ifndef LITEST_INT_H
#define LITEST_INT_H
#include "litest.h"

/* Use as designater for litest to change the value */
#define LITEST_AUTO_ASSIGN INT_MIN

/* Special event code to auto-assign the BTN_TOOL_PEN and friends */
#define LITEST_BTN_TOOL_AUTO (KEY_MAX << 1)

struct litest_test_device {
	struct list node; /* global test device list */

	enum litest_device_type type;
	int64_t features;
	const char *shortname;
	void (*setup)(void); /* test fixture, used by check */
	void (*teardown)(void); /* test fixture, used by check */
	/**
	* If create is non-NULL it will be called to initialize the device.
	* For such devices, no overrides are possible. If create is NULL,
	* the information in name, id, events, absinfo is used to
	* create the device instead.
	*
	* @return true if the device needs to be created by litest, false if
	*	the device creates itself
	*/
	bool (*create)(struct litest_device *d);

	/**
	 * The device name. Only used when create is NULL.
	 */
	const char *name;
	/**
	 * The device id. Only used when create is NULL.
	 */
	const struct input_id *id;
	/**
	* List of event type/code tuples, terminated with -1, e.g.
	* EV_REL, REL_X, EV_KEY, BTN_LEFT, -1
	* Special tuple is INPUT_PROP_MAX, <actual property> to set.
	*
	* Any EV_ABS codes in this list will be initialized with a default
	* axis range.
	*/
	int *events;
	/**
	 * List of abs codes to enable, with absinfo.value determining the
	 * code to set. List must be terminated with absinfo.value -1
	 */
	struct input_absinfo *absinfo;
	struct litest_device_interface *interface;

	const char *udev_rule;
	const char *quirk_file;

	const struct key_value_str udev_properties[32];
};

struct litest_device_interface {
	bool (*touch_down)(struct litest_device *d, unsigned int slot, double x, double y);
	bool (*touch_move)(struct litest_device *d, unsigned int slot, double x, double y);
	bool (*touch_up)(struct litest_device *d, unsigned int slot);

	/**
	 * Default value for the given EV_ABS axis.
	 * @return 0 on success, nonzero otherwise
	 */
	int (*get_axis_default)(struct litest_device *d, unsigned int code, int32_t *value);

	/**
	 * Set of of events to execute on touch down, terminated by a .type
	 * and .code value of -1. If the event value is LITEST_AUTO_ASSIGN,
	 * it will be automatically assigned by the framework (valid for x,
	 * y, tracking id and slot).
	 *
	 * These events are only used if touch_down is NULL.
	 */
	struct input_event *touch_down_events;
	struct input_event *touch_move_events;
	struct input_event *touch_up_events;

	/**
	 * Tablet events, LITEST_AUTO_ASSIGN is allowed on event values for
	 * ABS_X, ABS_Y, ABS_DISTANCE and ABS_PRESSURE.
	 */
	struct input_event *tablet_proximity_in_events;
	struct input_event *tablet_proximity_out_events;
	struct input_event *tablet_motion_events;

	bool (*tablet_proximity_in)(struct litest_device *d,
				    unsigned int tool_type,
				    double *x, double *y,
				    struct axis_replacement *axes);
	bool (*tablet_proximity_out)(struct litest_device *d, unsigned int tool_type);
	bool (*tablet_tip_down)(struct litest_device *d,
				double *x, double *y,
				struct axis_replacement *axes);
	bool (*tablet_tip_up)(struct litest_device *d,
			      double *x, double *y,
			      struct axis_replacement *axes);
	bool (*tablet_motion)(struct litest_device *d,
			      double *x, double *y,
			      struct axis_replacement *axes);

	/**
	 * Pad events, LITEST_AUTO_ASSIGN is allowed on event values
	 * for ABS_WHEEL
	 */
	struct input_event *pad_ring_start_events;
	struct input_event *pad_ring_change_events;
	struct input_event *pad_ring_end_events;

	/**
	 * Pad events, LITEST_AUTO_ASSIGN is allowed on event values
	 * for ABS_RX
	 */
	struct input_event *pad_strip_start_events;
	struct input_event *pad_strip_change_events;
	struct input_event *pad_strip_end_events;

	int min[2]; /* x/y axis minimum */
	int max[2]; /* x/y axis maximum */

	unsigned int tool_type;
};

struct path {
	struct list link;
	char *path;
	int fd;
};

struct litest_context {
	struct litest_user_data *user_data;
	struct list paths;
};

void litest_set_current_device(struct litest_device *device);
int litest_scale(const struct litest_device *d, unsigned int axis, double val);
void litest_generic_device_teardown(void);

#endif
