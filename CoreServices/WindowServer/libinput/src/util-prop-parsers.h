/*
 * Copyright © 2013-2019 Red Hat, Inc.
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

#pragma once

#include "config.h"

#include <linux/input.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct input_prop {
	unsigned int prop;
	bool enabled;
};

int parse_mouse_dpi_property(const char *prop);
int parse_mouse_wheel_click_angle_property(const char *prop);
int parse_mouse_wheel_click_count_property(const char *prop);
bool parse_dimension_property(const char *prop, size_t *width, size_t *height);
bool parse_calibration_property(const char *prop, float calibration[6]);
bool parse_range_property(const char *prop, int *hi, int *lo);
bool parse_boolean_property(const char *prop, bool *b);
#define EVENT_CODE_UNDEFINED 0xffff
bool parse_evcode_property(const char *prop, struct input_event *events, size_t *nevents);
bool parse_input_prop_property(const char *prop, struct input_prop *props_out, size_t *nprops);

enum tpkbcombo_layout {
	TPKBCOMBO_LAYOUT_UNKNOWN,
	TPKBCOMBO_LAYOUT_BELOW,
};
bool parse_tpkbcombo_layout_poperty(const char *prop,
				    enum tpkbcombo_layout *layout);

enum switch_reliability {
	RELIABILITY_RELIABLE,
	RELIABILITY_UNRELIABLE,
	RELIABILITY_WRITE_OPEN,
};

bool
parse_switch_reliability_property(const char *prop,
				  enum switch_reliability *reliability);

enum {
	ABS_MASK_MIN = 0x1,
	ABS_MASK_MAX = 0x2,
	ABS_MASK_RES = 0x4,
	ABS_MASK_FUZZ = 0x8,
	ABS_MASK_FLAT = 0x10,
};

uint32_t parse_evdev_abs_prop(const char *prop, struct input_absinfo *abs);
