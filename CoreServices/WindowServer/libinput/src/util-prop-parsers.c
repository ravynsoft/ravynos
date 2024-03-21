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

#include "util-prop-parsers.h"

#include <libevdev/libevdev.h>
#include <string.h>

#include "util-macros.h"
#include "util-strings.h"

/* Helper function to parse the mouse DPI tag from udev.
 * The tag is of the form:
 * MOUSE_DPI=400 *1000 2000
 * or
 * MOUSE_DPI=400@125 *1000@125 2000@125
 * Where the * indicates the default value and @number indicates device poll
 * rate.
 * Numbers should be in ascending order, and if rates are present they should
 * be present for all entries.
 *
 * When parsing the mouse DPI property, if we find an error we just return 0
 * since it's obviously invalid, the caller will treat that as an error and
 * use a reasonable default instead. If the property contains multiple DPI
 * settings but none flagged as default, we return the last because we're
 * lazy and that's a silly way to set the property anyway.
 *
 * @param prop The value of the udev property (without the MOUSE_DPI=)
 * @return The default dpi value on success, 0 on error
 */
int
parse_mouse_dpi_property(const char *prop)
{
	bool is_default = false;
	int nread, dpi = 0, rate;

	if (!prop)
		return 0;

	while (*prop != 0) {
		if (*prop == ' ') {
			prop++;
			continue;
		}
		if (*prop == '*') {
			prop++;
			is_default = true;
			if (!isdigit(prop[0]))
				return 0;
		}

		/* While we don't do anything with the rate right now we
		 * will validate that, if it's present, it is non-zero and
		 * positive
		 */
		rate = 1;
		nread = 0;
		sscanf(prop, "%d@%d%n", &dpi, &rate, &nread);
		if (!nread)
			sscanf(prop, "%d%n", &dpi, &nread);
		if (!nread || dpi <= 0 || rate <= 0 || prop[nread] == '@')
			return 0;

		if (is_default)
			break;
		prop += nread;
	}
	return dpi;
}

/**
 * Helper function to parse the MOUSE_WHEEL_CLICK_COUNT property from udev.
 * Property is of the form:
 * MOUSE_WHEEL_CLICK_COUNT=<integer>
 * Where the number indicates the number of wheel clicks per 360 deg
 * rotation.
 *
 * @param prop The value of the udev property (without the MOUSE_WHEEL_CLICK_COUNT=)
 * @return The click count of the wheel (may be negative) or 0 on error.
 */
int
parse_mouse_wheel_click_count_property(const char *prop)
{
	int count = 0;

	if (!prop)
		return 0;

	if (!safe_atoi(prop, &count) || abs(count) > 360)
		return 0;

        return count;
}

/**
 *
 * Helper function to parse the MOUSE_WHEEL_CLICK_ANGLE property from udev.
 * Property is of the form:
 * MOUSE_WHEEL_CLICK_ANGLE=<integer>
 * Where the number indicates the degrees travelled for each click.
 *
 * @param prop The value of the udev property (without the MOUSE_WHEEL_CLICK_ANGLE=)
 * @return The angle of the wheel (may be negative) or 0 on error.
 */
int
parse_mouse_wheel_click_angle_property(const char *prop)
{
	int angle = 0;

	if (!prop)
		return 0;

	if (!safe_atoi(prop, &angle) || abs(angle) > 360)
		return 0;

        return angle;
}

/**
 * Parses a simple dimension string in the form of "10x40". The two
 * numbers must be positive integers in decimal notation.
 * On success, the two numbers are stored in w and h. On failure, w and h
 * are unmodified.
 *
 * @param prop The value of the property
 * @param w Returns the first component of the dimension
 * @param h Returns the second component of the dimension
 * @return true on success, false otherwise
 */
bool
parse_dimension_property(const char *prop, size_t *w, size_t *h)
{
	int x, y;

	if (!prop)
		return false;

	if (sscanf(prop, "%dx%d", &x, &y) != 2)
		return false;

	if (x <= 0 || y <= 0)
		return false;

	*w = (size_t)x;
	*h = (size_t)y;
	return true;
}

/**
 * Parses a set of 6 space-separated floats.
 *
 * @param prop The string value of the property
 * @param calibration Returns the six components
 * @return true on success, false otherwise
 */
bool
parse_calibration_property(const char *prop, float calibration_out[6])
{
	if (!prop)
		return false;

	bool rc = false;

	size_t num_calibration;
	char **strv = strv_from_string(prop, " ", &num_calibration);
	if (!strv || num_calibration < 6)
		goto out;

	float calibration[6];
	for (size_t idx = 0; idx < 6; idx++) {
		double v;
		if (!safe_atod(strv[idx], &v))
			goto out;

		calibration[idx] = v;
	}

	memcpy(calibration_out, calibration, sizeof(calibration));
	rc = true;

out:
	strv_free(strv);
	return rc;
}

bool
parse_switch_reliability_property(const char *prop,
				  enum switch_reliability *reliability)
{
	if (!prop) {
		*reliability = RELIABILITY_RELIABLE;
		return true;
	}

	if (streq(prop, "reliable"))
		*reliability = RELIABILITY_RELIABLE;
	else if (streq(prop, "unreliable"))
		*reliability = RELIABILITY_UNRELIABLE;
	else if (streq(prop, "write_open"))
		*reliability = RELIABILITY_WRITE_OPEN;
	else
		return false;

	return true;
}

/**
 * Parses a string with the allowed values: "below"
 * The value refers to the position of the touchpad (relative to the
 * keyboard, i.e. your average laptop would be 'below')
 *
 * @param prop The value of the property
 * @param layout The layout
 * @return true on success, false otherwise
 */
bool
parse_tpkbcombo_layout_poperty(const char *prop,
			       enum tpkbcombo_layout *layout)
{
	if (!prop)
		return false;

	if (streq(prop, "below")) {
		*layout = TPKBCOMBO_LAYOUT_BELOW;
		return true;
	}

	return false;
}

/**
 * Parses a string of the format "a:b" where both a and b must be integer
 * numbers and a > b. Also allowed is the special string value "none" which
 * amounts to unsetting the property.
 *
 * @param prop The value of the property
 * @param hi Set to the first digit or 0 in case of 'none'
 * @param lo Set to the second digit or 0 in case of 'none'
 * @return true on success, false otherwise
 */
bool
parse_range_property(const char *prop, int *hi, int *lo)
{
	int first, second;

	if (!prop)
		return false;

	if (streq(prop, "none")) {
		*hi = 0;
		*lo = 0;
		return true;
	}

	if (sscanf(prop, "%d:%d", &first, &second) != 2)
		return false;

	if (second >= first)
		return false;

	*hi = first;
	*lo = second;

	return true;
}

bool
parse_boolean_property(const char *prop, bool *b)
{
	if (!prop)
		return false;

	if (streq(prop, "1"))
		*b = true;
	else if (streq(prop, "0"))
		*b = false;
	else
		return false;

	return true;
}

static bool
parse_evcode_string(const char *s, int *type_out, int *code_out)
{
	int type, code;

	if (strneq(s, "EV_", 3)) {
		type = libevdev_event_type_from_name(s);
		if (type == -1)
			return false;

		code = EVENT_CODE_UNDEFINED;
	} else {
		struct map {
			const char *str;
			int type;
		} map[] = {
			{ "KEY_", EV_KEY },
			{ "BTN_", EV_KEY },
			{ "ABS_", EV_ABS },
			{ "REL_", EV_REL },
			{ "SW_", EV_SW },
		};
		bool found = false;

		ARRAY_FOR_EACH(map, m) {
			if (!strstartswith(s, m->str))
				continue;

			type = m->type;
			code = libevdev_event_code_from_name(type, s);
			if (code == -1)
				return false;

			found = true;
			break;
		}
		if (!found)
			return false;
	}

	*type_out = type;
	*code_out = code;

	return true;
}

/**
 * Parses a string of the format "+EV_ABS;+KEY_A;-BTN_TOOL_DOUBLETAP;-ABS_X;"
 * where each element must be + or - (enable/disable) followed by a named event
 * type OR a named event code OR a tuple in the form of EV_KEY:0x123, i.e. a
 * named event type followed by a hex event code.
 *
 * events must point to an existing array of size nevents.
 * nevents specifies the size of the array in events and returns the number
 * of items, elements exceeding nevents are simply ignored, just make sure
 * events is large enough for your use-case.
 *
 * The results are returned as input events with type and code set, all
 * other fields undefined. Where only the event type is specified, the code
 * is set to EVENT_CODE_UNDEFINED.
 *
 * On success, events contains nevents events with each event's value set to 1
 * or 0 depending on the + or - prefix.
 */
bool
parse_evcode_property(const char *prop, struct input_event *events, size_t *nevents)
{
	bool rc = false;
	/* A randomly chosen max so we avoid crazy quirks */
	struct input_event evs[32];

	memset(evs, 0, sizeof evs);

	size_t ncodes;
	char **strv = strv_from_string(prop, ";", &ncodes);
	if (!strv || ncodes == 0 || ncodes > ARRAY_LENGTH(evs))
		goto out;

	ncodes = min(*nevents, ncodes);
	for (size_t idx = 0; strv[idx]; idx++) {
		char *s = strv[idx];
		bool enable;

		switch (*s) {
		case '+': enable = true; break;
		case '-': enable = false; break;
		default:
			goto out;
		}

		s++;

		int type, code;

		if (strstr(s, ":") == NULL) {
			if (!parse_evcode_string(s, &type, &code))
				goto out;
		} else {
			int consumed;
			char stype[13] = {0}; /* EV_FF_STATUS + '\0' */

			if (sscanf(s, "%12[A-Z_]:%x%n", stype, &code, &consumed) != 2 ||
			    strlen(s) != (size_t)consumed ||
			    (type = libevdev_event_type_from_name(stype)) == -1 ||
			    code < 0 || code > libevdev_event_type_get_max(type))
			    goto out;
		}

		evs[idx].type = type;
		evs[idx].code = code;
		evs[idx].value = enable;
	}

	memcpy(events, evs, ncodes * sizeof *events);
	*nevents = ncodes;
	rc = true;

out:
	strv_free(strv);
	return rc;
}

/**
 * Parses a string of the format "+INPUT_PROP_BUTTONPAD;-INPUT_PROP_POINTER;+0x123;"
 * where each element must be a named input prop OR a hexcode in the form
 * 0x1234. The prefix for each element must be either '+' (enable) or '-' (disable).
 *
 * props must point to an existing array of size nprops.
 * nprops specifies the size of the array in props and returns the number
 * of elements, elements exceeding nprops are simply ignored, just make sure
 * props is large enough for your use-case.
 *
 * On success, props contains nprops elements.
 */
bool
parse_input_prop_property(const char *prop, struct input_prop *props_out, size_t *nprops)
{
	bool rc = false;
	struct input_prop props[INPUT_PROP_CNT]; /* doubling up on quirks is a bug */

	size_t count;
	char **strv = strv_from_string(prop, ";", &count);
	if (!strv || count == 0 || count > ARRAY_LENGTH(props))
		goto out;

	count = min(*nprops, count);
	for (size_t idx = 0; strv[idx]; idx++) {
		char *s = strv[idx];
		unsigned int prop;
		bool enable;

		switch (*s) {
		case '+': enable = true; break;
		case '-': enable = false; break;
		default:
			goto out;
		}

		s++;

		if (safe_atou_base(s, &prop, 16)) {
			if (prop > INPUT_PROP_MAX)
				goto out;
		} else {
			int val = libevdev_property_from_name(s);
			if (val == -1)
				goto out;
			prop = (unsigned int)val;
		}
		props[idx].prop = prop;
		props[idx].enabled = enable;
	}

	memcpy(props_out, props, count * sizeof *props);
	*nprops = count;
	rc = true;

out:
	strv_free(strv);
	return rc;
}

/**
 * Parse the property value for the EVDEV_ABS_00 properties. Spec is
 *  EVDEV_ABS_00=min:max:res:fuzz:flat
 * where any element may be empty and subsequent elements may not be
 * present. So we have to parse
 *  EVDEV_ABS_00=min:max:res
 *  EVDEV_ABS_00=::res
 *  EVDEV_ABS_00=::res:fuzz:
 *
 * Returns a mask of the bits set and the absinfo struct with the values.
 * The abs value for an unset bit is undefined.
 */
uint32_t
parse_evdev_abs_prop(const char *prop, struct input_absinfo *abs)
{
	char *str = safe_strdup(prop);
	char *current, *next;
	uint32_t mask = 0;
	int bit = ABS_MASK_MIN;
	int *val;
	int values[5];

	/* basic sanity check: 5 digits for min/max, 3 for resolution, fuzz,
	 * flat and the colons. That's plenty, anything over is garbage */
	if (!prop || strlen(prop) > 24)
		goto out;

	current = str;
	val = values;
	while (current && *current != '\0' && bit <= ABS_MASK_FLAT) {
		if (*current != ':') {
			int v;
			next = index(current, ':');
			if (next)
				*next = '\0';

			if (!safe_atoi(current, &v)) {
				mask = 0;
				goto out;
			}
			*val = v;
			mask |= bit;
			current = next ? ++next : NULL;
		} else {
			current++;
		}
		bit <<= 1;
		val++;
	}

	if (mask & ABS_MASK_MIN)
		abs->minimum = values[0];
	if (mask & ABS_MASK_MAX)
		abs->maximum = values[1];
	if (mask & ABS_MASK_RES)
		abs->resolution = values[2];
	if (mask & ABS_MASK_FUZZ)
		abs->fuzz = values[3];
	if (mask & ABS_MASK_FLAT)
		abs->flat = values[4];

out:
	free(str);

	return mask;
}
