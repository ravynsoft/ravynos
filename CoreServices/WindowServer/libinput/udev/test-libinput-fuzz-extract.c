/*
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

#include <check.h>

/* remove the main() from the included program so we can define our own */
#define main __disabled
int main(int argc, char **argv);
#include "libinput-fuzz-extract.c"
#undef main

START_TEST(test_parse_ev_abs)
{
	struct test {
		uint32_t which;
		const char *prop;
		int min, max, res, fuzz, flat;

	} tests[] = {
		{ .which = (MIN|MAX),
		  .prop = "1:2",
		  .min = 1, .max = 2 },
		{ .which = (MIN|MAX),
		  .prop = "1:2:",
		  .min = 1, .max = 2 },
		{ .which = (MIN|MAX|RES),
		  .prop = "10:20:30",
		  .min = 10, .max = 20, .res = 30 },
		{ .which = (RES),
		  .prop = "::100",
		  .res = 100 },
		{ .which = (MIN),
		  .prop = "10:",
		  .min = 10 },
		{ .which = (MAX|RES),
		  .prop = ":10:1001",
		  .max = 10, .res = 1001 },
		{ .which = (MIN|MAX|RES|FUZZ),
		  .prop = "1:2:3:4",
		  .min = 1, .max = 2, .res = 3, .fuzz = 4},
		{ .which = (MIN|MAX|RES|FUZZ|FLAT),
		  .prop = "1:2:3:4:5",
		  .min = 1, .max = 2, .res = 3, .fuzz = 4, .flat = 5},
		{ .which = (MIN|RES|FUZZ|FLAT),
		  .prop = "1::3:4:50",
		  .min = 1, .res = 3, .fuzz = 4, .flat = 50},
		{ .which = FUZZ|FLAT,
		  .prop = ":::5:60",
		  .fuzz = 5, .flat = 60},
		{ .which = FUZZ,
		  .prop = ":::5:",
		  .fuzz = 5 },
		{ .which = RES, .prop = "::12::",
		  .res = 12 },
		/* Malformed property but parsing this one makes us more
		 * future proof */
		{ .which = (RES|FUZZ|FLAT), .prop = "::12:1:2:3:4:5:6",
		  .res = 12, .fuzz = 1, .flat = 2 },
		{ .which = 0, .prop = ":::::" },
		{ .which = 0, .prop = ":" },
		{ .which = 0, .prop = "" },
		{ .which = 0, .prop = ":asb::::" },
		{ .which = 0, .prop = "foo" },
	};
	struct test *t;

	ARRAY_FOR_EACH(tests, t) {
		struct input_absinfo abs;
		uint32_t mask;

		mask = parse_ev_abs_prop(t->prop, &abs);
		ck_assert_int_eq(mask, t->which);

		if (t->which & MIN)
			ck_assert_int_eq(abs.minimum, t->min);
		if (t->which & MAX)
			ck_assert_int_eq(abs.maximum, t->max);
		if (t->which & RES)
			ck_assert_int_eq(abs.resolution, t->res);
		if (t->which & FUZZ)
			ck_assert_int_eq(abs.fuzz, t->fuzz);
		if (t->which & FLAT)
			ck_assert_int_eq(abs.flat, t->flat);
	}
}
END_TEST

int main(int argc, char **argv) {

	SRunner *sr = srunner_create(NULL);
	Suite *s = suite_create("fuzz-override");
	TCase *tc = tcase_create("parser");
	int nfailed;

	tcase_add_test(tc, test_parse_ev_abs);
	suite_add_tcase(s, tc);
	srunner_add_suite(sr, s);

	srunner_run_all(sr, CK_NORMAL);
	nfailed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return nfailed;
}
