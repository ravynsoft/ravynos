// SPDX-License-Identifier: MIT
/*
 * Copyright © 2019 Red Hat, Inc.
 */

#include "config.h"
#include "test-common.h"

START_TEST(test_info)
{
	struct libevdev *d = libevdev_new();

	libevdev_set_name(d, "some name");
	ck_assert_str_eq(libevdev_get_name(d), "some name");
	libevdev_set_phys(d, "physical");
	ck_assert_str_eq(libevdev_get_phys(d), "physical");
	libevdev_set_uniq(d, "very unique");
	ck_assert_str_eq(libevdev_get_uniq(d), "very unique");

	libevdev_set_id_bustype(d, 1);
	libevdev_set_id_vendor(d, 2);
	libevdev_set_id_product(d, 3);
	libevdev_set_id_version(d, 4);
	ck_assert_int_eq(libevdev_get_id_bustype(d), 1);
	ck_assert_int_eq(libevdev_get_id_vendor(d), 2);
	ck_assert_int_eq(libevdev_get_id_product(d), 3);
	ck_assert_int_eq(libevdev_get_id_version(d), 4);

	libevdev_free(d);
}
END_TEST

START_TEST(test_properties)
{
	for (unsigned prop = 0; prop < INPUT_PROP_CNT; prop++) {
		struct libevdev *d = libevdev_new();

		ck_assert(!libevdev_has_property(d, prop));
		libevdev_enable_property(d, prop);
		ck_assert(libevdev_has_property(d, prop));
		libevdev_free(d);
	}
}
END_TEST

START_TEST(test_bits)
{
	for (unsigned type = 1; type < EV_CNT; type++) {
		unsigned max = libevdev_event_type_get_max(type);

		if((int)max == -1)
			continue;

		for (unsigned code = 0; code <= max; code++) {
			struct libevdev *d = libevdev_new();
			const struct input_absinfo abs = {
				.minimum = 10,
				.maximum = 20,
				.fuzz = 30,
				.flat = 40,
				.resolution = 50,
			};
			const void *data = NULL;

			if (type == EV_ABS || type == EV_REP)
				data = &abs;

			ck_assert(!libevdev_has_event_code(d, type, code));
			libevdev_enable_event_code(d, type, code, data);
			ck_assert(libevdev_has_event_code(d, type, code));
			libevdev_free(d);
		}
	}
}
END_TEST

START_TEST(test_mt_slots_enable_disable)
{
	struct libevdev *d = libevdev_new();
	struct input_absinfo abs = {0};

	abs.maximum = 5;
	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);
	ck_assert(libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), 6);

	libevdev_disable_event_code(d, EV_ABS, ABS_MT_SLOT);
	ck_assert(!libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), -1);

	abs.maximum = 2;
	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);
	ck_assert(libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), 3);

	libevdev_free(d);
}
END_TEST

START_TEST(test_mt_slots_increase_decrease)
{
	struct libevdev *d = libevdev_new();
	struct input_absinfo abs = {0};

	abs.maximum = 5;
	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);
	ck_assert(libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), 6);

	abs.maximum = 2;
	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);
	ck_assert(libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), 3);

	abs.maximum = 6;
	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);
	ck_assert(libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), 7);

	abs.maximum = 10;
	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);
	ck_assert(libevdev_has_event_code(d, EV_ABS, ABS_MT_SLOT));
	ck_assert_int_eq(libevdev_get_num_slots(d), 11);

	libevdev_free(d);
}
END_TEST

START_TEST(test_mt_tracking_id)
{
	struct libevdev *d = libevdev_new();
	struct input_absinfo abs = { .maximum = 5 };

	libevdev_enable_event_code(d, EV_ABS, ABS_MT_SLOT, &abs);

	/* Not yet enabled, so 0. This is technically undefined */
	for (int slot = 0; slot < 5; slot++)
		ck_assert_int_eq(libevdev_get_slot_value(d, 0, ABS_MT_TRACKING_ID), 0);

	libevdev_enable_event_code(d, EV_ABS, ABS_MT_TRACKING_ID, &abs);

	for (int slot = 0; slot < 5; slot++)
		ck_assert_int_eq(libevdev_get_slot_value(d, 0, ABS_MT_TRACKING_ID), -1);

	libevdev_free(d);
}
END_TEST

TEST_SUITE(event_name_suite)
{
	Suite *s = suite_create("Context manipulation");

	add_test(s, test_info);
	add_test(s, test_properties);
	add_test(s, test_bits);
	add_test(s, test_mt_slots_enable_disable);
	add_test(s, test_mt_slots_increase_decrease);
	add_test(s, test_mt_tracking_id);

	return s;
}
