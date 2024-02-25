/*
 * Copyright © 2014 Red Hat, Inc.
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

#include <valgrind/valgrind.h>

#include "util-list.h"
#include "util-strings.h"
#include "util-time.h"
#include "util-prop-parsers.h"
#include "util-macros.h"
#include "util-bits.h"
#include "util-ratelimit.h"
#include "util-matrix.h"

#define  TEST_VERSIONSORT
#include "libinput-versionsort.h"

#include "check-double-macros.h"

START_TEST(array_for_each)
{
	int ai[6];
	char ac[10];
	struct as {
		int a;
		char b;
		int *ptr;
	} as[32];

	for (size_t i = 0; i < 6; i++)
		ai[i] = 20 + i;
	for (size_t i = 0; i < 10; i++)
		ac[i] = 100 + i;
	for (size_t i = 0; i < 32; i++) {
		as[i].a = 10 + i;
		as[i].b = 20 + i;
		as[i].ptr = (int*)0xab + i;
	}

	int iexpected = 20;
	ARRAY_FOR_EACH(ai, entry) {
		ck_assert_int_eq(*entry, iexpected);
		++iexpected;
	}
	ck_assert_int_eq(iexpected, 26);

	int cexpected = 100;
	ARRAY_FOR_EACH(ac, entry) {
		ck_assert_int_eq(*entry, cexpected);
		++cexpected;
	}
	ck_assert_int_eq(cexpected, 110);

	struct as sexpected = {
		.a = 10,
		.b = 20,
		.ptr = (int*)0xab,
	};
	ARRAY_FOR_EACH(as, entry) {
		ck_assert_int_eq(entry->a, sexpected.a);
		ck_assert_int_eq(entry->b, sexpected.b);
		ck_assert_ptr_eq(entry->ptr, sexpected.ptr);
		++sexpected.a;
		++sexpected.b;
		++sexpected.ptr;
	}
	ck_assert_int_eq(sexpected.a, 42);
}
END_TEST

START_TEST(bitfield_helpers)
{
	/* This value has a bit set on all of the word boundaries we want to
	 * test: 0, 1, 7, 8, 31, 32, and 33
	 */
	unsigned char read_bitfield[] = { 0x83, 0x1, 0x0, 0x80, 0x3 };
	unsigned char write_bitfield[ARRAY_LENGTH(read_bitfield)] = {0};
	size_t i;

	/* Now check that the bitfield we wrote to came out to be the same as
	 * the bitfield we were writing from */
	for (i = 0; i < ARRAY_LENGTH(read_bitfield) * 8; i++) {
		switch (i) {
		case 0:
		case 1:
		case 7:
		case 8:
		case 31:
		case 32:
		case 33:
			ck_assert(bit_is_set(read_bitfield, i));
			set_bit(write_bitfield, i);
			break;
		default:
			ck_assert(!bit_is_set(read_bitfield, i));
			clear_bit(write_bitfield, i);
			break;
		}
	}

	ck_assert_int_eq(memcmp(read_bitfield,
				write_bitfield,
				sizeof(read_bitfield)),
			 0);
}
END_TEST

START_TEST(matrix_helpers)
{
	struct matrix m1, m2, m3;
	float f[6] = { 1, 2, 3, 4, 5, 6 };
	int x, y;
	int row, col;

	matrix_init_identity(&m1);

	for (row = 0; row < 3; row++) {
		for (col = 0; col < 3; col++) {
			ck_assert_int_eq(m1.val[row][col],
					 (row == col) ? 1 : 0);
		}
	}
	ck_assert(matrix_is_identity(&m1));

	matrix_from_farray6(&m2, f);
	ck_assert_int_eq(m2.val[0][0], 1);
	ck_assert_int_eq(m2.val[0][1], 2);
	ck_assert_int_eq(m2.val[0][2], 3);
	ck_assert_int_eq(m2.val[1][0], 4);
	ck_assert_int_eq(m2.val[1][1], 5);
	ck_assert_int_eq(m2.val[1][2], 6);
	ck_assert_int_eq(m2.val[2][0], 0);
	ck_assert_int_eq(m2.val[2][1], 0);
	ck_assert_int_eq(m2.val[2][2], 1);

	x = 100;
	y = 5;
	matrix_mult_vec(&m1, &x, &y);
	ck_assert_int_eq(x, 100);
	ck_assert_int_eq(y, 5);

	matrix_mult(&m3, &m1, &m1);
	ck_assert(matrix_is_identity(&m3));

	matrix_init_scale(&m2, 2, 4);
	ck_assert_int_eq(m2.val[0][0], 2);
	ck_assert_int_eq(m2.val[0][1], 0);
	ck_assert_int_eq(m2.val[0][2], 0);
	ck_assert_int_eq(m2.val[1][0], 0);
	ck_assert_int_eq(m2.val[1][1], 4);
	ck_assert_int_eq(m2.val[1][2], 0);
	ck_assert_int_eq(m2.val[2][0], 0);
	ck_assert_int_eq(m2.val[2][1], 0);
	ck_assert_int_eq(m2.val[2][2], 1);

	matrix_mult_vec(&m2, &x, &y);
	ck_assert_int_eq(x, 200);
	ck_assert_int_eq(y, 20);

	matrix_init_translate(&m2, 10, 100);
	ck_assert_int_eq(m2.val[0][0], 1);
	ck_assert_int_eq(m2.val[0][1], 0);
	ck_assert_int_eq(m2.val[0][2], 10);
	ck_assert_int_eq(m2.val[1][0], 0);
	ck_assert_int_eq(m2.val[1][1], 1);
	ck_assert_int_eq(m2.val[1][2], 100);
	ck_assert_int_eq(m2.val[2][0], 0);
	ck_assert_int_eq(m2.val[2][1], 0);
	ck_assert_int_eq(m2.val[2][2], 1);

	matrix_mult_vec(&m2, &x, &y);
	ck_assert_int_eq(x, 210);
	ck_assert_int_eq(y, 120);

	matrix_to_farray6(&m2, f);
	ck_assert_int_eq(f[0], 1);
	ck_assert_int_eq(f[1], 0);
	ck_assert_int_eq(f[2], 10);
	ck_assert_int_eq(f[3], 0);
	ck_assert_int_eq(f[4], 1);
	ck_assert_int_eq(f[5], 100);
}
END_TEST

START_TEST(ratelimit_helpers)
{
	struct ratelimit rl;
	unsigned int i, j;

	/* 10 attempts every 1000ms */
	ratelimit_init(&rl, ms2us(1000), 10);

	for (j = 0; j < 3; ++j) {
		/* a burst of 9 attempts must succeed */
		for (i = 0; i < 9; ++i) {
			ck_assert_int_eq(ratelimit_test(&rl),
					 RATELIMIT_PASS);
		}

		/* the 10th attempt reaches the threshold */
		ck_assert_int_eq(ratelimit_test(&rl), RATELIMIT_THRESHOLD);

		/* ..then further attempts must fail.. */
		ck_assert_int_eq(ratelimit_test(&rl), RATELIMIT_EXCEEDED);

		/* ..regardless of how often we try. */
		for (i = 0; i < 100; ++i) {
			ck_assert_int_eq(ratelimit_test(&rl),
					 RATELIMIT_EXCEEDED);
		}

		/* ..even after waiting 20ms */
		msleep(100);
		for (i = 0; i < 100; ++i) {
			ck_assert_int_eq(ratelimit_test(&rl),
					 RATELIMIT_EXCEEDED);
		}

		/* but after 1000ms the counter is reset */
		msleep(950); /* +50ms to account for time drifts */
	}
}
END_TEST

struct parser_test {
	char *tag;
	int expected_value;
};

START_TEST(dpi_parser)
{
	struct parser_test tests[] = {
		{ "450 *1800 3200", 1800 },
		{ "*450 1800 3200", 450 },
		{ "450 1800 *3200", 3200 },
		{ "450 1800 3200", 3200 },
		{ "450 1800 failboat", 0 },
		{ "450 1800 *failboat", 0 },
		{ "0 450 1800 *3200", 0 },
		{ "450@37 1800@12 *3200@6", 3200 },
		{ "450@125 1800@125   *3200@125  ", 3200 },
		{ "450@125 *1800@125  3200@125", 1800 },
		{ "*this @string fails", 0 },
		{ "12@34 *45@", 0 },
		{ "12@a *45@", 0 },
		{ "12@a *45@25", 0 },
		{ "                                      * 12, 450, 800", 0 },
		{ "                                      *12, 450, 800", 12 },
		{ "*12, *450, 800", 12 },
		{ "*-23412, 450, 800", 0 },
		{ "112@125, 450@125, 800@125, 900@-125", 0 },
		{ "", 0 },
		{ "   ", 0 },
		{ "* ", 0 },
		{ NULL, 0 }
	};
	int i, dpi;

	for (i = 0; tests[i].tag != NULL; i++) {
		dpi = parse_mouse_dpi_property(tests[i].tag);
		ck_assert_int_eq(dpi, tests[i].expected_value);
	}

	dpi = parse_mouse_dpi_property(NULL);
	ck_assert_int_eq(dpi, 0);
}
END_TEST

START_TEST(wheel_click_parser)
{
	struct parser_test tests[] = {
		{ "1", 1 },
		{ "10", 10 },
		{ "-12", -12 },
		{ "360", 360 },

		{ "0", 0 },
		{ "-0", 0 },
		{ "a", 0 },
		{ "10a", 0 },
		{ "10-", 0 },
		{ "sadfasfd", 0 },
		{ "361", 0 },
		{ NULL, 0 }
	};

	int i, angle;

	for (i = 0; tests[i].tag != NULL; i++) {
		angle = parse_mouse_wheel_click_angle_property(tests[i].tag);
		ck_assert_int_eq(angle, tests[i].expected_value);
	}
}
END_TEST

START_TEST(wheel_click_count_parser)
{
	struct parser_test tests[] = {
		{ "1", 1 },
		{ "10", 10 },
		{ "-12", -12 },
		{ "360", 360 },

		{ "0", 0 },
		{ "-0", 0 },
		{ "a", 0 },
		{ "10a", 0 },
		{ "10-", 0 },
		{ "sadfasfd", 0 },
		{ "361", 0 },
		{ NULL, 0 }
	};

	int i, angle;

	for (i = 0; tests[i].tag != NULL; i++) {
		angle = parse_mouse_wheel_click_count_property(tests[i].tag);
		ck_assert_int_eq(angle, tests[i].expected_value);
	}

	angle = parse_mouse_wheel_click_count_property(NULL);
	ck_assert_int_eq(angle, 0);
}
END_TEST

START_TEST(dimension_prop_parser)
{
	struct parser_test_dimension {
		char *tag;
		bool success;
		int x, y;
	} tests[] = {
		{ "10x10", true, 10, 10 },
		{ "1x20", true, 1, 20 },
		{ "1x8000", true, 1, 8000 },
		{ "238492x428210", true, 238492, 428210 },
		{ "0x0", false, 0, 0 },
		{ "-10x10", false, 0, 0 },
		{ "-1", false, 0, 0 },
		{ "1x-99", false, 0, 0 },
		{ "0", false, 0, 0 },
		{ "100", false, 0, 0 },
		{ "", false, 0, 0 },
		{ "abd", false, 0, 0 },
		{ "xabd", false, 0, 0 },
		{ "0xaf", false, 0, 0 },
		{ "0x0x", false, 0, 0 },
		{ "x10", false, 0, 0 },
		{ NULL, false, 0, 0 }
	};
	int i;
	size_t x, y;
	bool success;

	for (i = 0; tests[i].tag != NULL; i++) {
		x = y = 0xad;
		success = parse_dimension_property(tests[i].tag, &x, &y);
		ck_assert(success == tests[i].success);
		if (success) {
			ck_assert_int_eq(x, tests[i].x);
			ck_assert_int_eq(y, tests[i].y);
		} else {
			ck_assert_int_eq(x, 0xad);
			ck_assert_int_eq(y, 0xad);
		}
	}

	success = parse_dimension_property(NULL, &x, &y);
	ck_assert(success == false);
}
END_TEST

START_TEST(reliability_prop_parser)
{
	struct parser_test_reliability {
		char *tag;
		bool success;
		enum switch_reliability reliability;
	} tests[] = {
		{ "reliable", true, RELIABILITY_RELIABLE },
		{ "unreliable", true, RELIABILITY_UNRELIABLE },
		{ "write_open", true, RELIABILITY_WRITE_OPEN },
		{ "", false, 0 },
		{ "0", false, 0 },
		{ "1", false, 0 },
		{ NULL, false, 0, }
	};
	enum switch_reliability r;
	bool success;
	int i;

	for (i = 0; tests[i].tag != NULL; i++) {
		r = 0xaf;
		success = parse_switch_reliability_property(tests[i].tag, &r);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(r, tests[i].reliability);
		else
			ck_assert_int_eq(r, 0xaf);
	}

	success = parse_switch_reliability_property(NULL, &r);
	ck_assert(success == true);
	ck_assert_int_eq(r, RELIABILITY_RELIABLE);

	success = parse_switch_reliability_property("foo", NULL);
	ck_assert(success == false);
}
END_TEST

START_TEST(calibration_prop_parser)
{
#define DEFAULT_VALUES { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }
	const float untouched[6] = DEFAULT_VALUES;
	struct parser_test_calibration {
		char *prop;
		bool success;
		float values[6];
	} tests[] = {
		{ "", false, DEFAULT_VALUES },
		{ "banana", false, DEFAULT_VALUES },
		{ "1 2 3 a 5 6", false, DEFAULT_VALUES },
		{ "2", false, DEFAULT_VALUES },
		{ "2 3 4 5 6", false, DEFAULT_VALUES },
		{ "1 2 3 4 5 6", true, DEFAULT_VALUES },
		{ "6.00012 3.244 4.238 5.2421 6.0134 8.860", true,
			{ 6.00012, 3.244, 4.238, 5.2421, 6.0134, 8.860 }},
		{ "0xff 2 3 4 5 6", false, DEFAULT_VALUES },
		{ NULL, false, DEFAULT_VALUES }
	};
	bool success;
	float calibration[6];
	int rc;
	int i;

	for (i = 0; tests[i].prop != NULL; i++) {
		memcpy(calibration, untouched, sizeof(calibration));

		success = parse_calibration_property(tests[i].prop,
						     calibration);
		ck_assert_int_eq(success, tests[i].success);
		if (success)
			rc = memcmp(tests[i].values,
				    calibration,
				    sizeof(calibration));
		else
			rc = memcmp(untouched,
				    calibration,
				    sizeof(calibration));
		ck_assert_int_eq(rc, 0);
	}

	memcpy(calibration, untouched, sizeof(calibration));

	success = parse_calibration_property(NULL, calibration);
	ck_assert(success == false);
	rc = memcmp(untouched, calibration, sizeof(calibration));
	ck_assert_int_eq(rc, 0);
}
END_TEST

START_TEST(range_prop_parser)
{
	struct parser_test_range {
		char *tag;
		bool success;
		int hi, lo;
	} tests[] = {
		{ "10:8", true, 10, 8 },
		{ "100:-1", true, 100, -1 },
		{ "-203813:-502023", true, -203813, -502023 },
		{ "238492:28210", true, 238492, 28210 },
		{ "none", true, 0, 0 },
		{ "0:0", false, 0, 0 },
		{ "", false, 0, 0 },
		{ "abcd", false, 0, 0 },
		{ "10:30:10", false, 0, 0 },
		{ NULL, false, 0, 0 }
	};
	int i;
	int hi, lo;
	bool success;

	for (i = 0; tests[i].tag != NULL; i++) {
		hi = lo = 0xad;
		success = parse_range_property(tests[i].tag, &hi, &lo);
		ck_assert(success == tests[i].success);
		if (success) {
			ck_assert_int_eq(hi, tests[i].hi);
			ck_assert_int_eq(lo, tests[i].lo);
		} else {
			ck_assert_int_eq(hi, 0xad);
			ck_assert_int_eq(lo, 0xad);
		}
	}

	success = parse_range_property(NULL, NULL, NULL);
	ck_assert(success == false);
}
END_TEST

START_TEST(boolean_prop_parser)
{
	struct parser_test_range {
		char *tag;
		bool success;
		bool b;
	} tests[] = {
		{ "0", true, false },
		{ "1", true, true },
		{ "-1", false, false },
		{ "2", false, false },
		{ "abcd", false, false },
		{ NULL, false, false }
	};
	int i;
	bool success, b;

	for (i = 0; tests[i].tag != NULL; i++) {
		b = false;
		success = parse_boolean_property(tests[i].tag, &b);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(b, tests[i].b);
		else
			ck_assert_int_eq(b, false);
	}

	success = parse_boolean_property(NULL, NULL);
	ck_assert(success == false);
}
END_TEST

START_TEST(evcode_prop_parser)
{
	struct parser_test_tuple {
		const char *prop;
		bool success;
		size_t nevents;
		struct input_event events[20];
	} tests[] = {
		{ "+EV_KEY", true, 1, {{ .type = EV_KEY, .code = 0xffff, .value = 1 }} },
		{ "-EV_ABS;", true, 1, {{ .type = EV_ABS, .code = 0xffff, .value = 0 }} },
		{ "+ABS_X;", true, 1, {{ .type = EV_ABS, .code = ABS_X, .value = 1 }} },
		{ "-SW_TABLET_MODE;", true, 1, {{ .type = EV_SW, .code = SW_TABLET_MODE, .value = 0 }} },
		{ "+EV_SW", true, 1, {{ .type = EV_SW, .code = 0xffff, .value = 1 }} },
		{ "-ABS_Y", true, 1, {{ .type = EV_ABS, .code = ABS_Y, .value = 0 }} },
		{ "+EV_ABS:0x00", true, 1, {{ .type = EV_ABS, .code = ABS_X, .value = 1 }} },
		{ "-EV_ABS:01", true, 1, {{ .type = EV_ABS, .code = ABS_Y, .value = 0 }} },
		{ "+ABS_TILT_X;-ABS_TILT_Y;", true, 2,
			{{ .type = EV_ABS, .code = ABS_TILT_X, .value = 1 },
			 { .type = EV_ABS, .code = ABS_TILT_Y, .value = 0}} },
		{ "+BTN_TOOL_DOUBLETAP;+EV_KEY;-KEY_A", true, 3,
			{{ .type = EV_KEY, .code = BTN_TOOL_DOUBLETAP, .value = 1 } ,
			 { .type = EV_KEY, .code = 0xffff, .value = 1 },
			 { .type = EV_KEY, .code = KEY_A, .value = 0 }} },
		{ "+REL_Y;-ABS_Z;+BTN_STYLUS", true, 3,
			{{ .type = EV_REL, .code = REL_Y, .value = 1},
			 { .type = EV_ABS, .code = ABS_Z, .value = 0},
			 { .type = EV_KEY, .code = BTN_STYLUS, .value = 1 }} },
		{ "-REL_Y;+EV_KEY:0x123;-BTN_STYLUS", true, 3,
			{{ .type = EV_REL, .code = REL_Y, .value = 0 },
			 { .type = EV_KEY, .code = 0x123, .value = 1 },
			 { .type = EV_KEY, .code = BTN_STYLUS, .value = 0 }} },
		{ .prop = "", .success = false },
		{ .prop = "+", .success = false },
		{ .prop = "-", .success = false },
		{ .prop = "!", .success = false },
		{ .prop = "+EV_FOO", .success = false },
		{ .prop = "+EV_KEY;-EV_FOO", .success = false },
		{ .prop = "+BTN_STYLUS;-EV_FOO", .success = false },
		{ .prop = "-BTN_UNKNOWN", .success = false },
		{ .prop = "+BTN_UNKNOWN;+EV_KEY", .success = false },
		{ .prop = "-PR_UNKNOWN", .success = false },
		{ .prop = "-BTN_STYLUS;+PR_UNKNOWN;-ABS_X", .success = false },
		{ .prop = "-EV_REL:0xffff", .success = false },
		{ .prop = "-EV_REL:0x123.", .success = false },
		{ .prop = "-EV_REL:ffff", .success = false },
		{ .prop = "-EV_REL:blah", .success = false },
		{ .prop = "+KEY_A:0x11", .success = false },
		{ .prop = "+EV_KEY:0x11 ", .success = false },
		{ .prop = "+EV_KEY:0x11not", .success = false },
		{ .prop = "none", .success = false },
		{ .prop = NULL },
	};
	struct parser_test_tuple *t;

	for (int i = 0; tests[i].prop; i++) {
		bool success;
		struct input_event events[32];
		size_t nevents = ARRAY_LENGTH(events);

		t = &tests[i];
		success = parse_evcode_property(t->prop, events, &nevents);
		ck_assert(success == t->success);
		if (!success)
			continue;

		ck_assert_int_eq(nevents, t->nevents);
		for (size_t j = 0; j < nevents; j++) {
			unsigned int type = events[j].type;
			unsigned int code = events[j].code;
			int value = events[j].value;
			ck_assert_int_eq(t->events[j].type, type);
			ck_assert_int_eq(t->events[j].code, code);
			ck_assert_int_eq(t->events[j].value, value);
		}
	}
}
END_TEST

START_TEST(input_prop_parser)
{
	struct parser_test_val {
		const char *prop;
		bool success;
		size_t nvals;
		struct input_prop values[20];
	} tests[] = {
		{ "+INPUT_PROP_BUTTONPAD", true, 1, {{ INPUT_PROP_BUTTONPAD, true }}},
		{ "+INPUT_PROP_BUTTONPAD;-INPUT_PROP_POINTER", true, 2,
			{ { INPUT_PROP_BUTTONPAD, true },
			  { INPUT_PROP_POINTER, false }}},
		{ "+INPUT_PROP_BUTTONPAD;-0x00;+0x03", true, 3,
			{ { INPUT_PROP_BUTTONPAD, true },
			  { INPUT_PROP_POINTER, false },
			  { INPUT_PROP_SEMI_MT, true }}},
		{ .prop = "", .success = false },
		{ .prop = "0xff", .success = false },
		{ .prop = "INPUT_PROP", .success = false },
		{ .prop = "INPUT_PROP_FOO", .success = false },
		{ .prop = "INPUT_PROP_FOO;INPUT_PROP_FOO", .success = false },
		{ .prop = "INPUT_PROP_POINTER;INPUT_PROP_FOO", .success = false },
		{ .prop = "none", .success = false },
		{ .prop = NULL },
	};
	struct parser_test_val *t;

	for (int i = 0; tests[i].prop; i++) {
		bool success;
		struct input_prop props[32];
		size_t nprops = ARRAY_LENGTH(props);

		t = &tests[i];
		success = parse_input_prop_property(t->prop, props, &nprops);
		ck_assert(success == t->success);
		if (!success)
			continue;

		ck_assert_int_eq(nprops, t->nvals);
		for (size_t j = 0; j < t->nvals; j++) {
			ck_assert_int_eq(t->values[j].prop, props[j].prop);
			ck_assert_int_eq(t->values[j].enabled, props[j].enabled);
		}
	}
}
END_TEST

START_TEST(evdev_abs_parser)
{
	struct test {
		uint32_t which;
		const char *prop;
		int min, max, res, fuzz, flat;

	} tests[] = {
		{ .which = (ABS_MASK_MIN|ABS_MASK_MAX),
		  .prop = "1:2",
		  .min = 1, .max = 2 },
		{ .which = (ABS_MASK_MIN|ABS_MASK_MAX),
		  .prop = "1:2:",
		  .min = 1, .max = 2 },
		{ .which = (ABS_MASK_MIN|ABS_MASK_MAX|ABS_MASK_RES),
		  .prop = "10:20:30",
		  .min = 10, .max = 20, .res = 30 },
		{ .which = (ABS_MASK_RES),
		  .prop = "::100",
		  .res = 100 },
		{ .which = (ABS_MASK_MIN),
		  .prop = "10:",
		  .min = 10 },
		{ .which = (ABS_MASK_MAX|ABS_MASK_RES),
		  .prop = ":10:1001",
		  .max = 10, .res = 1001 },
		{ .which = (ABS_MASK_MIN|ABS_MASK_MAX|ABS_MASK_RES|ABS_MASK_FUZZ),
		  .prop = "1:2:3:4",
		  .min = 1, .max = 2, .res = 3, .fuzz = 4},
		{ .which = (ABS_MASK_MIN|ABS_MASK_MAX|ABS_MASK_RES|ABS_MASK_FUZZ|ABS_MASK_FLAT),
		  .prop = "1:2:3:4:5",
		  .min = 1, .max = 2, .res = 3, .fuzz = 4, .flat = 5},
		{ .which = (ABS_MASK_MIN|ABS_MASK_RES|ABS_MASK_FUZZ|ABS_MASK_FLAT),
		  .prop = "1::3:4:50",
		  .min = 1, .res = 3, .fuzz = 4, .flat = 50},
		{ .which = ABS_MASK_FUZZ|ABS_MASK_FLAT,
		  .prop = ":::5:60",
		  .fuzz = 5, .flat = 60},
		{ .which = ABS_MASK_FUZZ,
		  .prop = ":::5:",
		  .fuzz = 5 },
		{ .which = ABS_MASK_RES, .prop = "::12::",
		  .res = 12 },
		/* Malformed property but parsing this one makes us more
		 * future proof */
		{ .which = (ABS_MASK_RES|ABS_MASK_FUZZ|ABS_MASK_FLAT),
		  .prop = "::12:1:2:3:4:5:6",
		  .res = 12, .fuzz = 1, .flat = 2 },
		{ .which = 0, .prop = ":::::" },
		{ .which = 0, .prop = ":" },
		{ .which = 0, .prop = "" },
		{ .which = 0, .prop = ":asb::::" },
		{ .which = 0, .prop = "foo" },
	};

	ARRAY_FOR_EACH(tests, t) {
		struct input_absinfo abs;
		uint32_t mask;

		mask = parse_evdev_abs_prop(t->prop, &abs);
		ck_assert_int_eq(mask, t->which);

		if (t->which & ABS_MASK_MIN)
			ck_assert_int_eq(abs.minimum, t->min);
		if (t->which & ABS_MASK_MAX)
			ck_assert_int_eq(abs.maximum, t->max);
		if (t->which & ABS_MASK_RES)
			ck_assert_int_eq(abs.resolution, t->res);
		if (t->which & ABS_MASK_FUZZ)
			ck_assert_int_eq(abs.fuzz, t->fuzz);
		if (t->which & ABS_MASK_FLAT)
			ck_assert_int_eq(abs.flat, t->flat);
	}
}
END_TEST

START_TEST(time_conversion)
{
	ck_assert_int_eq(us(10), 10);
	ck_assert_int_eq(ns2us(10000), 10);
	ck_assert_int_eq(ms2us(10), 10000);
	ck_assert_int_eq(s2us(1), 1000000);
	ck_assert_int_eq(h2us(2), s2us(2 * 60 * 60));
	ck_assert_int_eq(us2ms(10000), 10);
}
END_TEST

START_TEST(human_time)
{
	struct ht_tests {
		uint64_t interval;
		unsigned int value;
		const char *unit;
	} tests[] = {
		{ 0, 0, "us" },
		{ 123, 123, "us" },
		{ ms2us(5), 5, "ms" },
		{ ms2us(100), 100, "ms" },
		{ s2us(5), 5, "s" },
		{ s2us(100), 100, "s" },
		{ s2us(120), 2, "min" },
		{ 5 * s2us(60), 5, "min" },
		{ 120 * s2us(60), 2, "h" },
		{ 5 * 60 * s2us(60), 5, "h" },
		{ 48 * 60 * s2us(60), 2, "d" },
		{ 1000 * 24 * 60 * s2us(60), 1000, "d" },
		{ 0, 0, NULL },
	};
	for (int i = 0; tests[i].unit != NULL; i++) {
		struct human_time ht;

		ht = to_human_time(tests[i].interval);
		ck_assert_int_eq(ht.value, tests[i].value);
		ck_assert_str_eq(ht.unit, tests[i].unit);
	}
}
END_TEST

struct atoi_test {
	char *str;
	bool success;
	int val;
};

START_TEST(safe_atoi_test)
{
	struct atoi_test tests[] = {
		{ "10", true, 10 },
		{ "20", true, 20 },
		{ "-1", true, -1 },
		{ "2147483647", true, 2147483647 },
		{ "-2147483648", true, -2147483648 },
		{ "4294967295", false, 0 },
		{ "0x0", false, 0 },
		{ "-10x10", false, 0 },
		{ "1x-99", false, 0 },
		{ "", false, 0 },
		{ "abd", false, 0 },
		{ "xabd", false, 0 },
		{ "0xaf", false, 0 },
		{ "0x0x", false, 0 },
		{ "x10", false, 0 },
		{ NULL, false, 0 }
	};
	int v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atoi(tests[i].str, &v);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

START_TEST(safe_atoi_base_16_test)
{
	struct atoi_test tests[] = {
		{ "10", true, 0x10 },
		{ "20", true, 0x20 },
		{ "-1", true, -1 },
		{ "0x10", true, 0x10 },
		{ "0xff", true, 0xff },
		{ "abc", true, 0xabc },
		{ "-10", true, -0x10 },
		{ "0x0", true, 0 },
		{ "0", true, 0 },
		{ "0x-99", false, 0 },
		{ "0xak", false, 0 },
		{ "0x", false, 0 },
		{ "x10", false, 0 },
		{ NULL, false, 0 }
	};

	int v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atoi_base(tests[i].str, &v, 16);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

START_TEST(safe_atoi_base_8_test)
{
	struct atoi_test tests[] = {
		{ "7", true, 07 },
		{ "10", true, 010 },
		{ "20", true, 020 },
		{ "-1", true, -1 },
		{ "010", true, 010 },
		{ "0ff", false, 0 },
		{ "abc", false, 0},
		{ "0xabc", false, 0},
		{ "-10", true, -010 },
		{ "0", true, 0 },
		{ "00", true, 0 },
		{ "0x0", false, 0 },
		{ "0x-99", false, 0 },
		{ "0xak", false, 0 },
		{ "0x", false, 0 },
		{ "x10", false, 0 },
		{ NULL, false, 0 }
	};

	int v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atoi_base(tests[i].str, &v, 8);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

struct atou_test {
	char *str;
	bool success;
	unsigned int val;
};

START_TEST(safe_atou_test)
{
	struct atou_test tests[] = {
		{ "10", true, 10 },
		{ "20", true, 20 },
		{ "-1", false, 0 },
		{ "2147483647", true, 2147483647 },
		{ "-2147483648", false, 0},
		{ "0x0", false, 0 },
		{ "-10x10", false, 0 },
		{ "1x-99", false, 0 },
		{ "", false, 0 },
		{ "abd", false, 0 },
		{ "xabd", false, 0 },
		{ "0xaf", false, 0 },
		{ "0x0x", false, 0 },
		{ "x10", false, 0 },
		{ NULL, false, 0 }
	};
	unsigned int v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atou(tests[i].str, &v);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

START_TEST(safe_atou_base_16_test)
{
	struct atou_test tests[] = {
		{ "10", true, 0x10 },
		{ "20", true, 0x20 },
		{ "-1", false, 0 },
		{ "0x10", true, 0x10 },
		{ "0xff", true, 0xff },
		{ "abc", true, 0xabc },
		{ "-10", false, 0 },
		{ "0x0", true, 0 },
		{ "0", true, 0 },
		{ "0x-99", false, 0 },
		{ "0xak", false, 0 },
		{ "0x", false, 0 },
		{ "x10", false, 0 },
		{ NULL, false, 0 }
	};

	unsigned int v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atou_base(tests[i].str, &v, 16);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

START_TEST(safe_atou_base_8_test)
{
	struct atou_test tests[] = {
		{ "7", true, 07 },
		{ "10", true, 010 },
		{ "20", true, 020 },
		{ "-1", false, 0 },
		{ "010", true, 010 },
		{ "0ff", false, 0 },
		{ "abc", false, 0},
		{ "0xabc", false, 0},
		{ "-10", false, 0 },
		{ "0", true, 0 },
		{ "00", true, 0 },
		{ "0x0", false, 0 },
		{ "0x-99", false, 0 },
		{ "0xak", false, 0 },
		{ "0x", false, 0 },
		{ "x10", false, 0 },
		{ NULL, false, 0 }
	};

	unsigned int v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atou_base(tests[i].str, &v, 8);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

START_TEST(safe_atod_test)
{
	struct atod_test {
		char *str;
		bool success;
		double val;
	} tests[] = {
		{ "10", true, 10 },
		{ "20", true, 20 },
		{ "-1", true, -1 },
		{ "2147483647", true, 2147483647 },
		{ "-2147483648", true, -2147483648 },
		{ "4294967295", true, 4294967295 },
		{ "0x0", false, 0 },
		{ "0x10", false, 0 },
		{ "0xaf", false, 0 },
		{ "x80", false, 0 },
		{ "0.0", true, 0.0 },
		{ "0.1", true, 0.1 },
		{ "1.2", true, 1.2 },
		{ "-324.9", true, -324.9 },
		{ "9324.9", true, 9324.9 },
		{ "NAN", false, 0 },
		{ "INFINITY", false, 0 },
		{ "-10x10", false, 0 },
		{ "1x-99", false, 0 },
		{ "", false, 0 },
		{ "abd", false, 0 },
		{ "xabd", false, 0 },
		{ "0x0x", false, 0 },
		{ NULL, false, 0 }
	};
	double v;
	bool success;

	for (int i = 0; tests[i].str != NULL; i++) {
		v = 0xad;
		success = safe_atod(tests[i].str, &v);
		ck_assert(success == tests[i].success);
		if (success)
			ck_assert_int_eq(v, tests[i].val);
		else
			ck_assert_int_eq(v, 0xad);
	}
}
END_TEST

START_TEST(strsplit_test)
{
	struct strsplit_test {
		const char *string;
		const char *delim;
		const char *results[10];
		const size_t nresults;
	} tests[] = {
		{ "one two three", " ", { "one", "two", "three", NULL }, 3 },
		{ "one two\tthree", " \t", { "one", "two", "three", NULL }, 3 },
		{ "one", " ", { "one", NULL }, 1 },
		{ "one two ", " ", { "one", "two", NULL }, 2 },
		{ "one  two", " ", { "one", "two", NULL }, 2 },
		{ " one two", " ", { "one", "two", NULL }, 2 },
		{ "one", "\t \r", { "one", NULL }, 1 },
		{ "one two three", " t", { "one", "wo", "hree", NULL }, 3 },
		{ " one two three", "te", { " on", " ", "wo ", "hr", NULL }, 4 },
		{ "one", "ne", { "o", NULL }, 1 },
		{ "onene", "ne", { "o", NULL }, 1 },
		{ "+1-2++3--4++-+5-+-", "+-", { "1", "2", "3", "4", "5", NULL }, 5 },
		/* special cases */
		{ "", " ", { NULL }, 0 },
		{ " ", " ", { NULL }, 0 },
		{ "     ", " ", { NULL }, 0 },
		{ "oneoneone", "one", { NULL} , 0 },
		{ NULL, NULL, { NULL }, 0}
	};
	struct strsplit_test *t = tests;

	while (t->string) {
		size_t nelem;
		char **strv = strv_from_string(t->string, t->delim, &nelem);

		for (size_t idx = 0; idx < t->nresults; idx++)
			ck_assert_str_eq(t->results[idx], strv[idx]);

		ck_assert_uint_eq(nelem, t->nresults);

		/* When there are no elements validate return value is Null,
		   otherwise validate result array is Null terminated. */
		if(t->nresults == 0)
			ck_assert_ptr_eq(strv, NULL);
		else
			ck_assert_ptr_eq(strv[t->nresults], NULL);

		strv_free(strv);
		t++;
	}
}
END_TEST

START_TEST(double_array_from_string_test)
{
	struct double_array_from_string_test {
		const char *string;
		const char *delim;
		const double array[10];
		const size_t len;
		const bool result;
	} tests[] = {
		{ "1 2 3", " ", { 1, 2, 3 }, 3 },
		{ "1", " ", { 1 }, 1 },
		{ "1,2.5,", ",", { 1, 2.5 }, 2 },
		{ "1.0  2", " ", { 1, 2.0 }, 2 },
		{ " 1 2", " ", { 1, 2 }, 2 },
		{ " ; 1;2  3.5  ;;4.1", "; ", { 1, 2, 3.5, 4.1 }, 4 },
		/* special cases */
		{ "1 two", " ", { 0 }, 0 },
		{ "one two", " ", { 0 }, 0 },
		{ "one 2", " ", { 0 }, 0 },
		{ "", " ", { 0 }, 0 },
		{ " ", " ", { 0 }, 0 },
		{ "    ", " ", { 0 }, 0 },
		{ "", " ", { 0 }, 0 },
		{ "oneoneone", "one", { 0 }, 0 },
		{ NULL, NULL, { 0 }, 0 }
	};
	struct double_array_from_string_test *t = tests;

	while (t->string) {
		size_t len;
		double *array = double_array_from_string(t->string,
							 t->delim,
							 &len);
		ck_assert_int_eq(len, t->len);

		for (size_t idx = 0; idx < len; idx++)
			ck_assert_double_eq(array[idx], t->array[idx]);

		free(array);
		t++;
	}
}
END_TEST

START_TEST(strargv_test)
{
	struct argv_test {
		int argc;
		char *argv[10];
		int expected;
	} tests[] = {
		{ 0, {NULL}, 0 },
		{ 1, {"hello", "World"}, 1 },
		{ 2, {"hello", "World"}, 2 },
		{ 2, {"", " "}, 2 },
		{ 2, {"", NULL}, 0 },
		{ 2, {NULL, NULL}, 0 },
		{ 1, {NULL, NULL}, 0 },
		{ 3, {"hello", NULL, "World"}, 0 },
	};

	ARRAY_FOR_EACH(tests, t) {
		char **strv = strv_from_argv(t->argc, t->argv);

		if (t->expected == 0) {
			ck_assert(strv == NULL);
		} else {
			int count = 0;
			char **s = strv;
			while (*s) {
				ck_assert_str_eq(*s, t->argv[count]);
				count++;
				s++;
			}
			ck_assert_int_eq(t->expected, count);
			strv_free(strv);
		}
	}
}
END_TEST

START_TEST(kvsplit_double_test)
{
	struct kvsplit_dbl_test {
		const char *string;
		const char *psep;
		const char *kvsep;
		ssize_t nresults;
		struct {
			double a;
			double b;
		} results[32];
	} tests[] = {
		{ "1:2;3:4;5:6", ";", ":", 3, { {1, 2}, {3, 4}, {5, 6}}},
		{ "1.0x2.3 -3.2x4.5 8.090909x-6.00", " ", "x", 3, { {1.0, 2.3}, {-3.2, 4.5}, {8.090909, -6}}},

		{ "1:2", "x", ":", 1, {{1, 2}}},
		{ "1:2", ":", "x", -1, {}},
		{ "1:2", NULL, "x", -1, {}},
		{ "1:2", "", "x", -1, {}},
		{ "1:2", "x", NULL, -1, {}},
		{ "1:2", "x", "", -1, {}},
		{ "a:b", "x", ":", -1, {}},
		{ "", " ", "x", -1, {}},
		{ "1.2.3.4.5", ".", "", -1, {}},
		{ NULL }
	};
	struct kvsplit_dbl_test *t = tests;

	while (t->string) {
		struct key_value_double *result = NULL;
		ssize_t npairs;

		npairs = kv_double_from_string(t->string,
					       t->psep,
					       t->kvsep,
					       &result);
		ck_assert_int_eq(npairs, t->nresults);

		for (ssize_t i = 0; i < npairs; i++) {
			ck_assert_double_eq(t->results[i].a, result[i].key);
			ck_assert_double_eq(t->results[i].b, result[i].value);
		}


		free(result);
		t++;
	}
}
END_TEST

START_TEST(strjoin_test)
{
	struct strjoin_test {
		char *strv[10];
		const char *joiner;
		const char *result;
	} tests[] = {
		{ { "one", "two", "three", NULL }, " ", "one two three" },
		{ { "one", NULL }, "x", "one" },
		{ { "one", "two", NULL }, "x", "onextwo" },
		{ { "one", "two", NULL }, ",", "one,two" },
		{ { "one", "two", NULL }, ", ", "one, two" },
		{ { "one", "two", NULL }, "one", "oneonetwo" },
		{ { "one", "two", NULL }, NULL, NULL },
		{ { "", "", "", NULL }, " ", "  " },
		{ { "a", "b", "c", NULL }, "", "abc" },
		{ { "", "b", "c", NULL }, "x", "xbxc" },
		{ { "", "", "", NULL }, "", "" },
		{ { NULL }, NULL, NULL }
	};
	struct strjoin_test *t = tests;
	struct strjoin_test nulltest = { {NULL}, "x", NULL };

	while (t->strv[0]) {
		char *str;
		str = strv_join(t->strv, t->joiner);
		if (t->result == NULL)
			ck_assert(str == NULL);
		else
			ck_assert_str_eq(str, t->result);
		free(str);
		t++;
	}

	ck_assert(strv_join(nulltest.strv, "x") == NULL);
}
END_TEST

START_TEST(strstrip_test)
{
	struct strstrip_test {
		const char *string;
		const char *expected;
		const char *what;
	} tests[] = {
		{ "foo",		"foo",		"1234" },
		{ "\"bar\"",		"bar",		"\"" },
		{ "'bar'",		"bar",		"'" },
		{ "\"bar\"",		"\"bar\"",	"'" },
		{ "'bar'",		"'bar'",	"\"" },
		{ "\"bar\"",		"bar",		"\"" },
		{ "\"\"",		"",		"\"" },
		{ "\"foo\"bar\"",	"foo\"bar",	"\"" },
		{ "\"'foo\"bar\"",	"foo\"bar",	"\"'" },
		{ "abcfooabcbarbca",	"fooabcbar",	"abc" },
		{ "xxxxfoo",		"foo",		"x" },
		{ "fooyyyy",		"foo",		"y" },
		{ "xxxxfooyyyy",	"foo",		"xy" },
		{ "x xfooy y",		" xfooy ",	"xy" },
		{ " foo\n",		"foo",		" \n" },
		{ "",			"",		"abc" },
		{ "",			"",		"" },
		{ NULL , NULL, NULL }
	};
	struct strstrip_test *t = tests;

	while (t->string) {
		char *str;
		str = strstrip(t->string, t->what);
		ck_assert_str_eq(str, t->expected);
		free(str);
		t++;
	}
}
END_TEST

START_TEST(strendswith_test)
{
	struct strendswith_test {
		const char *string;
		const char *suffix;
		bool expected;
	} tests[] = {
		{ "foobar", "bar", true },
		{ "foobar", "foo", false },
		{ "foobar", "foobar", true },
		{ "foo", "foobar", false },
		{ "foobar", "", false },
		{ "", "", false },
		{ "", "foo", false },
		{ NULL, NULL, false },
	};

	for (struct strendswith_test *t = tests; t->string; t++) {
		ck_assert_int_eq(strendswith(t->string, t->suffix),
				 t->expected);
	}
}
END_TEST

START_TEST(strstartswith_test)
{
	struct strstartswith_test {
		const char *string;
		const char *suffix;
		bool expected;
	} tests[] = {
		{ "foobar", "foo", true },
		{ "foobar", "bar", false },
		{ "foobar", "foobar", true },
		{ "foo", "foobar", false },
		{ "foo", "", false },
		{ "", "", false },
		{ "foo", "", false },
		{ NULL, NULL, false },
	};

	for (struct strstartswith_test *t = tests; t->string; t++) {
		ck_assert_int_eq(strstartswith(t->string, t->suffix),
				 t->expected);
	}
}
END_TEST

START_TEST(strsanitize_test)
{
	struct strsanitize_test {
		const char *string;
		const char *expected;
	} tests[] = {
		{ "foobar", "foobar" },
		{ "", "" },
		{ "%", "%%" },
		{ "%%%%", "%%%%%%%%" },
		{ "x %s", "x %%s" },
		{ "x %", "x %%" },
		{ "%sx", "%%sx" },
		{ "%s%s", "%%s%%s" },
		{ NULL, NULL },
	};

	for (struct strsanitize_test *t = tests; t->string; t++) {
		char *sanitized = str_sanitize(t->string);
		ck_assert_str_eq(sanitized, t->expected);
		free(sanitized);
	}
}
END_TEST

START_TEST(list_test_insert)
{
	struct list_test {
		int val;
		struct list node;
	} tests[] = {
		{ .val  = 1 },
		{ .val  = 2 },
		{ .val  = 3 },
		{ .val  = 4 },
	};
	struct list_test *t;
	struct list head;
	int val;

	list_init(&head);

	ARRAY_FOR_EACH(tests, t) {
		list_insert(&head, &t->node);
	}

	val = 4;
	list_for_each(t, &head, node) {
		ck_assert_int_eq(t->val, val);
		val--;
	}

	ck_assert_int_eq(val, 0);
}
END_TEST

START_TEST(list_test_append)
{
	struct list_test {
		int val;
		struct list node;
	} tests[] = {
		{ .val  = 1 },
		{ .val  = 2 },
		{ .val  = 3 },
		{ .val  = 4 },
	};
	struct list_test *t;
	struct list head;
	int val;

	list_init(&head);

	ARRAY_FOR_EACH(tests, t) {
		list_append(&head, &t->node);
	}

	val = 1;
	list_for_each(t, &head, node) {
		ck_assert_int_eq(t->val, val);
		val++;
	}
	ck_assert_int_eq(val, 5);
}
END_TEST

START_TEST(list_test_foreach)
{
	struct list_test {
		int val;
		struct list node;
	} tests[] = {
		{ .val  = 1 },
		{ .val  = 2 },
		{ .val  = 3 },
		{ .val  = 4 },
	};
	struct list_test *t;
	struct list head;

	list_init(&head);

	ARRAY_FOR_EACH(tests, t) {
		list_append(&head, &t->node);
	}

	/* Make sure both loop macros are a single line statement */
	if (false)
		list_for_each(t, &head, node) {
			ck_abort_msg("We should not get here");
		}

	if (false)
		list_for_each_safe(t, &head, node) {
			ck_abort_msg("We should not get here");
		}
}
END_TEST

START_TEST(strverscmp_test)
{
	ck_assert_int_eq(libinput_strverscmp("", ""), 0);
	ck_assert_int_gt(libinput_strverscmp("0.0.1", ""), 0);
	ck_assert_int_lt(libinput_strverscmp("", "0.0.1"), 0);
	ck_assert_int_eq(libinput_strverscmp("0.0.1", "0.0.1"), 0);
	ck_assert_int_eq(libinput_strverscmp("0.0.1", "0.0.2"), -1);
	ck_assert_int_eq(libinput_strverscmp("0.0.2", "0.0.1"), 1);
	ck_assert_int_eq(libinput_strverscmp("0.0.1", "0.1.0"), -1);
	ck_assert_int_eq(libinput_strverscmp("0.1.0", "0.0.1"), 1);
}
END_TEST

START_TEST(streq_test)
{
	ck_assert(streq("", "") == true);
	ck_assert(streq(NULL, NULL) == true);
	ck_assert(streq("0.0.1", "") == false);
	ck_assert(streq("foo", NULL) == false);
	ck_assert(streq(NULL, "foo") == false);
	ck_assert(streq("0.0.1", "0.0.1") == true);
}
END_TEST

START_TEST(strneq_test)
{
	ck_assert(strneq("", "", 1) == true);
	ck_assert(strneq(NULL, NULL, 1) == true);
	ck_assert(strneq("0.0.1", "", 6) == false);
	ck_assert(strneq("foo", NULL, 5) == false);
	ck_assert(strneq(NULL, "foo", 5) == false);
	ck_assert(strneq("0.0.1", "0.0.1", 6) == true);
}
END_TEST

START_TEST(basename_test)
{
	struct test {
		const char *path;
		const char *expected;
	} tests[] = {
		{ "a", "a" },
		{ "foo.c", "foo.c" },
		{ "foo", "foo" },
		{ "/path/to/foo.h", "foo.h" },
		{ "../bar.foo", "bar.foo" },
		{ "./bar.foo.baz", "bar.foo.baz" },
		{ "./", NULL },
		{ "/", NULL },
		{ "/bar/", NULL },
		{ "/bar", "bar" },
		{ "", NULL },
	};

	ARRAY_FOR_EACH(tests, t) {
		const char *result = safe_basename(t->path);
		if (t->expected == NULL)
			ck_assert(result == NULL);
		else
			ck_assert_str_eq(result, t->expected);
	}
}
END_TEST
START_TEST(trunkname_test)
{
	struct test {
		const char *path;
		const char *expected;
	} tests[] = {
		{ "foo.c", "foo" },
		{ "/path/to/foo.h", "foo" },
		{ "/path/to/foo", "foo" },
		{ "../bar.foo", "bar" },
		{ "./bar.foo.baz", "bar.foo" },
		{ "./", "" },
		{ "/", "" },
		{ "/bar/", "" },
		{ "/bar", "bar" },
		{ "", "" },
	};

	ARRAY_FOR_EACH(tests, t) {
		char *result = trunkname(t->path);
		ck_assert_str_eq(result, t->expected);
		free(result);
	}
}
END_TEST

static Suite *
litest_utils_suite(void)
{
	TCase *tc;
	Suite *s;

	s = suite_create("litest:utils");
	tc = tcase_create("utils");

	tcase_add_test(tc, array_for_each);

	tcase_add_test(tc, bitfield_helpers);
	tcase_add_test(tc, matrix_helpers);
	tcase_add_test(tc, ratelimit_helpers);
	tcase_add_test(tc, dpi_parser);
	tcase_add_test(tc, wheel_click_parser);
	tcase_add_test(tc, wheel_click_count_parser);
	tcase_add_test(tc, dimension_prop_parser);
	tcase_add_test(tc, reliability_prop_parser);
	tcase_add_test(tc, calibration_prop_parser);
	tcase_add_test(tc, range_prop_parser);
	tcase_add_test(tc, boolean_prop_parser);
	tcase_add_test(tc, evcode_prop_parser);
	tcase_add_test(tc, input_prop_parser);
	tcase_add_test(tc, evdev_abs_parser);
	tcase_add_test(tc, safe_atoi_test);
	tcase_add_test(tc, safe_atoi_base_16_test);
	tcase_add_test(tc, safe_atoi_base_8_test);
	tcase_add_test(tc, safe_atou_test);
	tcase_add_test(tc, safe_atou_base_16_test);
	tcase_add_test(tc, safe_atou_base_8_test);
	tcase_add_test(tc, safe_atod_test);
	tcase_add_test(tc, strsplit_test);
	tcase_add_test(tc, double_array_from_string_test);
	tcase_add_test(tc, strargv_test);
	tcase_add_test(tc, kvsplit_double_test);
	tcase_add_test(tc, strjoin_test);
	tcase_add_test(tc, strstrip_test);
	tcase_add_test(tc, strendswith_test);
	tcase_add_test(tc, strstartswith_test);
	tcase_add_test(tc, strsanitize_test);
	tcase_add_test(tc, time_conversion);
	tcase_add_test(tc, human_time);

	tcase_add_test(tc, list_test_insert);
	tcase_add_test(tc, list_test_append);
	tcase_add_test(tc, list_test_foreach);
	tcase_add_test(tc, strverscmp_test);
	tcase_add_test(tc, streq_test);
	tcase_add_test(tc, strneq_test);
	tcase_add_test(tc, trunkname_test);
	tcase_add_test(tc, basename_test);

	suite_add_tcase(s, tc);

	return s;
}

int main(int argc, char **argv)
{
	int nfailed;
	Suite *s;
	SRunner *sr;

	s = litest_utils_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	nfailed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
