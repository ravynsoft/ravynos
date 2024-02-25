// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 Red Hat, Inc.
 */

#include "config.h"
#include <libevdev/libevdev.h>
#include <stdbool.h>
#include <stdio.h>

#include <check.h>

#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_

struct libevdev_test {
	const char *name;
	Suite* (*setup)(void);
	bool needs_root_privileges;
} __attribute__((aligned(16)));

#define _TEST_SUITE(name, root_privs) \
	static Suite* (name##_setup)(void); \
	static const struct libevdev_test _test \
	__attribute__((used)) \
	__attribute__((section ("test_section"))) = { \
		#name, name##_setup, root_privs \
	}; \
	static Suite* (name##_setup)(void)

#define TEST_SUITE(name) \
	_TEST_SUITE(name, false)

#define TEST_SUITE_ROOT_PRIVILEGES(name) \
	_TEST_SUITE(name, true)

#define TEST_DEVICE_NAME "libevdev test device"

#define add_test(suite, func) do { \
	TCase *tc = tcase_create(#func); \
	tcase_add_test(tc, func); \
	suite_add_tcase(suite, tc); \
} while(0)

#include "test-common-uinput.h"

#define assert_event(e_, t, c, v) \
do { \
	const struct input_event *e = (e_); \
	ck_assert_int_eq(e->type, (t)); \
	ck_assert_int_eq(e->code, (c)); \
	ck_assert_int_eq(e->value, (v)); \
} while(0)

void test_create_device(struct uinput_device **uidev,
			struct libevdev **dev,
			...);
void test_create_abs_device(struct uinput_device **uidev,
			    struct libevdev **dev,
			    int nabs,
			    const struct input_absinfo *abs,
			    ...);

void test_logfunc_abort_on_error(enum libevdev_log_priority priority,
				 void *data,
				 const char *file, int line,
				 const char *func,
				 const char *format, va_list args);
void test_logfunc_ignore_error(enum libevdev_log_priority priority,
			       void *data,
			       const char *file, int line,
			       const char *func,
			       const char *format, va_list args);

static inline void
print_event(const struct input_event *ev)
{
	if (ev->type == EV_SYN)
		printf("Event: time %ld.%06ld, ++++++++++++++++++++ %s +++++++++++++++\n",
			ev->input_event_sec,
			ev->input_event_usec,
			libevdev_event_type_get_name(ev->type));
	else
		printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
			ev->input_event_sec,
			ev->input_event_usec,
			ev->type,
			libevdev_event_type_get_name(ev->type),
			ev->code,
			libevdev_event_code_get_name(ev->type, ev->code),
			ev->value);
}
#endif /* _TEST_COMMON_H_ */
