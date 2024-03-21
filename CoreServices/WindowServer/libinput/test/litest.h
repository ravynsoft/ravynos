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
#include "litest-config.h"

#ifndef LITEST_H
#define LITEST_H

#include <stdbool.h>
#include <stdarg.h>
#include <check.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <libinput.h>
#include <math.h>

#ifndef ck_assert_notnull
#define ck_assert_notnull(ptr) ck_assert_ptr_ne(ptr, NULL)
#endif

#include "check-double-macros.h"

#include "libinput-private-config.h"
#include "libinput-util.h"
#include "quirks.h"

struct test_device {
	const char *name;
	struct litest_test_device *device;
} __attribute__((aligned(16)));

#define TEST_DEVICE(name, ...) \
	static struct litest_test_device _device; \
	\
	static void _setup(void) { \
		struct litest_device *d = litest_create_device(_device.type); \
		litest_set_current_device(d); \
	} \
	\
	static const struct test_device _test_device \
		__attribute__ ((used)) \
		__attribute__ ((section ("test_section"))) = { \
		name, &_device \
	}; \
	static struct litest_test_device _device = { \
		.setup = _setup, \
		.shortname = name, \
		__VA_ARGS__ \
	};

struct test_collection {
	const char *name;
	void (*setup)(void);
} __attribute__((aligned(16)));

#define TEST_COLLECTION(name) \
	static void (name##_setup)(void); \
	static const struct test_collection _test_collection \
	__attribute__ ((used)) \
	__attribute__ ((section ("test_collection_section"))) = { \
		#name, name##_setup \
	}; \
	static void (name##_setup)(void)


/**
 * litest itself needs the user_data to store some test-suite-specific
 * information. Tests must not override this pointer, any data they need
 * they can hang off the private pointer in this struct.
 */
struct litest_user_data {
	void *private;
};

void
litest_fail_condition(const char *file,
		      int line,
		      const char *func,
		      const char *condition,
		      const char *message,
		      ...);
void
litest_fail_comparison_int(const char *file,
			   int line,
			   const char *func,
			   const char *operator,
			   int a,
			   int b,
			   const char *astr,
			   const char *bstr);
void
litest_fail_comparison_double(const char *file,
			      int line,
			      const char *func,
			      const char *operator,
			      double a,
			      double b,
			      const char *astr,
			      const char *bstr);
void
litest_fail_comparison_ptr(const char *file,
			   int line,
			   const char *func,
			   const char *comparison);

#define litest_assert(cond) \
	do { \
		if (!(cond)) \
			litest_fail_condition(__FILE__, __LINE__, __func__, \
					      #cond, NULL); \
	} while(0)

#define litest_assert_msg(cond, ...) \
	do { \
		if (!(cond)) \
			litest_fail_condition(__FILE__, __LINE__, __func__, \
					      #cond, __VA_ARGS__); \
	} while(0)

#define litest_abort_msg(...) \
	litest_fail_condition(__FILE__, __LINE__, __func__, \
			      "aborting", __VA_ARGS__); \

#define litest_assert_notnull(cond) \
	do { \
		if ((cond) == NULL) \
			litest_fail_condition(__FILE__, __LINE__, __func__, \
					      #cond, " expected to be not NULL\n"); \
	} while(0)

#define litest_assert_comparison_int_(a_, op_, b_) \
	do { \
		__typeof__(a_) _a = a_; \
		__typeof__(b_) _b = b_; \
		if (trunc(_a) != _a || trunc(_b) != _b) \
			litest_abort_msg("litest_assert_int_* used for non-integer value\n"); \
		if (!((_a) op_ (_b))) \
			litest_fail_comparison_int(__FILE__, __LINE__, __func__,\
						   #op_, _a, _b, \
						   #a_, #b_); \
	} while(0)

#define litest_assert_int_eq(a_, b_) \
	litest_assert_comparison_int_(a_, ==, b_)

#define litest_assert_int_ne(a_, b_) \
	litest_assert_comparison_int_(a_, !=, b_)

#define litest_assert_int_lt(a_, b_) \
	litest_assert_comparison_int_(a_, <, b_)

#define litest_assert_int_le(a_, b_) \
	litest_assert_comparison_int_(a_, <=, b_)

#define litest_assert_int_ge(a_, b_) \
	litest_assert_comparison_int_(a_, >=, b_)

#define litest_assert_int_gt(a_, b_) \
	litest_assert_comparison_int_(a_, >, b_)

#define litest_assert_comparison_ptr_(a_, op_, b_) \
	do { \
		__typeof__(a_) _a = a_; \
		__typeof__(b_) _b = b_; \
		if (!((_a) op_ (_b))) \
			litest_fail_comparison_ptr(__FILE__, __LINE__, __func__,\
						   #a_ " " #op_ " " #b_); \
	} while(0)

#define litest_assert_comparison_double_(a_, op_, b_) \
	do { \
		const double EPSILON = 1.0/256; \
		__typeof__(a_) _a = a_; \
		__typeof__(b_) _b = b_; \
		if (!((_a) op_ (_b)) && fabs((_a) - (_b)) > EPSILON)  \
			litest_fail_comparison_double(__FILE__, __LINE__, __func__,\
						      #op_, _a, _b, \
						      #a_, #b_); \
	} while(0)

#define litest_assert_ptr_eq(a_, b_) \
	litest_assert_comparison_ptr_(a_, ==, b_)

#define litest_assert_ptr_ne(a_, b_) \
	litest_assert_comparison_ptr_(a_, !=, b_)

#define litest_assert_ptr_null(a_) \
	litest_assert_comparison_ptr_(a_, ==, NULL)

#define litest_assert_ptr_notnull(a_) \
	litest_assert_comparison_ptr_(a_, !=, NULL)

#define litest_assert_double_eq(a_, b_)\
	litest_assert_comparison_double_((a_), ==, (b_))

#define litest_assert_double_ne(a_, b_)\
	litest_assert_comparison_double_((a_), !=, (b_))

#define litest_assert_double_lt(a_, b_)\
	litest_assert_comparison_double_((a_), <, (b_))

#define litest_assert_double_le(a_, b_)\
	litest_assert_comparison_double_((a_), <=, (b_))

#define litest_assert_double_gt(a_, b_)\
	litest_assert_comparison_double_((a_), >, (b_))

#define litest_assert_double_ge(a_, b_)\
	litest_assert_comparison_double_((a_), >=, (b_))

enum litest_device_type {
	LITEST_NO_DEVICE = -1,
	/* Touchpads and associated devices */
	LITEST_ACER_HAWAII_TOUCHPAD = -1000,
	LITEST_AIPTEK,
	LITEST_ALPS_3FG,
	LITEST_ALPS_DUALPOINT,
	LITEST_ALPS_SEMI_MT,
	LITEST_APPLETOUCH,
	LITEST_ATMEL_HOVER,
	LITEST_BCM5974,
	LITEST_ELANTECH_TOUCHPAD,
	LITEST_GENERIC_PRESSUREPAD,
	LITEST_MAGIC_TRACKPAD,
	LITEST_SYNAPTICS_CLICKPAD_X220,
	LITEST_SYNAPTICS_HOVER_SEMI_MT,
	LITEST_SYNAPTICS_I2C,
	LITEST_SYNAPTICS_PHANTOMCLICKS,
	LITEST_SYNAPTICS_PRESSUREPAD,
	LITEST_SYNAPTICS_RMI4,
	LITEST_SYNAPTICS_TOPBUTTONPAD,
	LITEST_SYNAPTICS_TOUCHPAD,
	LITEST_TOUCHPAD_PALMPRESSURE_ZERO,
	LITEST_WACOM_FINGER,

	/* Touchscreens */
	LITEST_CALIBRATED_TOUCHSCREEN,
	LITEST_GENERIC_MULTITOUCH_SCREEN,
	LITEST_GENERIC_SINGLETOUCH,
	LITEST_MS_SURFACE_COVER,
	LITEST_MULTITOUCH_FUZZ_SCREEN,
	LITEST_NEXUS4_TOUCH_SCREEN,
	LITEST_PROTOCOL_A_SCREEN,
	LITEST_TOUCHSCREEN_INVALID_RANGE,
	LITEST_TOUCHSCREEN_MT_TOOL_TYPE,
	LITEST_WACOM_TOUCH,

	/* Pointing devices and keyboards */
	LITEST_MOUSE,
	LITEST_KEYBOARD,
	LITEST_TRACKPOINT,
	LITEST_ABSINFO_OVERRIDE,
	LITEST_ACER_HAWAII_KEYBOARD,
	LITEST_ANKER_MOUSE_KBD,
	LITEST_APPLE_KEYBOARD,
	LITEST_CYBORG_RAT,
	LITEST_HP_WMI_HOTKEYS,
	LITEST_IGNORED_MOUSE,
	LITEST_KEYBOARD_ALL_CODES,
	LITEST_KEYBOARD_BLACKWIDOW,
	LITEST_KEYBOARD_BLADE_STEALTH,
	LITEST_KEYBOARD_BLADE_STEALTH_VIDEOSWITCH,
	LITEST_KEYBOARD_LOGITECH_MEDIA_KEYBOARD_ELITE,
	LITEST_KEYBOARD_QUIRKED,
	LITEST_LENOVO_SCROLLPOINT,
	LITEST_LOGITECH_TRACKBALL,
	LITEST_MAGICMOUSE,
	LITEST_MOUSE_FORMAT_STRING,
	LITEST_MOUSE_GLADIUS,
	LITEST_MOUSE_LOW_DPI,
	LITEST_MOUSE_ROCCAT,
	LITEST_MOUSE_WHEEL_CLICK_ANGLE,
	LITEST_MOUSE_WHEEL_CLICK_COUNT,
	LITEST_MOUSE_WHEEL_TILT,
	LITEST_MS_NANO_TRANSCEIVER_MOUSE,
	LITEST_SONY_VAIO_KEYS,
	LITEST_SYNAPTICS_TRACKPOINT_BUTTONS,
	LITEST_THINKPAD_EXTRABUTTONS,
	LITEST_VMWARE_VIRTMOUSE,
	LITEST_WHEEL_ONLY,
	LITEST_XEN_VIRTUAL_POINTER,

	/* Switches */
	LITEST_LID_SWITCH,
	LITEST_LID_SWITCH_SURFACE3,
	LITEST_TABLET_MODE_UNRELIABLE,

	/* Special devices */
	LITEST_DELL_CANVAS_TOTEM,
	LITEST_DELL_CANVAS_TOTEM_TOUCH,
	LITEST_GPIO_KEYS,
	LITEST_YUBIKEY,

	/* Tablets */
	LITEST_ELAN_TABLET,
	LITEST_HUION_TABLET,
	LITEST_QEMU_TABLET,
	LITEST_UCLOGIC_TABLET,
	LITEST_WACOM_BAMBOO,
	LITEST_WACOM_BAMBOO_2FG_FINGER,
	LITEST_WACOM_BAMBOO_2FG_PAD,
	LITEST_WACOM_BAMBOO_2FG_PEN,
	LITEST_WACOM_CALIBRATED_TABLET,
	LITEST_WACOM_CINTIQ,
	LITEST_WACOM_CINTIQ_13HDT_FINGER,
	LITEST_WACOM_CINTIQ_13HDT_PAD,
	LITEST_WACOM_CINTIQ_13HDT_PEN,
	LITEST_WACOM_CINTIQ_24HD,
	LITEST_WACOM_CINTIQ_24HDT_PAD,
	LITEST_WACOM_CINTIQ_PRO16_FINGER,
	LITEST_WACOM_CINTIQ_PRO16_PAD,
	LITEST_WACOM_CINTIQ_PRO16_PEN,
	LITEST_WACOM_EKR,
	LITEST_WACOM_HID4800_PEN,
	LITEST_WACOM_INTUOS,
	LITEST_WACOM_INTUOS3_PAD,
	LITEST_WACOM_INTUOS5_PAD,
	LITEST_WACOM_ISDV4,
	LITEST_WACOM_ISDV4_4200_PEN,
	LITEST_WACOM_ISDV4_524C_PEN,
	LITEST_WACOM_MOBILESTUDIO_PRO_16_PAD,
	LITEST_WALTOP,
};

#define LITEST_DEVICELESS	-2
#define LITEST_DISABLE_DEVICE	-1
#define LITEST_ANY		0
#define LITEST_TOUCHPAD		bit(0)
#define LITEST_CLICKPAD		bit(1)
#define LITEST_BUTTON		bit(2)
#define LITEST_KEYS		bit(3)
#define LITEST_RELATIVE		bit(4)
#define LITEST_WHEEL		bit(5)
#define LITEST_TOUCH		bit(6)
#define LITEST_SINGLE_TOUCH	bit(7)
#define LITEST_APPLE_CLICKPAD	bit(8)
#define LITEST_TOPBUTTONPAD	bit(9)
#define LITEST_SEMI_MT		bit(10)
#define LITEST_POINTINGSTICK	bit(11)
#define LITEST_FAKE_MT		bit(12)
#define LITEST_ABSOLUTE		bit(13)
#define LITEST_PROTOCOL_A	bit(14)
#define LITEST_HOVER		bit(15)
#define LITEST_ELLIPSE		bit(16)
#define LITEST_TABLET		bit(17)
#define LITEST_DISTANCE		bit(18)
#define LITEST_TOOL_SERIAL	bit(19)
#define LITEST_TILT		bit(20)
#define LITEST_TABLET_PAD	bit(21)
#define LITEST_RING		bit(22)
#define LITEST_STRIP		bit(23)
#define LITEST_TRACKBALL	bit(24)
#define LITEST_LEDS		bit(25)
#define LITEST_SWITCH		bit(26)
#define LITEST_IGNORED		bit(27)
#define LITEST_NO_DEBOUNCE	bit(28)
#define LITEST_TOOL_MOUSE	bit(29)
#define LITEST_DIRECT		bit(30)
#define LITEST_TOTEM		bit(31)
#define LITEST_FORCED_PROXOUT	bit(32)
#define LITEST_PRECALIBRATED	bit(33)

/* this is a semi-mt device, so we keep track of the touches that the tests
 * send and modify them so that the first touch is always slot 0 and sends
 * the top-left of the bounding box, the second is always slot 1 and sends
 * the bottom-right of the bounding box.
 * Lifting any of two fingers terminates slot 1
 */
struct litest_semi_mt {
	bool is_semi_mt;

	int tracking_id;
	/* The actual touches requested by the test for the two slots
	 * in the 0..100 range used by litest */
	struct {
		double x, y;
	} touches[2];
};

struct litest_device {
	enum litest_device_type which;
	struct libevdev *evdev;
	struct libevdev_uinput *uinput;
	struct libinput *libinput;
	struct quirks *quirks;
	bool owns_context;
	struct libinput_device *libinput_device;
	struct litest_device_interface *interface;

	int ntouches_down;
	int skip_ev_syn;
	struct litest_semi_mt semi_mt; /** only used for semi-mt device */

	void *private; /* device-specific data */
};

struct axis_replacement {
	int32_t evcode;
	double value;
};

/**
 * Same as litest_axis_set_value but allows for ranges outside 0..100%
 */
static inline void
litest_axis_set_value_unchecked(struct axis_replacement *axes, int code, double value)
{
	while (axes->evcode != -1) {
		if (axes->evcode == code) {
			axes->value = value;
			return;
		}
		axes++;
	}

	litest_abort_msg("Missing axis code %d\n", code);
}

/**
 * Takes a value in percent and sets the given axis to that code.
 */
static inline void
litest_axis_set_value(struct axis_replacement *axes, int code, double value)
{
	litest_assert_double_ge(value, 0.0);
	litest_assert_double_le(value, 100.0);

	litest_axis_set_value_unchecked(axes, code, value);
}

/* A loop range, resolves to:
   for (i = lower; i < upper; i++)
 */
struct range {
	int lower; /* inclusive */
	int upper; /* exclusive */
};

struct libinput *litest_create_context(void);
void litest_destroy_context(struct libinput *li);
void litest_disable_log_handler(struct libinput *libinput);
void litest_restore_log_handler(struct libinput *libinput);
void litest_set_log_handler_bug(struct libinput *libinput);

#define litest_add(func_, ...) \
	_litest_add(__FILE__, #func_, func_, __VA_ARGS__)
#define litest_add_ranged(func_, ...) \
	_litest_add_ranged(__FILE__, #func_, func_, __VA_ARGS__)
#define litest_add_for_device(func_, ...) \
	_litest_add_for_device(__FILE__, #func_, func_, __VA_ARGS__)
#define litest_add_ranged_for_device(func_, ...) \
	_litest_add_ranged_for_device(__FILE__, #func_, func_, __VA_ARGS__)
#define litest_add_no_device(func_) \
	_litest_add_no_device(__FILE__, #func_, func_)
#define litest_add_ranged_no_device(func_, ...) \
	_litest_add_ranged_no_device(__FILE__, #func_, func_, __VA_ARGS__)
#define litest_add_deviceless(func_) \
	_litest_add_deviceless(__FILE__, #func_, func_)

void
_litest_add(const char *name,
	    const char *funcname,
	    const void *func,
	    int64_t required_feature,
	    int64_t excluded_feature);
void
_litest_add_ranged(const char *name,
		   const char *funcname,
		   const void *func,
		   int64_t required,
		   int64_t excluded,
		   const struct range *range);
void
_litest_add_for_device(const char *name,
		       const char *funcname,
		       const void *func,
		       enum litest_device_type type);
void
_litest_add_ranged_for_device(const char *name,
			      const char *funcname,
			      const void *func,
			      enum litest_device_type type,
			      const struct range *range);
void
_litest_add_no_device(const char *name,
		      const char *funcname,
		      const void *func);
void
_litest_add_ranged_no_device(const char *name,
			     const char *funcname,
			     const void *func,
			     const struct range *range);
void
_litest_add_deviceless(const char *name,
		       const char *funcname,
		       const void *func);

struct litest_device *
litest_create_device(enum litest_device_type which);

struct litest_device *
litest_add_device(struct libinput *libinput,
		  enum litest_device_type which);
struct libevdev_uinput *
litest_create_uinput_device_from_description(const char *name,
					     const struct input_id *id,
					     const struct input_absinfo *abs,
					     const int *events);
struct litest_device *
litest_create(enum litest_device_type which,
	      const char *name_override,
	      struct input_id *id_override,
	      const struct input_absinfo *abs_override,
	      const int *events_override);

struct litest_device *
litest_create_device_with_overrides(enum litest_device_type which,
				    const char *name_override,
				    struct input_id *id_override,
				    const struct input_absinfo *abs_override,
				    const int *events_override);
struct litest_device *
litest_add_device_with_overrides(struct libinput *libinput,
				 enum litest_device_type which,
				 const char *name_override,
				 struct input_id *id_override,
				 const struct input_absinfo *abs_override,
				 const int *events_override);

struct litest_device *
litest_current_device(void);

void
litest_grab_device(struct litest_device *d);

void
litest_ungrab_device(struct litest_device *d);

void
litest_delete_device(struct litest_device *d);

void
litest_event(struct litest_device *t,
	     unsigned int type,
	     unsigned int code,
	     int value);
int
litest_auto_assign_value(struct litest_device *d,
			 const struct input_event *ev,
			 int slot, double x, double y,
			 struct axis_replacement *axes,
			 bool touching);
void
litest_touch_up(struct litest_device *d, unsigned int slot);

void
litest_touch_move(struct litest_device *d,
		  unsigned int slot,
		  double x,
		  double y);

void
litest_touch_move_extended(struct litest_device *d,
			   unsigned int slot,
			   double x,
			   double y,
			   struct axis_replacement *axes);

void
litest_touch_sequence(struct litest_device *d,
		      unsigned int slot,
		      double x1,
		      double y1,
		      double x2,
		      double y2,
		      int steps);

void
litest_touch_down(struct litest_device *d,
		  unsigned int slot,
		  double x,
		  double y);

void
litest_touch_down_extended(struct litest_device *d,
			   unsigned int slot,
			   double x,
			   double y,
			   struct axis_replacement *axes);

void
litest_touch_move_to(struct litest_device *d,
		     unsigned int slot,
		     double x_from, double y_from,
		     double x_to, double y_to,
		     int steps);

void
litest_touch_move_to_extended(struct litest_device *d,
			      unsigned int slot,
			      double x_from, double y_from,
			      double x_to, double y_to,
			      struct axis_replacement *axes,
			      int steps);

void
litest_touch_move_two_touches(struct litest_device *d,
			      double x0, double y0,
			      double x1, double y1,
			      double dx, double dy,
			      int steps);

void
litest_touch_move_three_touches(struct litest_device *d,
				double x0, double y0,
				double x1, double y1,
				double x2, double y2,
				double dx, double dy,
				int steps);

void
litest_tablet_set_tool_type(struct litest_device *d,
			    unsigned int code);

void
litest_tablet_proximity_in(struct litest_device *d,
			   double x, double y,
			   struct axis_replacement *axes);

void
litest_tablet_proximity_out(struct litest_device *d);

void
litest_tablet_tip_down(struct litest_device *d,
		       double x, double y,
		       struct axis_replacement *axes);

void
litest_tablet_tip_up(struct litest_device *d,
		     double x, double y,
		     struct axis_replacement *axes);

void
litest_tablet_motion(struct litest_device *d,
		     double x, double y,
		     struct axis_replacement *axes);

void
litest_pad_ring_start(struct litest_device *d, double value);

void
litest_pad_ring_change(struct litest_device *d, double value);

void
litest_pad_ring_end(struct litest_device *d);

void
litest_pad_strip_start(struct litest_device *d, double value);

void
litest_pad_strip_change(struct litest_device *d, double value);

void
litest_pad_strip_end(struct litest_device *d);

void
litest_hover_start(struct litest_device *d,
		   unsigned int slot,
		   double x,
		   double y);

void
litest_hover_end(struct litest_device *d, unsigned int slot);

void litest_hover_move(struct litest_device *d,
		       unsigned int slot,
		       double x,
		       double y);

void
litest_hover_move_to(struct litest_device *d,
		     unsigned int slot,
		     double x_from, double y_from,
		     double x_to, double y_to,
		     int steps);

void
litest_hover_move_two_touches(struct litest_device *d,
			      double x0, double y0,
			      double x1, double y1,
			      double dx, double dy,
			      int steps);

void
litest_button_click_debounced(struct litest_device *d,
			      struct libinput *li,
			      unsigned int button,
			      bool is_press);

void
litest_button_click(struct litest_device *d,
		    unsigned int button,
		    bool is_press);

void
litest_button_scroll(struct litest_device *d,
		     unsigned int button,
		     double dx, double dy);
void
litest_button_scroll_locked(struct litest_device *d,
			    unsigned int button,
			    double dx, double dy);

void
litest_keyboard_key(struct litest_device *d,
		    unsigned int key,
		    bool is_press);

void litest_switch_action(struct litest_device *d,
			  enum libinput_switch sw,
			  enum libinput_switch_state state);

void
litest_wait_for_event(struct libinput *li);

void
litest_wait_for_event_of_type(struct libinput *li, ...);

void
litest_drain_events(struct libinput *li);

void
litest_drain_events_of_type(struct libinput *li, ...);

void
litest_assert_event_type(struct libinput_event *event,
			 enum libinput_event_type want);

void
litest_assert_empty_queue(struct libinput *li);

void
litest_assert_touch_sequence(struct libinput *li);

void
litest_assert_touch_motion_frame(struct libinput *li);
void
litest_assert_touch_down_frame(struct libinput *li);
void
litest_assert_touch_up_frame(struct libinput *li);
void
litest_assert_touch_cancel(struct libinput *li);

struct libinput_event_pointer *
litest_is_button_event(struct libinput_event *event,
		       unsigned int button,
		       enum libinput_button_state state);

struct libinput_event_pointer *
litest_is_axis_event(struct libinput_event *event,
		     enum libinput_event_type axis_type,
		     enum libinput_pointer_axis axis,
		     enum libinput_pointer_axis_source source);

bool
litest_is_high_res_axis_event(struct libinput_event *event);

struct libinput_event_pointer *
litest_is_motion_event(struct libinput_event *event);

struct libinput_event_touch *
litest_is_touch_event(struct libinput_event *event,
		      enum libinput_event_type type);

struct libinput_event_keyboard *
litest_is_keyboard_event(struct libinput_event *event,
			 unsigned int key,
			 enum libinput_key_state state);

struct libinput_event_gesture *
litest_is_gesture_event(struct libinput_event *event,
			enum libinput_event_type type,
			int nfingers);

struct libinput_event_tablet_tool *
litest_is_tablet_event(struct libinput_event *event,
		       enum libinput_event_type type);

struct libinput_event_tablet_pad *
litest_is_pad_button_event(struct libinput_event *event,
			   unsigned int button,
			   enum libinput_button_state state);
struct libinput_event_tablet_pad *
litest_is_pad_ring_event(struct libinput_event *event,
			 unsigned int number,
			 enum libinput_tablet_pad_ring_axis_source source);
struct libinput_event_tablet_pad *
litest_is_pad_strip_event(struct libinput_event *event,
			  unsigned int number,
			  enum libinput_tablet_pad_strip_axis_source source);
struct libinput_event_tablet_pad *
litest_is_pad_key_event(struct libinput_event *event,
			unsigned int key,
			enum libinput_key_state state);

struct libinput_event_switch *
litest_is_switch_event(struct libinput_event *event,
		       enum libinput_switch sw,
		       enum libinput_switch_state state);

struct libinput_event_tablet_tool *
litest_is_proximity_event(struct libinput_event *event,
			  enum libinput_tablet_tool_proximity_state state);

double
litest_event_pointer_get_value(struct libinput_event_pointer *ptrev,
			       enum libinput_pointer_axis axis);

enum libinput_pointer_axis_source
litest_event_pointer_get_axis_source(struct libinput_event_pointer *event);

void
litest_assert_key_event(struct libinput *li, unsigned int key,
			enum libinput_key_state state);

void
litest_assert_button_event(struct libinput *li,
			   unsigned int button,
			   enum libinput_button_state state);

void
litest_assert_switch_event(struct libinput *li,
			   enum libinput_switch sw,
			   enum libinput_switch_state state);

void
litest_assert_scroll(struct libinput *li,
		     enum libinput_event_type axis_type,
		     enum libinput_pointer_axis axis,
		     int minimum_movement);

void
litest_assert_axis_end_sequence(struct libinput *li,
				enum libinput_event_type axis_type,
				enum libinput_pointer_axis axis,
				enum libinput_pointer_axis_source source);

void
litest_assert_only_typed_events(struct libinput *li,
				enum libinput_event_type type);

void
litest_assert_only_axis_events(struct libinput *li,
			       enum libinput_event_type axis_type);

void
litest_assert_no_typed_events(struct libinput *li,
			      enum libinput_event_type type);

void
litest_assert_tablet_button_event(struct libinput *li,
				  unsigned int button,
				  enum libinput_button_state state);

void
litest_assert_tablet_proximity_event(struct libinput *li,
				     enum libinput_tablet_tool_proximity_state state);

void
litest_assert_tablet_tip_event(struct libinput *li,
			       enum libinput_tablet_tool_tip_state state);

void
litest_assert_pad_button_event(struct libinput *li,
				    unsigned int button,
				    enum libinput_button_state state);
void
litest_assert_pad_key_event(struct libinput *li,
			    unsigned int key,
			    enum libinput_key_state state);

void
litest_assert_gesture_event(struct libinput *li,
			    enum libinput_event_type type,
			    int nfingers);

struct libevdev_uinput *
litest_create_uinput_device(const char *name,
			    struct input_id *id,
			    ...);

struct libevdev_uinput *
litest_create_uinput_abs_device(const char *name,
				struct input_id *id,
				const struct input_absinfo *abs,
				...);

void
litest_timeout_tap(void);

void
litest_timeout_tapndrag(void);

void
litest_timeout_debounce(void);

void
litest_timeout_softbuttons(void);

void
litest_timeout_buttonscroll(void);

void
litest_timeout_wheel_scroll(void);

void
litest_timeout_edgescroll(void);

void
litest_timeout_finger_switch(void);

void
litest_timeout_middlebutton(void);

void
litest_timeout_dwt_short(void);

void
litest_timeout_dwt_long(void);

void
litest_timeout_gesture(void);

void
litest_timeout_gesture_scroll(void);

void
litest_timeout_gesture_hold(void);

void
litest_timeout_gesture_quick_hold(void);

void
litest_timeout_trackpoint(void);

void
litest_timeout_tablet_proxout(void);

void
litest_timeout_touch_arbitration(void);

void
litest_timeout_hysteresis(void);

void
litest_push_event_frame(struct litest_device *dev);

void
litest_pop_event_frame(struct litest_device *dev);

void
litest_filter_event(struct litest_device *dev,
		    unsigned int type,
		    unsigned int code);

void
litest_unfilter_event(struct litest_device *dev,
		      unsigned int type,
		      unsigned int code);
void
litest_semi_mt_touch_down(struct litest_device *d,
			  struct litest_semi_mt *semi_mt,
			  unsigned int slot,
			  double x, double y);

void
litest_semi_mt_touch_move(struct litest_device *d,
			  struct litest_semi_mt *semi_mt,
			  unsigned int slot,
			  double x, double y);

void
litest_semi_mt_touch_up(struct litest_device *d,
			struct litest_semi_mt *semi_mt,
			unsigned int slot);

static inline void
litest_enable_tap(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_enabled(device,
							LIBINPUT_CONFIG_TAP_ENABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_disable_tap(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_enabled(device,
							LIBINPUT_CONFIG_TAP_DISABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_set_tap_map(struct libinput_device *device,
		   enum libinput_config_tap_button_map map)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_button_map(device, map);
	litest_assert_int_eq(status, expected);
}

static inline void
litest_enable_tap_drag(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_drag_enabled(device,
							     LIBINPUT_CONFIG_DRAG_ENABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_disable_tap_drag(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_drag_enabled(device,
							     LIBINPUT_CONFIG_DRAG_DISABLED);

	litest_assert_int_eq(status, expected);
}

static inline bool
litest_has_2fg_scroll(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;

	return !!(libinput_device_config_scroll_get_methods(device) &
		  LIBINPUT_CONFIG_SCROLL_2FG);
}

static inline void
litest_enable_2fg_scroll(struct litest_device *dev)
{
	enum libinput_config_status status, expected;
	struct libinput_device *device = dev->libinput_device;

	status = libinput_device_config_scroll_set_method(device,
					  LIBINPUT_CONFIG_SCROLL_2FG);

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	litest_assert_int_eq(status, expected);

	libinput_device_config_scroll_set_natural_scroll_enabled(device, 0);
}

static inline void
litest_enable_edge_scroll(struct litest_device *dev)
{
	enum libinput_config_status status, expected;
	struct libinput_device *device = dev->libinput_device;

	status = libinput_device_config_scroll_set_method(device,
					  LIBINPUT_CONFIG_SCROLL_EDGE);

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	litest_assert_int_eq(status, expected);

	libinput_device_config_scroll_set_natural_scroll_enabled(device, 0);
}

static inline bool
litest_has_clickfinger(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	uint32_t methods = libinput_device_config_click_get_methods(device);

	return methods & LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER;
}

static inline bool
litest_has_btnareas(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	uint32_t methods = libinput_device_config_click_get_methods(device);

	return methods & LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;
}

static inline void
litest_enable_clickfinger(struct litest_device *dev)
{
	enum libinput_config_status status, expected;
	struct libinput_device *device = dev->libinput_device;

	status = libinput_device_config_click_set_method(device,
				 LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);
	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	litest_assert_int_eq(status, expected);
}

static inline void
litest_enable_buttonareas(struct litest_device *dev)
{
	enum libinput_config_status status, expected;
	struct libinput_device *device = dev->libinput_device;

	status = libinput_device_config_click_set_method(device,
				 LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);
	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	litest_assert_int_eq(status, expected);
}

static inline void
litest_enable_drag_lock(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_ENABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_disable_drag_lock(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_enable_middleemu(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_middle_emulation_set_enabled(device,
								     LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_disable_middleemu(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_middle_emulation_set_enabled(device,
								     LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_sendevents_off(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_send_events_set_mode(device,
				    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	litest_assert_int_eq(status, expected);
}

static inline void
litest_sendevents_on(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_send_events_set_mode(device,
				    LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	litest_assert_int_eq(status, expected);
}

static inline void
litest_sendevents_ext_mouse(struct litest_device *dev)
{
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_send_events_set_mode(device,
				    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE);
	litest_assert_int_eq(status, expected);
}

static inline void
litest_enable_hold_gestures(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_gesture_set_hold_enabled(device,
								 LIBINPUT_CONFIG_HOLD_ENABLED);

	litest_assert_int_eq(status, expected);
}

static inline void
litest_disable_hold_gestures(struct libinput_device *device)
{
	enum libinput_config_status status, expected;

	expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_gesture_set_hold_enabled(device,
								 LIBINPUT_CONFIG_HOLD_DISABLED);

	litest_assert_int_eq(status, expected);
}

static inline bool
litest_touchpad_is_external(struct litest_device *dev)
{
	struct udev_device *udev_device;
	const char *prop;
	bool is_external;

	if (libinput_device_get_id_vendor(dev->libinput_device) == VENDOR_ID_WACOM)
		return true;

	udev_device = libinput_device_get_udev_device(dev->libinput_device);
	prop = udev_device_get_property_value(udev_device,
					      "ID_INPUT_TOUCHPAD_INTEGRATION");
	is_external = prop && streq(prop, "external");
	udev_device_unref(udev_device);

	return is_external;
}

static inline int
litest_send_file(int sock, int fd)
{
	char buf[40960];
	int n = read(fd, buf, 40960);
	litest_assert_int_gt(n, 0);
	return write(sock, buf, n);
}

static inline int
litest_slot_count(struct litest_device *dev)
{
	if (dev->which == LITEST_ALPS_3FG)
		return 2;

	return libevdev_get_num_slots(dev->evdev);
}

static inline bool
litest_has_palm_detect_size(struct litest_device *dev)
{
	double width, height;
	unsigned int vendor;
	unsigned int bustype;
	int rc;

	vendor = libinput_device_get_id_vendor(dev->libinput_device);
	bustype = libevdev_get_id_bustype(dev->evdev);
	if (vendor == VENDOR_ID_WACOM)
		return 0;
	if (bustype == BUS_BLUETOOTH)
		return 0;
	if (vendor == VENDOR_ID_APPLE)
		return 0;

	rc = libinput_device_get_size(dev->libinput_device, &width, &height);

	return rc == 0 && width >= 70;
}

#endif /* LITEST_H */
