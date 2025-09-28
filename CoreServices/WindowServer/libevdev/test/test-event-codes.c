// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 David Herrmann <dh.herrmann@gmail.com>
 */

#include "config.h"
#include "test-common.h"

START_TEST(test_type_names)
{
	ck_assert_int_eq(libevdev_event_type_from_name("EV_SYN"), EV_SYN);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_KEY"), EV_KEY);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_REL"), EV_REL);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_ABS"), EV_ABS);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_MSC"), EV_MSC);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_SND"), EV_SND);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_SW"), EV_SW);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_LED"), EV_LED);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_REP"), EV_REP);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_FF"), EV_FF);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_FF_STATUS"), EV_FF_STATUS);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_MAX"), EV_MAX);

	ck_assert_int_eq(libevdev_event_type_from_name_n("EV_SYNTAX", 6), EV_SYN);
	ck_assert_int_eq(libevdev_event_type_from_name_n("EV_REPTILE", 6), EV_REP);
}
END_TEST

START_TEST(test_type_names_invalid)
{
	ck_assert_int_eq(libevdev_event_type_from_name("EV_Syn"), -1);
	ck_assert_int_eq(libevdev_event_type_from_name("ev_SYN"), -1);
	ck_assert_int_eq(libevdev_event_type_from_name("SYN"), -1);
	ck_assert_int_eq(libevdev_event_type_from_name("EV_SYNTAX"), -1);

	ck_assert_int_eq(libevdev_event_type_from_name_n("EV_SYN", 5), -1);
	ck_assert_int_eq(libevdev_event_type_from_name_n("EV_REPTILE", 7), -1);
}
END_TEST

START_TEST(test_type_name_lookup)
{
	ck_assert_int_eq(libevdev_event_type_from_code_name("SYN_REPORT"), EV_SYN);
	ck_assert_int_eq(libevdev_event_type_from_code_name("KEY_A"), EV_KEY);
	ck_assert_int_eq(libevdev_event_type_from_code_name("REL_Z"), EV_REL);
	ck_assert_int_eq(libevdev_event_type_from_code_name("ABS_Z"), EV_ABS);
	ck_assert_int_eq(libevdev_event_type_from_code_name("MSC_SERIAL"), EV_MSC);
	ck_assert_int_eq(libevdev_event_type_from_code_name("SND_TONE"), EV_SND);
	ck_assert_int_eq(libevdev_event_type_from_code_name("SW_TABLET_MODE"), EV_SW);
	ck_assert_int_eq(libevdev_event_type_from_code_name("LED_CHARGING"), EV_LED);
	ck_assert_int_eq(libevdev_event_type_from_code_name("REP_PERIOD"), EV_REP);
	ck_assert_int_eq(libevdev_event_type_from_code_name("FF_SPRING"), EV_FF);
	ck_assert_int_eq(libevdev_event_type_from_code_name("FF_STATUS_STOPPED"), EV_FF_STATUS);

	ck_assert_int_eq(libevdev_event_type_from_code_name_n("KEY_1zzzzz", 5), EV_KEY);
	ck_assert_int_eq(libevdev_event_type_from_code_name_n("ABS_Zooom", 5), EV_ABS);

	ck_assert_int_eq(libevdev_event_type_from_code_name("KEY_MAX"), EV_KEY);
	ck_assert_int_eq(libevdev_event_type_from_code_name("REL_MAX"), EV_REL);
	ck_assert_int_eq(libevdev_event_type_from_code_name("ABS_MAX"), EV_ABS);
}
END_TEST

START_TEST(test_type_name_lookup_invalid)
{
	ck_assert_int_eq(libevdev_event_type_from_name("SYN_REPORTED"),  -1);
	ck_assert_int_eq(libevdev_event_type_from_name("syn_blah"), -1);
	ck_assert_int_eq(libevdev_event_type_from_name("SYN_"), -1);
	ck_assert_int_eq(libevdev_event_type_from_name("KEY_BANANA"), -1);

	ck_assert_int_eq(libevdev_event_type_from_name_n("KEY_BA", 6), -1);
	ck_assert_int_eq(libevdev_event_type_from_name_n("KEY_BLAH", 8), -1);
}
END_TEST

START_TEST(test_code_names)
{
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SYN, "SYN_REPORT"), SYN_REPORT);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_ABS, "ABS_X"), ABS_X);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "BTN_A"), BTN_A);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "KEY_A"), KEY_A);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_REL, "REL_X"), REL_X);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_MSC, "MSC_RAW"), MSC_RAW);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_LED, "LED_KANA"), LED_KANA);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SND, "SND_BELL"), SND_BELL);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_REP, "REP_DELAY"), REP_DELAY);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SYN, "SYN_DROPPED"), SYN_DROPPED);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "KEY_RESERVED"), KEY_RESERVED);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "BTN_0"), BTN_0);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "KEY_0"), KEY_0);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF, "FF_GAIN"), FF_GAIN);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF_STATUS, "FF_STATUS_MAX"), FF_STATUS_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SW, "SW_PEN_INSERTED"), SW_PEN_INSERTED);

	ck_assert_int_eq(libevdev_event_code_from_name_n(EV_ABS, "ABS_YXZ", 5), ABS_Y);
}
END_TEST

START_TEST(test_code_name_lookup)
{
	/* Same as test_code_names() but without the type */
	ck_assert_int_eq(libevdev_event_code_from_code_name("SYN_REPORT"), SYN_REPORT);
	ck_assert_int_eq(libevdev_event_code_from_code_name("ABS_X"), ABS_X);
	ck_assert_int_eq(libevdev_event_code_from_code_name("BTN_A"), BTN_A);
	ck_assert_int_eq(libevdev_event_code_from_code_name("KEY_A"), KEY_A);
	ck_assert_int_eq(libevdev_event_code_from_code_name("REL_X"), REL_X);
	ck_assert_int_eq(libevdev_event_code_from_code_name("MSC_RAW"), MSC_RAW);
	ck_assert_int_eq(libevdev_event_code_from_code_name("LED_KANA"), LED_KANA);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SND_BELL"), SND_BELL);
	ck_assert_int_eq(libevdev_event_code_from_code_name("REP_DELAY"), REP_DELAY);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SYN_DROPPED"), SYN_DROPPED);
	ck_assert_int_eq(libevdev_event_code_from_code_name("KEY_RESERVED"), KEY_RESERVED);
	ck_assert_int_eq(libevdev_event_code_from_code_name("BTN_0"), BTN_0);
	ck_assert_int_eq(libevdev_event_code_from_code_name("KEY_0"), KEY_0);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_GAIN"), FF_GAIN);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_STATUS_MAX"), FF_STATUS_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SW_PEN_INSERTED"), SW_PEN_INSERTED);

	ck_assert_int_eq(libevdev_event_code_from_code_name_n("ABS_YXZ", 5), ABS_Y);
}
END_TEST

START_TEST(test_code_names_invalid)
{
	ck_assert_int_eq(libevdev_event_code_from_name(EV_MAX, "MAX_FAKE"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_CNT, "CNT_FAKE"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_PWR, "PWR_SOMETHING"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_ABS, "EV_ABS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_ABS, "ABS_XY"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "BTN_GAMEPAD"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "BUS_PCI"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF_STATUS, "FF_STATUS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF_STATUS, "FF_STATUS_"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF, "FF_STATUS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF, "FF_STATUS_"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "ID_BUS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SND, "SND_CNT"), -1);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SW, "SW_CNT"), -1);

	ck_assert_int_eq(libevdev_event_code_from_name_n(EV_ABS, "ABS_X", 4), -1);
}
END_TEST

START_TEST(test_code_name_lookup_invalid)
{
	/* Same as test_code_names_invalid() but without the type */
	ck_assert_int_eq(libevdev_event_code_from_code_name("MAX_FAKE"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("CNT_FAKE"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("PWR_SOMETHING"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("EV_ABS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("ABS_XY"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("BTN_GAMEPAD"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("BUS_PCI"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_STATUS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_STATUS_"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_STATUS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_STATUS_"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("ID_BUS"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SND_CNT"), -1);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SW_CNT"), -1);

	ck_assert_int_eq(libevdev_event_code_from_code_name_n("ABS_X", 4), -1);
}
END_TEST

START_TEST(test_code_names_max)
{
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SYN, "SYN_MAX"), SYN_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_KEY, "KEY_MAX"), KEY_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_REL, "REL_MAX"), REL_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_ABS, "ABS_MAX"), ABS_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_MSC, "MSC_MAX"), MSC_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SW, "SW_MAX"), SW_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_LED, "LED_MAX"), LED_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_SND, "SND_MAX"), SND_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_REP, "REP_MAX"), REP_MAX);
	ck_assert_int_eq(libevdev_event_code_from_name(EV_FF, "FF_MAX"), FF_MAX);

	ck_assert_int_eq(libevdev_event_code_from_code_name("SYN_MAX"), SYN_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("KEY_MAX"), KEY_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("REL_MAX"), REL_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("ABS_MAX"), ABS_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("MSC_MAX"), MSC_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SW_MAX"), SW_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("LED_MAX"), LED_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("SND_MAX"), SND_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("REP_MAX"), REP_MAX);
	ck_assert_int_eq(libevdev_event_code_from_code_name("FF_MAX"), FF_MAX);
}
END_TEST

START_TEST(test_value_names)
{
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "MT_TOOL_PALM"), MT_TOOL_PALM);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "MT_TOOL_FINGER"), MT_TOOL_FINGER);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "MT_TOOL_PEN"), MT_TOOL_PEN);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "MT_TOOL_MAX"), MT_TOOL_MAX);
}
END_TEST

START_TEST(test_value_names_invalid)
{
	ck_assert_int_eq(libevdev_event_value_from_name(EV_SYN, REL_X, "MT_TOOL_PALM"), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_REL, REL_X, "MT_TOOL_PALM"), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_X, "MT_TOOL_PALM"), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "MT_TOOL_"), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "MT_TOOL_PALMA"), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, ""), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "EV_ABS"), -1);
	ck_assert_int_eq(libevdev_event_value_from_name(EV_ABS, ABS_MT_TOOL_TYPE, "ABS_X"), -1);
}
END_TEST

START_TEST(test_properties)
{
	struct prop {
		int val;
		const char *name;
	} lut[] = {
		{ INPUT_PROP_DIRECT, "INPUT_PROP_DIRECT" },
		{ INPUT_PROP_POINTER, "INPUT_PROP_POINTER" },
		{ INPUT_PROP_MAX, "INPUT_PROP_MAX" },
		{ -1, NULL}
	};
	struct prop *p = lut;
	while (p->val != -1) {
		ck_assert_int_eq(libevdev_property_from_name(p->name), p->val);
		p++;
	}
}
END_TEST

START_TEST(test_properties_invalid)
{
	ck_assert_int_eq(libevdev_property_from_name("EV_ABS"), -1);
	ck_assert_int_eq(libevdev_property_from_name("INPUT_PROP"), -1);
	ck_assert_int_eq(libevdev_property_from_name("INPUT_PROP_"), -1);
	ck_assert_int_eq(libevdev_property_from_name("INPUT_PROP_FOO"), -1);

	ck_assert_int_eq(libevdev_property_from_name_n("INPUT_PROP_POINTER", 11), -1);
	ck_assert_int_eq(libevdev_property_from_name_n("INPUT_PROP_POINTER",
						strlen("INPUT_PROP_POINTER") - 1), -1);
}
END_TEST

TEST_SUITE(event_code_suite)
{
	Suite *s = suite_create("Event codes");

	add_test(s, test_type_names);
	add_test(s, test_type_names_invalid);
	add_test(s, test_type_name_lookup);
	add_test(s, test_type_name_lookup_invalid);

	add_test(s, test_code_names);
	add_test(s, test_code_name_lookup);
	add_test(s, test_code_names_invalid);
	add_test(s, test_code_name_lookup_invalid);
	add_test(s, test_code_names_max);

	add_test(s, test_value_names);
	add_test(s, test_value_names_invalid);

	add_test(s, test_properties);
	add_test(s, test_properties_invalid);

	return s;
}
