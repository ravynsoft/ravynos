/*
 * Copyright © 2018 Red Hat, Inc.
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

#include <errno.h>
#include <sys/epoll.h>
#include <inttypes.h>
#include <linux/input.h>
#include <libevdev/libevdev.h>
#include <libudev.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include "libinput-versionsort.h"
#include "libinput-version.h"
#include "libinput-git-version.h"
#include "shared.h"
#include "builddir.h"
#include "util-bits.h"
#include "util-list.h"
#include "util-time.h"
#include "util-input-event.h"
#include "util-macros.h"

static const int FILE_VERSION_NUMBER = 1;

/* Indentation levels for the various data nodes */
enum indent {
	I_NONE = 0,
	I_TOPLEVEL = 0,
	I_LIBINPUT = 2,			/* nodes inside libinput: */
	I_SYSTEM = 2,			/* nodes inside system:   */
	I_DEVICE = 2,			/* nodes inside devices:  */
	I_EVDEV = 4,			/* nodes inside evdev:    */
	I_EVDEV_DATA = 6,		/* nodes below evdev:	  */
	I_UDEV = 4,			/* nodes inside udev:     */
	I_UDEV_DATA = 6,		/* nodes below udev:      */
	I_QUIRKS = 4,			/* nodes inside quirks:	  */
	I_LIBINPUTDEV = 4,		/* nodes inside libinput: (the
					   device description */
	I_EVENTTYPE = 4,		/* event type (evdev:, libinput:,
					   hidraw:) */
	I_EVENT = 6,			/* event data */
};

struct record_device {
	struct record_context *ctx;
	struct list link;
	char *devnode;		/* device node of the source device */
	struct libevdev *evdev;
	struct libevdev *evdev_prev; /* previous value, used for EV_ABS
					deltas */
	struct libinput_device *device;
	struct list hidraw_devices;

	struct {
		bool is_touch_device;
		uint16_t slot_state;
		uint16_t last_slot_state;
	} touch;

	FILE *fp;
};

struct hidraw {
	struct list link;
	struct record_device *device;
	int fd;
	char *name;
};

struct record_context {
	int timeout;
	bool show_keycodes;

	uint64_t offset;

	/* The first device to be added */
	struct record_device *first_device;

	struct list devices;
	int ndevices;

	struct {
		char *name;		 /* file name given on cmdline */
		char *name_with_suffix;  /* full file name with suffix */
	} output_file;

	struct libinput *libinput;

	int epoll_fd;
	struct list sources;

	struct {
		bool had_events_since_last_time;
		bool skipped_timer_print;
	} timestamps;

	bool had_events;
	bool stop;
};

#define resize(array_, sz_) \
{ \
	size_t new_size = (sz_) + 1000; \
	void *tmp = realloc((array_), new_size * sizeof(*(array_))); \
	assert(tmp); \
	(array_)  = tmp; \
	(sz_) = new_size; \
}

typedef void (*source_dispatch_t)(struct record_context *ctx,
				  int fd,
				  void *user_data);

struct source {
	source_dispatch_t dispatch;
	void *user_data;
	int fd;
	struct list link;
};

static bool
obfuscate_keycode(struct input_event *ev)
{
	switch (ev->type) {
	case EV_KEY:
		switch (ev->code) {
		case KEY_ESC:
		case KEY_TAB:
		case KEY_ENTER:
		case KEY_LEFTCTRL:
			break;
		default:
			if ((ev->code > KEY_ESC && ev->code < KEY_CAPSLOCK) ||
			    (ev->code >= KEY_KP7 && ev->code <= KEY_KPDOT)) {
				ev->code = KEY_A;
				return true;
			}
		}
		break;
	case EV_MSC:
		if (ev->code == MSC_SCAN) {
			ev->value = 30; /* KEY_A scancode */
			return true;
		}
		break;
	}

	return false;
}

/**
 * Indented dprintf, indentation is in the context
 */
LIBINPUT_ATTRIBUTE_PRINTF(3, 4)
static void
iprintf(FILE *fp,
	enum indent indent,
	const char *format, ...)
{
	va_list args;
	char fmt[1024];
	static const char space[] = "                                     ";
	static const size_t len = sizeof(space);
	int rc;

	assert(indent < len);
	assert(strlen(format) >= 1);

	/* Special case: if we're printing a new list item, we want less
	 * indentation because the '- ' takes up one level of indentation
	 *
	 * This is only needed because I don't want to deal with open/close
	 * lists statements.
	 */
	if (format[0] == '-' && indent > 0)
		indent -= 2;

	snprintf(fmt, sizeof(fmt), "%s%s", &space[len - indent - 1], format);
	va_start(args, format);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	rc = vfprintf(fp, fmt, args);
#pragma GCC diagnostic pop
	va_end(args);

	assert(rc != -1 && (unsigned int)rc > indent);
}

static uint64_t
time_offset(struct record_context *ctx, uint64_t time)
{
	return ctx->offset ? time - ctx->offset : 0;
}

static void
print_evdev_event(struct record_device *dev,
		  struct input_event *ev)
{
	const char *tname, *cname;
	bool was_modified = false;
	char desc[1024];
	uint64_t time = input_event_time(ev) - dev->ctx->offset;

	input_event_set_time(ev, time);

	/* Don't leak passwords unless the user wants to */
	if (!dev->ctx->show_keycodes)
		was_modified = obfuscate_keycode(ev);

	tname = libevdev_event_type_get_name(ev->type);
	cname = libevdev_event_code_get_name(ev->type, ev->code);

	if (ev->type == EV_SYN && ev->code == SYN_MT_REPORT) {
		snprintf(desc,
			 sizeof(desc),
			 "++++++++++++ %s (%d) ++++++++++",
			 cname,
			 ev->value);
	} else if (ev->type == EV_SYN) {
		static unsigned long last_ms = 0;
		unsigned long time, dt;

		time = us2ms(input_event_time(ev));
		dt = time - last_ms;
		last_ms = time;

		snprintf(desc,
			 sizeof(desc),
			"------------ %s (%d) ---------- %+ldms",
			cname,
			ev->value,
			dt);
	} else if (ev->type == EV_ABS) {
		int oldval = 0;
		enum { DELTA, SLOT_DELTA, NO_DELTA } want = DELTA;
		int delta = 0;

		/* We want to print deltas for abs axes but there are a few
		 * that we don't care about for actual deltas because
		 * they're meaningless.
		 *
		 * Also, any slotted axis needs to be printed per slot
		 */
		switch (ev->code) {
		case ABS_MT_SLOT:
			libevdev_set_event_value(dev->evdev_prev,
						 ev->type,
						 ev->code,
						 ev->value);
			want = NO_DELTA;
			break;
		case ABS_MT_TRACKING_ID:
		case ABS_MT_BLOB_ID:
			want = NO_DELTA;
			break;
		case ABS_MT_TOUCH_MAJOR ... ABS_MT_POSITION_Y:
		case ABS_MT_PRESSURE ... ABS_MT_TOOL_Y:
			if (libevdev_get_num_slots(dev->evdev_prev) > 0)
				want = SLOT_DELTA;
			break;
		default:
			break;
		}

		switch (want) {
		case DELTA:
			oldval = libevdev_get_event_value(dev->evdev_prev,
							  ev->type,
							  ev->code);
			libevdev_set_event_value(dev->evdev_prev,
						 ev->type,
						 ev->code,
						 ev->value);
			break;
		case SLOT_DELTA: {
			int slot = libevdev_get_current_slot(dev->evdev_prev);
			oldval = libevdev_get_slot_value(dev->evdev_prev,
							 slot,
							 ev->code);
			libevdev_set_slot_value(dev->evdev_prev,
						slot,
						ev->code,
						ev->value);
			break;
		}
		case NO_DELTA:
			break;

		}

		delta = ev->value - oldval;

		switch (want) {
		case DELTA:
		case SLOT_DELTA:
			snprintf(desc,
				 sizeof(desc),
				 "%s / %-20s %6d (%+d)",
				 tname,
				 cname,
				 ev->value,
				 delta);
			break;
		case NO_DELTA:
			snprintf(desc,
				 sizeof(desc),
				 "%s / %-20s %6d",
				 tname,
				 cname,
				 ev->value);
			break;
		}
	} else {
		snprintf(desc,
			 sizeof(desc),
			 "%s / %-20s %6d%s",
			 tname,
			 cname,
			 ev->value,
			 was_modified ? " (obfuscated)" : "");
	}

	iprintf(dev->fp,
		I_EVENT,
		"- [%3lu, %6u, %3d, %3d, %7d] # %s\n",
		ev->input_event_sec,
		(unsigned int)ev->input_event_usec,
		ev->type,
		ev->code,
		ev->value,
		desc);
}

static bool
handle_evdev_frame(struct record_device *d)
{
	struct libevdev *evdev = d->evdev;
	struct input_event e;

	if (libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &e) !=
		LIBEVDEV_READ_STATUS_SUCCESS)
		return false;

	iprintf(d->fp, I_EVENTTYPE, "- evdev:\n");
	do {

		if (d->ctx->offset == 0) {
			uint64_t time = input_event_time(&e);
			d->ctx->offset = time;
		}

		print_evdev_event(d, &e);

		if (d->touch.is_touch_device &&
		    e.type == EV_ABS &&
		    e.code == ABS_MT_TRACKING_ID) {
			unsigned int slot = libevdev_get_current_slot(evdev);
			assert(slot < sizeof(d->touch.slot_state) * 8);

			if (e.value != -1)
				d->touch.slot_state |= bit(slot);
			else
				d->touch.slot_state &= ~bit(slot);
		}

		if (e.type == EV_SYN && e.code == SYN_REPORT)
			break;
	} while (libevdev_next_event(evdev,
				     LIBEVDEV_READ_FLAG_NORMAL,
				     &e) == LIBEVDEV_READ_STATUS_SUCCESS);

	if (d->touch.slot_state != d->touch.last_slot_state) {
		d->touch.last_slot_state = d->touch.slot_state;
		if (d->touch.slot_state == 0) {
			iprintf(d->fp,
				I_EVENT,
				 "                                 # Touch device in neutral state\n");
		}
	}

	return true;
}

static void
print_device_notify(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_device *d = libinput_event_get_device(e);
	struct libinput_seat *seat = libinput_device_get_seat(d);
	const char *type = NULL;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_DEVICE_ADDED:
		type = "DEVICE_ADDED";
		break;
	case LIBINPUT_EVENT_DEVICE_REMOVED:
		type = "DEVICE_REMOVED";
		break;
	default:
		abort();
	}

	iprintf(dev->fp,
		I_EVENT,
		"- {type: %s, seat: %5s, logical_seat: %7s}\n",
		type,
		libinput_seat_get_physical_name(seat),
		libinput_seat_get_logical_name(seat));
}

static void
print_key_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_keyboard *k = libinput_event_get_keyboard_event(e);
	enum libinput_key_state state;
	uint32_t key;
	uint64_t time;
	const char *type;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_KEYBOARD_KEY:
		type = "KEYBOARD_KEY";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_keyboard_get_time_usec(k));
	state = libinput_event_keyboard_get_key_state(k);

	key = libinput_event_keyboard_get_key(k);
	if (!dev->ctx->show_keycodes &&
	    (key >= KEY_ESC && key < KEY_ZENKAKUHANKAKU))
		key = -1;

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, key: %d, state: %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		key,
		state == LIBINPUT_KEY_STATE_PRESSED ? "pressed" : "released");
}

static void
print_motion_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(e);
	double x = libinput_event_pointer_get_dx(p),
	       y = libinput_event_pointer_get_dy(p);
	double uax = libinput_event_pointer_get_dx_unaccelerated(p),
	       uay = libinput_event_pointer_get_dy_unaccelerated(p);
	uint64_t time;
	const char *type;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_POINTER_MOTION:
		type = "POINTER_MOTION";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_pointer_get_time_usec(p));
	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, delta: [%6.2f, %6.2f], unaccel: [%6.2f, %6.2f]}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		x, y,
		uax, uay);
}

static void
print_absmotion_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(e);
	double x = libinput_event_pointer_get_absolute_x(p),
	       y = libinput_event_pointer_get_absolute_y(p);
	double tx = libinput_event_pointer_get_absolute_x_transformed(p, 100),
	       ty = libinput_event_pointer_get_absolute_y_transformed(p, 100);
	uint64_t time;
	const char *type;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
		type = "POINTER_MOTION_ABSOLUTE";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_pointer_get_time_usec(p));

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, point: [%6.2f, %6.2f], transformed: [%6.2f, %6.2f]}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		x, y,
		tx, ty);
}

static void
print_pointer_button_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(e);
	enum libinput_button_state state;
	int button;
	uint64_t time;
	const char *type;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_POINTER_BUTTON:
		type = "POINTER_BUTTON";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_pointer_get_time_usec(p));
	button = libinput_event_pointer_get_button(p);
	state = libinput_event_pointer_get_button_state(p);

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, button: %d, state: %s, seat_count: %u}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		button,
		state == LIBINPUT_BUTTON_STATE_PRESSED ? "pressed" : "released",
		libinput_event_pointer_get_seat_button_count(p));
}

static void
print_pointer_axis_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(e);
	uint64_t time;
	const char *type, *source;
	double h = 0, v = 0;
	int hd = 0, vd = 0;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_POINTER_AXIS:
		type = "POINTER_AXIS";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_pointer_get_time_usec(p));
	if (libinput_event_pointer_has_axis(p,
				LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
		h = libinput_event_pointer_get_axis_value(p,
				LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
		hd = libinput_event_pointer_get_axis_value_discrete(p,
				LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
	}
	if (libinput_event_pointer_has_axis(p,
				LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
		v = libinput_event_pointer_get_axis_value(p,
				LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
		vd = libinput_event_pointer_get_axis_value_discrete(p,
				LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
	}
	switch(libinput_event_pointer_get_axis_source(p)) {
	case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL: source = "wheel"; break;
	case LIBINPUT_POINTER_AXIS_SOURCE_FINGER: source = "finger"; break;
	case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS: source = "continuous"; break;
	case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL_TILT: source = "wheel-tilt"; break;
	default:
		source = "unknown";
		break;
	}

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, axes: [%2.2f, %2.2f], discrete: [%d, %d], source: %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		h, v,
		hd, vd,
		source);
}

static void
print_touch_event(struct record_device *dev, struct libinput_event *e)
{
	enum libinput_event_type etype = libinput_event_get_type(e);
	struct libinput_event_touch *t = libinput_event_get_touch_event(e);
	const char *type;
	double x, y;
	double tx, ty;
	uint64_t time;
	int32_t slot, seat_slot;

	switch(etype) {
	case LIBINPUT_EVENT_TOUCH_DOWN:
		type = "TOUCH_DOWN";
		break;
	case LIBINPUT_EVENT_TOUCH_UP:
		type = "TOUCH_UP";
		break;
	case LIBINPUT_EVENT_TOUCH_MOTION:
		type = "TOUCH_MOTION";
		break;
	case LIBINPUT_EVENT_TOUCH_CANCEL:
		type = "TOUCH_CANCEL";
		break;
	case LIBINPUT_EVENT_TOUCH_FRAME:
		type = "TOUCH_FRAME";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_touch_get_time_usec(t));

	if (etype != LIBINPUT_EVENT_TOUCH_FRAME) {
		slot = libinput_event_touch_get_slot(t);
		seat_slot = libinput_event_touch_get_seat_slot(t);
	}

	switch (etype) {
	case LIBINPUT_EVENT_TOUCH_FRAME:
		iprintf(dev->fp,
			I_EVENT,
			"- {time: %ld.%06ld, type: %s}\n",
			(long)(time / (int)1e6),
			(long)(time % (int)1e6),
			type);
		break;
	case LIBINPUT_EVENT_TOUCH_DOWN:
	case LIBINPUT_EVENT_TOUCH_MOTION:
		x = libinput_event_touch_get_x(t);
		y = libinput_event_touch_get_y(t);
		tx = libinput_event_touch_get_x_transformed(t, 100);
		ty = libinput_event_touch_get_y_transformed(t, 100);
		iprintf(dev->fp,
			I_EVENT,
			"- {time: %ld.%06ld, type: %s, slot: %d, seat_slot: %d, "
			"point: [%6.2f, %6.2f], transformed: [%6.2f, %6.2f]}\n",
			(long)(time / (int)1e6),
			(long)(time % (int)1e6),
			type,
			slot,
			seat_slot,
			x, y,
			tx, ty);
		break;
	case LIBINPUT_EVENT_TOUCH_UP:
	case LIBINPUT_EVENT_TOUCH_CANCEL:
		iprintf(dev->fp,
			I_EVENT,
			"- {time: %ld.%06ld, type: %s, slot: %d, seat_slot: %d}\n",
			(long)(time / (int)1e6),
			(long)(time % (int)1e6),
			type,
			slot,
			seat_slot);
		break;
	default:
		abort();
	}
}

static void
print_gesture_event(struct record_device *dev, struct libinput_event *e)
{
	enum libinput_event_type etype = libinput_event_get_type(e);
	struct libinput_event_gesture *g = libinput_event_get_gesture_event(e);
	const char *type;
	uint64_t time;

	switch(etype) {
	case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
		type = "GESTURE_PINCH_BEGIN";
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
		type = "GESTURE_PINCH_UPDATE";
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_END:
		type = "GESTURE_PINCH_END";
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
		type = "GESTURE_SWIPE_BEGIN";
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
		type = "GESTURE_SWIPE_UPDATE";
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_END:
		type = "GESTURE_SWIPE_END";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_gesture_get_time_usec(g));

	switch (etype) {
	case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
	case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
	case LIBINPUT_EVENT_GESTURE_PINCH_END:
		iprintf(dev->fp,
			I_EVENT,
			"- {time: %ld.%06ld, type: %s, nfingers: %d, "
			"delta: [%6.2f, %6.2f], unaccel: [%6.2f, %6.2f], "
			"angle_delta: %6.2f, scale: %6.2f}\n",
			(long)(time / (int)1e6),
			(long)(time % (int)1e6),
			type,
			libinput_event_gesture_get_finger_count(g),
			libinput_event_gesture_get_dx(g),
			libinput_event_gesture_get_dy(g),
			libinput_event_gesture_get_dx_unaccelerated(g),
			libinput_event_gesture_get_dy_unaccelerated(g),
			libinput_event_gesture_get_angle_delta(g),
			libinput_event_gesture_get_scale(g)
		       );
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
	case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
	case LIBINPUT_EVENT_GESTURE_SWIPE_END:
		iprintf(dev->fp,
			I_EVENT,
			"- {time: %ld.%06ld, type: %s, nfingers: %d, "
			"delta: [%6.2f, %6.2f], unaccel: [%6.2f, %6.2f]}\n",
			(long)(time / (int)1e6),
			(long)(time % (int)1e6),
			type,
			libinput_event_gesture_get_finger_count(g),
			libinput_event_gesture_get_dx(g),
			libinput_event_gesture_get_dy(g),
			libinput_event_gesture_get_dx_unaccelerated(g),
			libinput_event_gesture_get_dy_unaccelerated(g)
		       );
		break;
	default:
		abort();
	}
}

static char *
buffer_tablet_axes(struct libinput_event_tablet_tool *t)
{
	const int MAX_AXES = 10;
	struct libinput_tablet_tool *tool;
	char *s = NULL;
	int idx = 0;
	int len;
	double x, y;
	char **strv;

	tool = libinput_event_tablet_tool_get_tool(t);

	strv = zalloc(MAX_AXES * sizeof *strv);

	x = libinput_event_tablet_tool_get_x(t);
	y = libinput_event_tablet_tool_get_y(t);
	len = xasprintf(&strv[idx++], "point: [%.2f, %.2f]", x, y);
	if (len <= 0)
		goto out;

	if (libinput_tablet_tool_has_tilt(tool)) {
		x = libinput_event_tablet_tool_get_tilt_x(t);
		y = libinput_event_tablet_tool_get_tilt_y(t);
		len = xasprintf(&strv[idx++], "tilt: [%.2f, %.2f]", x, y);
		if (len <= 0)
			goto out;
	}

	if (libinput_tablet_tool_has_distance(tool) ||
	    libinput_tablet_tool_has_pressure(tool)) {
		double dist, pressure;

		dist = libinput_event_tablet_tool_get_distance(t);
		pressure = libinput_event_tablet_tool_get_pressure(t);
		if (dist)
			len = xasprintf(&strv[idx++], "distance: %.2f", dist);
		else
			len = xasprintf(&strv[idx++], "pressure: %.2f", pressure);
		if (len <= 0)
			goto out;
	}

	if (libinput_tablet_tool_has_rotation(tool)) {
		double rotation;

		rotation = libinput_event_tablet_tool_get_rotation(t);
		len = xasprintf(&strv[idx++], "rotation: %.2f", rotation);
		if (len <= 0)
			goto out;
	}

	if (libinput_tablet_tool_has_slider(tool)) {
		double slider;

		slider = libinput_event_tablet_tool_get_slider_position(t);
		len = xasprintf(&strv[idx++], "slider: %.2f", slider);
		if (len <= 0)
			goto out;

	}

	if (libinput_tablet_tool_has_wheel(tool)) {
		double wheel;
		int delta;

		wheel = libinput_event_tablet_tool_get_wheel_delta(t);
		len = xasprintf(&strv[idx++], "wheel: %.2f", wheel);
		if (len <= 0)
			goto out;

		delta = libinput_event_tablet_tool_get_wheel_delta_discrete(t);
		len = xasprintf(&strv[idx++], "wheel-discrete: %d", delta);
		if (len <= 0)
			goto out;
	}

	assert(idx < MAX_AXES);

	s = strv_join(strv, ", ");
out:
	strv_free(strv);
	return s;
}

static void
print_tablet_tool_proximity_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_tablet_tool *t =
		libinput_event_get_tablet_tool_event(e);
	struct libinput_tablet_tool *tool =
		libinput_event_tablet_tool_get_tool(t);
	uint64_t time;
	const char *type, *tool_type;
	char *axes;
	char caps[10] = {0};
	enum libinput_tablet_tool_proximity_state prox;
	size_t idx;

	switch (libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
		type = "TABLET_TOOL_PROXIMITY";
		break;
	default:
		abort();
	}

	switch (libinput_tablet_tool_get_type(tool)) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:
		tool_type = "pen";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:
		tool_type = "eraser";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:
		tool_type = "brush";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:
		tool_type = "brush";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:
		tool_type = "airbrush";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
		tool_type = "mouse";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:
		tool_type = "lens";
		break;
	default:
		tool_type = "unknown";
		break;
	}

	prox = libinput_event_tablet_tool_get_proximity_state(t);
	time = time_offset(dev->ctx, libinput_event_tablet_tool_get_time_usec(t));
	axes = buffer_tablet_axes(t);

	idx = 0;
	if (libinput_tablet_tool_has_pressure(tool))
		caps[idx++] = 'p';
	if (libinput_tablet_tool_has_distance(tool))
		caps[idx++] = 'd';
	if (libinput_tablet_tool_has_tilt(tool))
		caps[idx++] = 't';
	if (libinput_tablet_tool_has_rotation(tool))
		caps[idx++] = 'r';
	if (libinput_tablet_tool_has_slider(tool))
		caps[idx++] = 's';
	if (libinput_tablet_tool_has_wheel(tool))
		caps[idx++] = 'w';
	assert(idx <= ARRAY_LENGTH(caps));

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, proximity: %s, tool-type: %s, serial: %" PRIu64 ", axes: %s, %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		prox ? "in" : "out",
		tool_type,
		libinput_tablet_tool_get_serial(tool),
		caps,
		axes);
	free(axes);
}

static void
print_tablet_tool_button_event(struct record_device *dev,
			       struct libinput_event *e)
{
	struct libinput_event_tablet_tool *t =
		libinput_event_get_tablet_tool_event(e);
	uint64_t time;
	const char *type;
	uint32_t button;
	enum libinput_button_state state;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		type = "TABLET_TOOL_BUTTON";
		break;
	default:
		abort();
	}

	button = libinput_event_tablet_tool_get_button(t);
	state = libinput_event_tablet_tool_get_button_state(t);
	time = time_offset(dev->ctx, libinput_event_tablet_tool_get_time_usec(t));

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, button: %d, state: %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		button,
		state ? "pressed" : "released");
}

static void
print_tablet_tool_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_tablet_tool *t =
		libinput_event_get_tablet_tool_event(e);
	uint64_t time;
	const char *type;
	char *axes;
	enum libinput_tablet_tool_tip_state tip;
	char btn_buffer[30] = {0};

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
		type = "TABLET_TOOL_AXIS";
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_TIP:
		type = "TABLET_TOOL_TIP";
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		type = "TABLET_TOOL_BUTTON";
		break;
	default:
		abort();
	}

	if (libinput_event_get_type(e) == LIBINPUT_EVENT_TABLET_TOOL_BUTTON) {
		uint32_t button;
		enum libinput_button_state state;

		button = libinput_event_tablet_tool_get_button(t);
		state = libinput_event_tablet_tool_get_button_state(t);
		snprintf(btn_buffer, sizeof(btn_buffer),
			 ", button: %d, state: %s\n",
			 button,
			 state ? "pressed" : "released");
	}

	tip = libinput_event_tablet_tool_get_tip_state(t);
	time = time_offset(dev->ctx, libinput_event_tablet_tool_get_time_usec(t));
	axes = buffer_tablet_axes(t);

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s%s, tip: %s, %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		btn_buffer, /* may be empty string */
		tip ? "down" : "up",
		axes);
	free(axes);
}

static void
print_tablet_pad_button_event(struct record_device *dev,
			      struct libinput_event *e)
{
	struct libinput_event_tablet_pad *p =
		libinput_event_get_tablet_pad_event(e);
	struct libinput_tablet_pad_mode_group *group;
	enum libinput_button_state state;
	unsigned int button, mode;
	const char *type;
	uint64_t time;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
		type = "TABLET_PAD_BUTTON";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_tablet_pad_get_time_usec(p));
	button = libinput_event_tablet_pad_get_button_number(p),
	state = libinput_event_tablet_pad_get_button_state(p);
	mode = libinput_event_tablet_pad_get_mode(p);
	group = libinput_event_tablet_pad_get_mode_group(p);

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, button: %d, state: %s, mode: %d, is-toggle: %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		button,
		state == LIBINPUT_BUTTON_STATE_PRESSED ? "pressed" : "released",
		mode,
		libinput_tablet_pad_mode_group_button_is_toggle(group, button) ? "true" : "false"
	       );

}

static void
print_tablet_pad_ringstrip_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_tablet_pad *p =
		libinput_event_get_tablet_pad_event(e);
	const char *source = NULL;
	unsigned int mode, number;
	const char *type;
	uint64_t time;
	double pos;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_TABLET_PAD_RING:
		type = "TABLET_PAD_RING";
		number = libinput_event_tablet_pad_get_ring_number(p);
	        pos = libinput_event_tablet_pad_get_ring_position(p);

		switch (libinput_event_tablet_pad_get_ring_source(p)) {
		case LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER:
			source = "finger";
			break;
		case LIBINPUT_TABLET_PAD_RING_SOURCE_UNKNOWN:
			source = "unknown";
			break;
		}
		break;
	case LIBINPUT_EVENT_TABLET_PAD_STRIP:
		type = "TABLET_PAD_STRIP";
		number = libinput_event_tablet_pad_get_strip_number(p);
	        pos = libinput_event_tablet_pad_get_strip_position(p);

		switch (libinput_event_tablet_pad_get_strip_source(p)) {
		case LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER:
			source = "finger";
			break;
		case LIBINPUT_TABLET_PAD_STRIP_SOURCE_UNKNOWN:
			source = "unknown";
			break;
		}
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_tablet_pad_get_time_usec(p));
	mode = libinput_event_tablet_pad_get_mode(p);

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, number: %d, position: %.2f, source: %s, mode: %d}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		number,
		pos,
		source,
		mode);
}

static void
print_switch_event(struct record_device *dev, struct libinput_event *e)
{
	struct libinput_event_switch *s = libinput_event_get_switch_event(e);
	enum libinput_switch_state state;
	uint32_t sw;
	const char *type;
	uint64_t time;

	switch(libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_SWITCH_TOGGLE:
		type = "SWITCH_TOGGLE";
		break;
	default:
		abort();
	}

	time = time_offset(dev->ctx, libinput_event_switch_get_time_usec(s));
	sw = libinput_event_switch_get_switch(s);
	state = libinput_event_switch_get_switch_state(s);

	iprintf(dev->fp,
		I_EVENT,
		"- {time: %ld.%06ld, type: %s, switch: %d, state: %s}\n",
		(long)(time / (int)1e6),
		(long)(time % (int)1e6),
		type,
		sw,
		state == LIBINPUT_SWITCH_STATE_ON ? "on" : "off");
}

static void
print_libinput_event(struct record_device *dev, struct libinput_event *e)
{
	switch (libinput_event_get_type(e)) {
	case LIBINPUT_EVENT_NONE:
		abort();
	case LIBINPUT_EVENT_DEVICE_ADDED:
	case LIBINPUT_EVENT_DEVICE_REMOVED:
		print_device_notify(dev, e);
		break;
	case LIBINPUT_EVENT_KEYBOARD_KEY:
		print_key_event(dev, e);
		break;
	case LIBINPUT_EVENT_POINTER_MOTION:
		print_motion_event(dev, e);
		break;
	case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
		print_absmotion_event(dev, e);
		break;
	case LIBINPUT_EVENT_POINTER_BUTTON:
		print_pointer_button_event(dev, e);
		break;
	case LIBINPUT_EVENT_POINTER_AXIS:
		print_pointer_axis_event(dev, e);
		break;
	case LIBINPUT_EVENT_TOUCH_DOWN:
	case LIBINPUT_EVENT_TOUCH_UP:
	case LIBINPUT_EVENT_TOUCH_MOTION:
	case LIBINPUT_EVENT_TOUCH_CANCEL:
	case LIBINPUT_EVENT_TOUCH_FRAME:
		print_touch_event(dev, e);
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
	case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
	case LIBINPUT_EVENT_GESTURE_PINCH_END:
	case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
	case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
	case LIBINPUT_EVENT_GESTURE_SWIPE_END:
		print_gesture_event(dev, e);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
		print_tablet_tool_proximity_event(dev, e);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
	case LIBINPUT_EVENT_TABLET_TOOL_TIP:
		print_tablet_tool_event(dev, e);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		print_tablet_tool_button_event(dev, e);
		break;
	case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
		print_tablet_pad_button_event(dev, e);
		break;
	case LIBINPUT_EVENT_TABLET_PAD_RING:
	case LIBINPUT_EVENT_TABLET_PAD_STRIP:
		print_tablet_pad_ringstrip_event(dev, e);
		break;
	case LIBINPUT_EVENT_SWITCH_TOGGLE:
		print_switch_event(dev, e);
		break;
	default:
		break;
	}
}

static bool
handle_hidraw(struct hidraw *hidraw)
{
	struct record_device *d = hidraw->device;
	unsigned char report[4096];
	const char *sep = "";
	struct timespec ts;
	struct timeval tv;
	uint64_t time;

	int rc = read(hidraw->fd, report, sizeof(report));
	if (rc <= 0)
		return false;

	/* hidraw doesn't give us a timestamps, we have to make them up */
	clock_gettime(CLOCK_MONOTONIC, &ts);
	time = s2us(ts.tv_sec) + ns2us(ts.tv_nsec);

	/* The first evdev event is guaranteed to have an event time earlier
	   than now, so we don't set the offset here, we rely on the evdev
	   events to do so. This potentially leaves us with multiple hidraw
	   events at timestap 0 but it's too niche to worry about.  */
	if (d->ctx->offset == 0)
		time = 0;
	else
		time = time_offset(d->ctx, time);

	tv = us2tv(time);

	iprintf(d->fp, I_EVENTTYPE, "- hid:\n");
	iprintf(d->fp, I_EVENT, "time: [%3lu, %6lu]\n", tv.tv_sec, tv.tv_usec);
	iprintf(d->fp, I_EVENT, "%s: [", hidraw->name);

	for (int byte = 0; byte < rc; byte++) {
		if (byte % 16 == 0) {
			iprintf(d->fp, I_NONE, "%s\n", sep);
			iprintf(d->fp, I_EVENT, "  ");
			iprintf(d->fp, I_NONE, "0x%02x", report[byte]);
		} else {
			iprintf(d->fp, I_NONE, "%s0x%02x", sep, report[byte]);
		}
		sep = ", ";
	}
	iprintf(d->fp, I_NONE, "\n");
	iprintf(d->fp, I_EVENT, "]\n");

	return true;
}

static bool
handle_libinput_events(struct record_context *ctx,
		       struct record_device *d,
		       bool start_frame)
{
	struct libinput_event *e;
	struct record_device *current = d;

	libinput_dispatch(ctx->libinput);
	e = libinput_get_event(ctx->libinput);
	if (!e)
		return false;

	if (start_frame)
		iprintf(d->fp, I_EVENTTYPE, "- libinput:\n");
	else
		iprintf(d->fp, I_EVENTTYPE, "libinput:\n");
	do {
		struct libinput_device *device = libinput_event_get_device(e);

		if (device != current->device) {
			struct record_device *tmp;
			bool found = false;
			list_for_each(tmp, &ctx->devices, link) {
				if (device == tmp->device) {
					current = tmp;
					found = true;
					break;
				}
			}
			assert(found);
		}

		print_libinput_event(current, e);
		libinput_event_destroy(e);
	} while ((e = libinput_get_event(ctx->libinput)) != NULL);

	return true;
}

static void
handle_events(struct record_context *ctx, struct record_device *d)
{
	bool has_events = true;

	while (has_events) {
		has_events = handle_evdev_frame(d);

		if (ctx->libinput)
			has_events |= handle_libinput_events(ctx,
							     d,
							     !has_events);
	}

	fflush(d->fp);
}

static void
print_libinput_header(FILE *fp, int timeout)
{
	iprintf(fp, I_TOPLEVEL, "libinput:\n");
	iprintf(fp, I_LIBINPUT, "version: \"%s\"\n", LIBINPUT_VERSION);
	iprintf(fp, I_LIBINPUT, "git: \"%s\"\n", LIBINPUT_GIT_VERSION);
	if (timeout > 0)
		iprintf(fp, I_LIBINPUT, "autorestart: %d\n", timeout);
}

static void
print_system_header(FILE *fp)
{
	struct utsname u;
	const char *kernel = "unknown";
	FILE *dmi, *osrelease;
	char dmistr[2048] = "unknown";

	iprintf(fp, I_TOPLEVEL, "system:\n");

	/* /etc/os-release version and distribution name */
	osrelease = fopen("/etc/os-release", "r");
	if (!osrelease)
		osrelease = fopen("/usr/lib/os-release", "r");
	if (osrelease) {
		char *distro = NULL, *version = NULL;
		char osrstr[256] = "unknown";

		while (fgets(osrstr, sizeof(osrstr), osrelease)) {
			osrstr[strlen(osrstr) - 1] = '\0'; /* linebreak */

			if (!distro && strneq(osrstr, "ID=", 3))
				distro = strstrip(&osrstr[3], "\"'");
			else if (!version && strneq(osrstr, "VERSION_ID=", 11))
				version = strstrip(&osrstr[11], "\"'");

			if (distro && version) {
				iprintf(fp,
					I_SYSTEM,
					"os: \"%s:%s\"\n",
					distro,
					version);
				break;
			}
		}
		free(distro);
		free(version);
		fclose(osrelease);
	}

	/* kernel version */
	if (uname(&u) != -1)
		kernel = u.release;
	iprintf(fp, I_SYSTEM, "kernel: \"%s\"\n", kernel);

	/* dmi modalias */
	dmi = fopen("/sys/class/dmi/id/modalias", "r");
	if (dmi) {
		if (fgets(dmistr, sizeof(dmistr), dmi)) {
			dmistr[strlen(dmistr) - 1] = '\0'; /* linebreak */
		} else {
			sprintf(dmistr, "unknown");
		}
		fclose(dmi);
	}
	iprintf(fp, I_SYSTEM, "dmi: \"%s\"\n", dmistr);
}

static void
print_header(FILE *fp, struct record_context *ctx)
{
	iprintf(fp, I_TOPLEVEL, "# libinput record\n");
	iprintf(fp, I_TOPLEVEL, "version: %d\n", FILE_VERSION_NUMBER);
	iprintf(fp, I_TOPLEVEL, "ndevices: %d\n", ctx->ndevices);
	print_libinput_header(fp, ctx->timeout);
	print_system_header(fp);
}

static void
print_description_abs(FILE *fp,
		      struct libevdev *dev,
		      unsigned int code)
{
	const struct input_absinfo *abs;

	abs = libevdev_get_abs_info(dev, code);
	assert(abs);

	iprintf(fp, I_EVDEV, "#       Value      %6d\n", abs->value);
	iprintf(fp, I_EVDEV, "#       Min        %6d\n", abs->minimum);
	iprintf(fp, I_EVDEV, "#       Max        %6d\n", abs->maximum);
	iprintf(fp, I_EVDEV, "#       Fuzz       %6d\n", abs->fuzz);
	iprintf(fp, I_EVDEV, "#       Flat       %6d\n", abs->flat);
	iprintf(fp, I_EVDEV, "#       Resolution %6d\n", abs->resolution);
}

static void
print_description_state(FILE *fp,
			struct libevdev *dev,
			unsigned int type,
			unsigned int code)
{
	int state = libevdev_get_event_value(dev, type, code);
	iprintf(fp, I_EVDEV, "#       State %d\n", state);
}

static void
print_description_codes(FILE *fp,
			struct libevdev *dev,
			unsigned int type)
{
	int max;

	max = libevdev_event_type_get_max(type);
	if (max == -1)
		return;

	iprintf(fp,
		I_EVDEV,
		"# Event type %d (%s)\n",
		type,
		libevdev_event_type_get_name(type));

	if (type == EV_SYN)
		return;

	for (unsigned int code = 0; code <= (unsigned int)max; code++) {
		if (!libevdev_has_event_code(dev, type, code))
			continue;

		iprintf(fp,
			I_EVDEV,
			"#   Event code %d (%s)\n",
			code,
			libevdev_event_code_get_name(type,
						     code));

		switch (type) {
		case EV_ABS:
			print_description_abs(fp, dev, code);
			break;
		case EV_LED:
		case EV_SW:
			print_description_state(fp, dev, type, code);
			break;
		}
	}
}

static void
print_description(FILE *fp, struct libevdev *dev)
{
	const struct input_absinfo *x, *y;
	int bustype;
	const char *busname;

	bustype = libevdev_get_id_bustype(dev);
	switch (bustype) {
	case BUS_USB:
		busname = " (usb) ";
		break;
	case BUS_BLUETOOTH:
		busname = " (bluetooth) ";
		break;
	case BUS_I2C:
		busname = " (i2c) ";
		break;
	case BUS_SPI:
		busname = " (spi) ";
		break;
	case BUS_RMI:
		busname = " (rmi) ";
		break;
	default:
		busname = " ";
		break;
	}

	iprintf(fp, I_EVDEV, "# Name: %s\n", libevdev_get_name(dev));
	iprintf(fp,
		I_EVDEV,
		"# ID: bus 0x%04x%svendor 0x%04x product 0x%04x version 0x%04x\n",
		bustype,
		busname,
		libevdev_get_id_vendor(dev),
		libevdev_get_id_product(dev),
		libevdev_get_id_version(dev));

	x = libevdev_get_abs_info(dev, ABS_X);
	y = libevdev_get_abs_info(dev, ABS_Y);
	if (x && y) {
		if (x->resolution && y->resolution) {
			int w, h;

			w = (x->maximum - x->minimum)/x->resolution;
			h = (y->maximum - y->minimum)/y->resolution;
			iprintf(fp, I_EVDEV, "# Size in mm: %dx%d\n", w, h);
		} else {
			iprintf(fp,
				I_EVDEV,
				"# Size in mm: unknown, missing resolution\n");
		}
	}

	iprintf(fp, I_EVDEV, "# Supported Events:\n");

	for (unsigned int type = 0; type < EV_CNT; type++) {
		if (!libevdev_has_event_type(dev, type))
			continue;

		print_description_codes(fp, dev, type);
	}

	iprintf(fp, I_EVDEV, "# Properties:\n");

	for (unsigned int prop = 0; prop < INPUT_PROP_CNT; prop++) {
		if (libevdev_has_property(dev, prop)) {
			iprintf(fp,
				I_EVDEV,
				"#    Property %d (%s)\n",
				prop,
				libevdev_property_get_name(prop));
		}
	}
}

static void
print_bits_info(FILE *fp, struct libevdev *dev)
{
	iprintf(fp, I_EVDEV, "name: \"%s\"\n", libevdev_get_name(dev));
	iprintf(fp,
		I_EVDEV,
		"id: [%d, %d, %d, %d]\n",
		libevdev_get_id_bustype(dev),
		libevdev_get_id_vendor(dev),
		libevdev_get_id_product(dev),
		libevdev_get_id_version(dev));
}

static void
print_bits_absinfo(FILE *fp, struct libevdev *dev)
{
	const struct input_absinfo *abs;

	if (!libevdev_has_event_type(dev, EV_ABS))
		return;

	iprintf(fp, I_EVDEV, "absinfo:\n");
	for (unsigned int code = 0; code < ABS_CNT; code++) {
		abs = libevdev_get_abs_info(dev, code);
		if (!abs)
			continue;

		iprintf(fp,
			I_EVDEV_DATA,
			"%d: [%d, %d, %d, %d, %d]\n",
			code,
			abs->minimum,
			abs->maximum,
			abs->fuzz,
			abs->flat,
			abs->resolution);
	}
}

static void
print_bits_codes(FILE *fp, struct libevdev *dev, unsigned int type)
{
	int max;
	const char *sep = "";

	max = libevdev_event_type_get_max(type);
	if (max == -1)
		return;

	iprintf(fp, I_EVDEV_DATA, "%d: [", type);

	for (unsigned int code = 0; code <= (unsigned int)max; code++) {
		if (!libevdev_has_event_code(dev, type, code))
			continue;

		iprintf(fp, I_NONE, "%s%d", sep, code);
		sep = ", ";
	}

	iprintf(fp, I_NONE, "] # %s\n", libevdev_event_type_get_name(type));
}

static void
print_bits_types(FILE *fp, struct libevdev *dev)
{
	iprintf(fp, I_EVDEV, "codes:\n");
	for (unsigned int type = 0; type < EV_CNT; type++) {
		if (!libevdev_has_event_type(dev, type))
			continue;
		print_bits_codes(fp, dev, type);
	}
}

static void
print_bits_props(FILE *fp, struct libevdev *dev)
{
	const char *sep = "";

	iprintf(fp, I_EVDEV, "properties: [");
	for (unsigned int prop = 0; prop < INPUT_PROP_CNT; prop++) {
		if (libevdev_has_property(dev, prop)) {
			iprintf(fp, I_NONE, "%s%d", sep, prop);
			sep = ", ";
		}
	}
	iprintf(fp, I_NONE, "]\n"); /* last entry, no comma */
}

static void
print_evdev_description(struct record_device *dev)
{
	struct libevdev *evdev = dev->evdev;

	iprintf(dev->fp, I_DEVICE, "evdev:\n");

	print_description(dev->fp, evdev);
	print_bits_info(dev->fp, evdev);
	print_bits_types(dev->fp, evdev);
	print_bits_absinfo(dev->fp, evdev);
	print_bits_props(dev->fp, evdev);
}

static void
print_hid_report_descriptor(struct record_device *dev)
{
	const char *prefix = "/dev/input/event";
	char syspath[PATH_MAX];
	unsigned char buf[1024];
	int len;
	int fd;
	const char *sep = "";

	/* we take the shortcut rather than the proper udev approach, the
	   report_descriptor is available in sysfs and two devices up from
	   our device.
	   This approach won't work for /dev/input/by-id devices. */
	if (!strstartswith(dev->devnode, prefix))
		return;

	len = snprintf(syspath,
		       sizeof(syspath),
		       "/sys/class/input/%s/device/device/report_descriptor",
		       safe_basename(dev->devnode));
	if (len <= 0)
		return;

	fd = open(syspath, O_RDONLY);
	if (fd == -1)
		return;

	iprintf(dev->fp, I_DEVICE, "hid: [");

	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		for (int i = 0; i < len; i++) {
			/* We can't have a trailing comma, so our line-break
			 * handling is awkward.
			 * For a linebreak: print the comma, break, indent,
			 *    then just the hex code.
			 * For the other values: print the comma plus the
			 *    hex code, unindented.
			 */
			if (i % 16 == 0) {
				iprintf(dev->fp, I_NONE, "%s\n", sep);
				iprintf(dev->fp, I_DEVICE, "  ");
				iprintf(dev->fp, I_NONE, "0x%02x", buf[i]);
			} else {
				iprintf(dev->fp, I_NONE, "%s0x%02x", sep, buf[i]);
			}
			sep = ", ";
		}
	}
	iprintf(dev->fp, I_NONE, "\n");
	iprintf(dev->fp, I_DEVICE, "]\n");

	close(fd);
}

static void
print_udev_properties(struct record_device *dev)
{
	struct udev *udev = NULL;
	struct udev_device *udev_device = NULL;
	struct udev_list_entry *entry;
	struct stat st;

	if (stat(dev->devnode, &st) < 0)
		return;

	udev = udev_new();
	if (!udev)
		goto out;

	udev_device = udev_device_new_from_devnum(udev, 'c', st.st_rdev);
	if (!udev_device)
		goto out;

	iprintf(dev->fp, I_DEVICE, "udev:\n");

	iprintf(dev->fp, I_UDEV, "properties:\n");

	entry = udev_device_get_properties_list_entry(udev_device);
	while (entry) {
		const char *key, *value;

		key = udev_list_entry_get_name(entry);

		if (strneq(key, "ID_INPUT", 8) ||
		    strneq(key, "LIBINPUT", 8) ||
		    strneq(key, "EVDEV_ABS", 9) ||
		    strneq(key, "MOUSE_DPI", 9) ||
		    strneq(key, "POINTINGSTICK_", 14)) {
			value = udev_list_entry_get_value(entry);
			iprintf(dev->fp, I_UDEV_DATA, "- %s=%s\n", key, value);
		}

		entry = udev_list_entry_get_next(entry);
	}

out:
	udev_device_unref(udev_device);
	udev_unref(udev);
}

static void
quirks_log_handler(struct libinput *this_is_null,
		   enum libinput_log_priority priority,
		   const char *format,
		   va_list args)
{
}

static void
list_print(void *userdata, const char *val)
{
	FILE *fp = userdata;

	iprintf(fp, I_QUIRKS, "- %s\n", val);
}

static void
print_device_quirks(struct record_device *dev)
{
	struct udev *udev = NULL;
	struct udev_device *udev_device = NULL;
	struct stat st;
	struct quirks_context *quirks;
	const char *data_path = LIBINPUT_QUIRKS_DIR;
	const char *override_file = LIBINPUT_QUIRKS_OVERRIDE_FILE;
	char *builddir = NULL;

	if (stat(dev->devnode, &st) < 0)
		return;

	if ((builddir = builddir_lookup())) {
		setenv("LIBINPUT_QUIRKS_DIR", LIBINPUT_QUIRKS_SRCDIR, 0);
		data_path = LIBINPUT_QUIRKS_SRCDIR;
		override_file = NULL;
	}

	free(builddir);

	quirks = quirks_init_subsystem(data_path,
				       override_file,
				       quirks_log_handler,
				       NULL,
				       QLOG_CUSTOM_LOG_PRIORITIES);
	if (!quirks) {
		fprintf(stderr,
			"Failed to load the device quirks from %s%s%s. "
			"This will negatively affect device behavior. "
			"See %s/device-quirks.html for details.\n",
			data_path,
			override_file ? " and " : "",
			override_file ? override_file : "",
			HTTP_DOC_LINK);
		return;
	}

	udev = udev_new();
	if (!udev)
		goto out;

	udev_device = udev_device_new_from_devnum(udev, 'c', st.st_rdev);
	if (!udev_device)
		goto out;

	iprintf(dev->fp, I_DEVICE, "quirks:\n");
	tools_list_device_quirks(quirks, udev_device, list_print, dev->fp);
out:
	udev_device_unref(udev_device);
	udev_unref(udev);
	quirks_context_unref(quirks);
}

static void
print_libinput_description(struct record_device *dev)
{
	struct libinput_device *device = dev->device;
	double w, h;
	struct cap {
		enum libinput_device_capability cap;
		const char *name;
	} caps[] =  {
		{LIBINPUT_DEVICE_CAP_KEYBOARD, "keyboard"},
		{LIBINPUT_DEVICE_CAP_POINTER, "pointer"},
		{LIBINPUT_DEVICE_CAP_TOUCH, "touch"},
		{LIBINPUT_DEVICE_CAP_TABLET_TOOL, "tablet"},
		{LIBINPUT_DEVICE_CAP_TABLET_PAD, "pad"},
		{LIBINPUT_DEVICE_CAP_GESTURE, "gesture"},
		{LIBINPUT_DEVICE_CAP_SWITCH, "switch"},
	};
	const char *sep = "";

	if (!device)
		return;

	iprintf(dev->fp, I_DEVICE, "libinput:\n");
	if (libinput_device_get_size(device, &w, &h) == 0)
		iprintf(dev->fp, I_LIBINPUTDEV, "size: [%.f, %.f]\n", w, h);

	iprintf(dev->fp, I_LIBINPUTDEV, "capabilities: [");
	ARRAY_FOR_EACH(caps, cap) {
		if (!libinput_device_has_capability(device, cap->cap))
			continue;
		iprintf(dev->fp, I_NONE, "%s%s", sep, cap->name);
		sep = ", ";
	}
	iprintf(dev->fp, I_NONE, "]\n");

	/* Configuration options should be printed here, but since they
	 * don't reflect the user-configured ones their usefulness is
	 * questionable. We need the ability to specify the options like in
	 * debug-events.
	 */
}

static void
print_device_description(struct record_device *dev)
{
	iprintf(dev->fp, I_DEVICE, "- node: %s\n", dev->devnode);

	print_evdev_description(dev);
	print_hid_report_descriptor(dev);
	print_udev_properties(dev);
	print_device_quirks(dev);
	print_libinput_description(dev);
}

static int is_event_node(const struct dirent *dir) {
	return strneq(dir->d_name, "event", 5);
}

static char *
select_device(void)
{
	struct dirent **namelist;
	int ndev, selected_device;
	int rc;
	char *device_path;
	bool has_eaccess = false;
	int available_devices = 0;
	const char *prefix = "";

	if (!isatty(STDERR_FILENO))
		prefix = "# ";

	ndev = scandir("/dev/input", &namelist, is_event_node, versionsort);
	if (ndev <= 0)
		return NULL;

	fprintf(stderr, "%sAvailable devices:\n", prefix);
	for (int i = 0; i < ndev; i++) {
		struct libevdev *device;
		char path[PATH_MAX];
		int fd = -1;

		snprintf(path,
			 sizeof(path),
			 "/dev/input/%s",
			 namelist[i]->d_name);
		fd = open(path, O_RDONLY);
		if (fd < 0) {
			if (errno == EACCES)
				has_eaccess = true;
			continue;
		}

		rc = libevdev_new_from_fd(fd, &device);
		close(fd);
		if (rc != 0)
			continue;

		fprintf(stderr, "%s%s:	%s\n", prefix, path, libevdev_get_name(device));
		libevdev_free(device);
		available_devices++;
	}

	for (int i = 0; i < ndev; i++)
		free(namelist[i]);
	free(namelist);

	if (available_devices == 0) {
		fprintf(stderr,
			"No devices available.%s\n",
			has_eaccess ? " Please re-run as root." : "");
		return NULL;
	}

	fprintf(stderr, "%sSelect the device event number: ", prefix);
	rc = scanf("%d", &selected_device);

	if (rc != 1 || selected_device < 0)
		return NULL;

	rc = xasprintf(&device_path, "/dev/input/event%d", selected_device);
	if (rc == -1)
		return NULL;

	return device_path;
}

static char **
all_devices(void)
{
	struct dirent **namelist;
	int ndev;
	char **devices = NULL;

	ndev = scandir("/dev/input", &namelist, is_event_node, versionsort);
	if (ndev <= 0)
		return NULL;

	devices = zalloc((ndev + 1)* sizeof *devices); /* NULL-terminated */
	for (int i = 0; i < ndev; i++) {
		char *device_path;

		int rc = xasprintf(&device_path,
				   "/dev/input/%s",
				   namelist[i]->d_name);
		if (rc == -1)
			goto error;

		devices[i] = device_path;
	}

	return devices;

error:
	for (int i = 0; i < ndev; i++)
		free(namelist[i]);
	free(namelist);
	if (devices)
		strv_free(devices);
	return NULL;
}

static char *
init_output_file(const char *file, bool is_prefix)
{
	char name[PATH_MAX];

	assert(file != NULL);

	if (is_prefix) {
		struct tm *tm;
		time_t t;
		char suffix[64];

		t = time(NULL);
		tm = localtime(&t);
		strftime(suffix, sizeof(suffix), "%F-%T", tm);
		snprintf(name,
			 sizeof(name),
			 "%s.%s",
			 file,
			 suffix);
	} else {
		snprintf(name, sizeof(name), "%s", file);
	}

	return safe_strdup(name);
}

static bool
open_output_files(struct record_context *ctx, bool is_prefix)
{
	FILE *out_file;
	struct record_device *d;

	if (ctx->output_file.name) {
		char *fname = init_output_file(ctx->output_file.name, is_prefix);
		ctx->output_file.name_with_suffix = fname;
		out_file = fopen(fname, "w");
		if (!out_file)
			return false;
	} else {
		ctx->output_file.name_with_suffix = safe_strdup("stdout");
		out_file = stdout;
	}

	ctx->first_device->fp = out_file;

	list_for_each(d, &ctx->devices, link) {
		if (d->fp)
			continue;
		d->fp = tmpfile();
	}

	return true;
}

static void
print_progress_bar(void)
{
	static uint8_t foo = 0;

	if (!isatty(STDERR_FILENO))
		return;

	if (++foo > 20)
		foo = 1;
	fprintf(stderr, "\rReceiving events: [%*s%*s]", foo, "*", 21 - foo, " ");
}

static void
print_wall_time(struct record_context *ctx)
{
	time_t t = time(NULL);
	struct tm tm;
	struct record_device *d;

	localtime_r(&t, &tm);

	list_for_each(d, &ctx->devices, link) {
		iprintf(d->fp,
			I_DEVICE,
			"# Current time is %02d:%02d:%02d\n",
			tm.tm_hour, tm.tm_min, tm.tm_sec);
		fflush(d->fp);
	}
}

static void
arm_timer(int timerfd)
{
	time_t t = time(NULL);
	struct tm tm;
	struct itimerspec interval = {
		.it_value = { 0, 0 },
		.it_interval = { 5, 0 },
	};

	localtime_r(&t, &tm);
	interval.it_value.tv_sec = 5 - (tm.tm_sec % 5);
	timerfd_settime(timerfd, 0, &interval, NULL);
}

static struct source *
add_source(struct record_context *ctx,
	   int fd,
	   source_dispatch_t dispatch,
	   void *user_data)
{
	struct source *source;
	struct epoll_event ep;

	assert(fd != -1);

	source = zalloc(sizeof *source);
	source->dispatch = dispatch;
	source->user_data = user_data;
	source->fd = fd;
	list_append(&ctx->sources, &source->link);

	memset(&ep, 0, sizeof ep);
	ep.events = EPOLLIN;
	ep.data.ptr = source;

	if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, fd, &ep) < 0) {
		free(source);
		return NULL;
	}

	return source;
}

static void
destroy_source(struct record_context *ctx, struct source *source)
{
	list_remove(&source->link);
	epoll_ctl(ctx->epoll_fd, EPOLL_CTL_DEL, source->fd, NULL);
	close(source->fd);
	free(source);
}

static void
signalfd_dispatch(struct record_context *ctx, int fd, void *data)
{
	struct signalfd_siginfo fdsi;

	(void)read(fd, &fdsi, sizeof(fdsi));

	ctx->stop = true;
}

static void
timefd_dispatch(struct record_context *ctx, int fd, void *data)
{
	char discard[64];

	(void)read(fd, discard, sizeof(discard));

	if (ctx->timestamps.had_events_since_last_time) {
		print_wall_time(ctx);
		ctx->timestamps.had_events_since_last_time = false;
		ctx->timestamps.skipped_timer_print = false;
	} else {
		ctx->timestamps.skipped_timer_print = true;
	}
}

static void
evdev_dispatch(struct record_context *ctx, int fd, void *data)
{
	struct record_device *this_device = data;

	if (ctx->timestamps.skipped_timer_print) {
		print_wall_time(ctx);
		ctx->timestamps.skipped_timer_print = false;
	}

	ctx->had_events = true;
	ctx->timestamps.had_events_since_last_time = true;

	handle_events(ctx, this_device);
}

static void
libinput_ctx_dispatch(struct record_context *ctx, int fd, void *data)
{
	/* This function should only handle events caused by internal
	 * timeouts etc. The real input events caused by the evdev devices
	 * are already processed in handle_events */
	libinput_dispatch(ctx->libinput);
	handle_libinput_events(ctx, ctx->first_device, true);
}

static void
hidraw_dispatch(struct record_context *ctx, int fd, void *data)
{
	struct hidraw *hidraw = data;

	ctx->had_events = true;
	ctx->timestamps.had_events_since_last_time = true;
	handle_hidraw(hidraw);
}

static int
dispatch_sources(struct record_context *ctx)
{
	struct source *source;
	struct epoll_event ep[64];
	int i, count;

	count = epoll_wait(ctx->epoll_fd, ep, ARRAY_LENGTH(ep), ctx->timeout);
	if (count < 0)
		return -errno;

	for (i = 0; i < count; ++i) {
		source = ep[i].data.ptr;
		if (source->fd == -1)
			continue;
		source->dispatch(ctx, source->fd, source->user_data);
	}

	return count;
}

static int
mainloop(struct record_context *ctx)
{
	bool autorestart = (ctx->timeout > 0);
	struct source *source;
	struct record_device *d = NULL;
	sigset_t mask;
	int sigfd, timerfd;

	assert(ctx->timeout != 0);
	assert(!list_empty(&ctx->devices));

	ctx->epoll_fd = epoll_create1(0);
	assert(ctx->epoll_fd >= 0);

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	sigfd = signalfd(-1, &mask, SFD_NONBLOCK);
	add_source(ctx, sigfd, signalfd_dispatch, NULL);

	timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
	add_source(ctx, timerfd, timefd_dispatch, NULL);
	arm_timer(timerfd);

	list_for_each(d, &ctx->devices, link) {
		struct hidraw *hidraw;

		add_source(ctx, libevdev_get_fd(d->evdev), evdev_dispatch, d);

		list_for_each(hidraw, &d->hidraw_devices, link) {
			add_source(ctx, hidraw->fd, hidraw_dispatch, hidraw);
		}
	}

	if (ctx->libinput) {
		/* See the note in the dispatch function */
		add_source(ctx,
			   libinput_get_fd(ctx->libinput),
			   libinput_ctx_dispatch,
			   NULL);
	}

	/* If we have more than one device, the time starts at recording
	 * start time. Otherwise, the first event starts the recording time.
	 */
	if (ctx->ndevices > 1) {
		struct timespec ts;

		clock_gettime(CLOCK_MONOTONIC, &ts);
		ctx->offset = s2us(ts.tv_sec) + ns2us(ts.tv_nsec);
	}

	do {
		struct record_device *d;

		if (!open_output_files(ctx, autorestart)) {
			fprintf(stderr,
				"Failed to open '%s'\n",
				ctx->output_file.name_with_suffix);
			break;
		}
		fprintf(stderr, "%sRecording to '%s'.\n",
			isatty(STDERR_FILENO) ? "" : "# ",
			ctx->output_file.name_with_suffix);

		ctx->had_events = false;

		print_header(ctx->first_device->fp, ctx);
		if (autorestart)
			iprintf(ctx->first_device->fp,
				I_NONE,
				"# Autorestart timeout: %d\n",
				ctx->timeout);

		iprintf(ctx->first_device->fp, I_TOPLEVEL, "devices:\n");

		/* we only print the first device's description, the
		 * rest is assembled after CTRL+C */
		list_for_each(d, &ctx->devices, link) {
			print_device_description(d);
			iprintf(d->fp, I_DEVICE, "events:\n");
		}
		print_wall_time(ctx);

		if (ctx->libinput) {
			libinput_dispatch(ctx->libinput);
			handle_libinput_events(ctx, ctx->first_device, true);
		}

		while (true) {
			int rc = dispatch_sources(ctx);
			if (rc < 0) { /* error */
				fprintf(stderr, "Error: %s\n", strerror(-rc));
				ctx->stop = true;
				break;
			}

			/* set by the signalfd handler */
			if (ctx->stop)
				break;

			if (rc == 0) {
				fprintf(stderr,
					" ... timeout%s\n",
					ctx->had_events ? "" : " (file is empty)");
				break;

			}

			if (ctx->first_device->fp != stdout)
				print_progress_bar();

		}

		if (autorestart) {
			list_for_each(d, &ctx->devices, link) {
				iprintf(d->fp,
					I_NONE,
					"# Closing after %ds inactivity",
					ctx->timeout/1000);
			}
		}

		/* First device is printed, now append all the data from the
		 * other devices, if any */
		list_for_each(d, &ctx->devices, link) {
			char buf[4096];
			size_t n;

			if (d == ctx->first_device)
				continue;

			rewind(d->fp);
			do {

				n = fread(buf, 1, sizeof(buf), d->fp);
				if (n > 0)
					fwrite(buf, 1, n, ctx->first_device->fp);
			} while (n == sizeof(buf));

			fclose(d->fp);
			d->fp = NULL;
		}

		/* If we didn't have events, delete the file. */
		if (!isatty(fileno(ctx->first_device->fp))) {
			struct record_device *d;

			if (!ctx->had_events && ctx->output_file.name_with_suffix) {
				fprintf(stderr,
					"No events recorded, deleting '%s'\n",
					ctx->output_file.name_with_suffix);
				unlink(ctx->output_file.name_with_suffix);
			}

			list_for_each(d, &ctx->devices, link) {
				if (d->fp && d->fp != stdout) {
					fclose(d->fp);
					d->fp = NULL;
				}
			}
		}
		free(ctx->output_file.name_with_suffix);
		ctx->output_file.name_with_suffix = NULL;
	} while (autorestart && !ctx->stop);

	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	list_for_each_safe(source, &ctx->sources, link) {
		destroy_source(ctx, source);
	}
	close(ctx->epoll_fd);

	return 0;
}

static bool
init_device(struct record_context *ctx, const char *path, bool grab)
{
	struct record_device *d;
	int fd, rc;

	d = zalloc(sizeof(*d));
	d->ctx = ctx;
	d->devnode = safe_strdup(path);

	list_init(&d->hidraw_devices);

	fd = open(d->devnode, O_RDONLY|O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr,
			"Failed to open device %s (%m)\n",
			d->devnode);
		goto error;
	}

	rc = libevdev_new_from_fd(fd, &d->evdev);
	if (rc == 0)
		rc = libevdev_new_from_fd(fd, &d->evdev_prev);
	if (rc != 0) {
		fprintf(stderr,
			"Failed to create context for %s (%s)\n",
			d->devnode,
			strerror(-rc));
		goto error;
	}

	if (grab) {
		rc = libevdev_grab(d->evdev, LIBEVDEV_GRAB);
		if (rc != 0) {
			fprintf(stderr,
				"Grab failed on %s: %s\n",
				path,
				strerror(-rc));
			goto error;
		}
	}

	libevdev_set_clock_id(d->evdev, CLOCK_MONOTONIC);

	if (libevdev_get_num_slots(d->evdev) > 0)
		d->touch.is_touch_device = true;

	list_append(&ctx->devices, &d->link);
	if (!ctx->first_device)
		ctx->first_device = d;
	ctx->ndevices++;

	return true;
error:
	close(fd);
	free(d);
	return false;

}
static int
open_restricted(const char *path, int flags, void *user_data)
{
	int fd = open(path, flags);
	return fd == -1 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data)
{
	close(fd);
}

static const struct libinput_interface interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

static bool
init_libinput(struct record_context *ctx)
{
	struct record_device *dev;
	struct libinput *li;

	li = libinput_path_create_context(&interface, NULL);
	if (li == NULL) {
		fprintf(stderr,
			"Failed to create libinput context\n");
		return false;
	}

	ctx->libinput = li;

	list_for_each(dev, &ctx->devices, link) {
		struct libinput_device *d;

		d = libinput_path_add_device(li, dev->devnode);
		if (!d) {
			fprintf(stderr,
				"Failed to add device %s\n",
				dev->devnode);
			continue;
		}
		dev->device = libinput_device_ref(d);
		/* FIXME: this needs to be a commandline option */
		libinput_device_config_tap_set_enabled(d,
					       LIBINPUT_CONFIG_TAP_ENABLED);
	}

	return true;
}

static bool
init_hidraw(struct record_context *ctx)
{
	struct record_device *dev;

	list_for_each(dev, &ctx->devices, link) {
		char syspath[PATH_MAX];
		DIR *dir;
		struct dirent *entry;

		snprintf(syspath,
			 sizeof(syspath),
			 "/sys/class/input/%s/device/device/hidraw",
			 safe_basename(dev->devnode));
		dir = opendir(syspath);
		if (!dir)
			continue;

		while ((entry = readdir(dir))) {
			char hidraw_node[PATH_MAX];
			int fd;
			struct hidraw *hidraw = NULL;

			if (!strstartswith(entry->d_name, "hidraw"))
				continue;

			snprintf(hidraw_node,
				 sizeof(hidraw_node),
				 "/dev/%s",
				 entry->d_name);
			fd = open(hidraw_node, O_RDONLY|O_NONBLOCK);
			if (fd == -1)
				continue;

			hidraw = zalloc(sizeof(*hidraw));
			hidraw->fd = fd;
			hidraw->name = safe_strdup(entry->d_name);
			hidraw->device = dev;
			list_insert(&dev->hidraw_devices, &hidraw->link);
		}
		closedir(dir);
	}

	return true;
}

static void
usage(void)
{
	printf("Usage: %s [--help] [--all] [--autorestart] [--output-file filename] [/dev/input/event0] [...]\n"
	       "Common use-cases:\n"
	       "\n"
	       " sudo %s -o recording.yml\n"
	       "    Then select the device to record and it Ctrl+C to stop.\n"
	       "    The recorded data is in recording.yml and can be attached to a bug report.\n"
	       "\n"
	       " sudo %s -o recording.yml --autorestart 2\n"
	       "    As above, but restarts after 2s of inactivity on the device.\n"
	       "    Note, the output file is only the prefix.\n"
	       "\n"
	       " sudo %s -o recording.yml /dev/input/event3 /dev/input/event4\n"
	       "    Records the two devices into the same recordings file.\n"
	       "\n"
	       "For more information, see the %s(1) man page\n",
	       program_invocation_short_name,
	       program_invocation_short_name,
	       program_invocation_short_name,
	       program_invocation_short_name,
	       program_invocation_short_name);
}

enum ftype {
	F_FILE = 8,
	F_DEVICE,
	F_NOEXIST,
};

static enum ftype
is_char_dev(const char *path)
{
	struct stat st;

	if (strneq(path, "/dev", 4))
		return F_DEVICE;

	if (stat(path, &st) != 0) {
		if (errno == ENOENT)
			return F_NOEXIST;
		return F_FILE;
	}

	return S_ISCHR(st.st_mode) ? F_DEVICE : F_FILE;
}

enum fposition {
	ERROR,
	NO_FILE,
	FIRST,
	LAST,
};

static enum fposition
find_output_file(int argc, char *argv[], const char **output_file)
{
	char *first, *last;
	enum ftype ftype_first, ftype_last;

	first = argv[0];

	ftype_first = is_char_dev(first);
	if (argc == 1) {
		/* arg is *not* a char device, so let's assume it's
		 * the output file */
		if (ftype_first != F_DEVICE) {
			*output_file = first;
			return FIRST;
		}
	}

	/* multiple arguments, yay */
	last = argv[argc - 1];
	ftype_last = is_char_dev(last);
	/*
	   first is device, last is file -> last
	   first is device, last is device -> noop
	   first is device, last !exist -> last
	   first is file, last is device -> first
	   first is file, last is file -> error
	   first is file, last !exist -> error
	   first !exist, last is device -> first
	   first !exist, last is file -> error
	   first !exit, last !exist -> error
	 */
#define _m(f, l) (((f) << 8) | (l))
	switch (_m(ftype_first, ftype_last)) {
	case _m(F_FILE,    F_DEVICE):
	case _m(F_FILE,    F_NOEXIST):
	case _m(F_NOEXIST, F_DEVICE):
		*output_file = first;
		return FIRST;
	case _m(F_DEVICE,  F_FILE):
	case _m(F_DEVICE,  F_NOEXIST):
		*output_file = last;
		return LAST;
	case _m(F_DEVICE,  F_DEVICE):
		break;
	case _m(F_FILE,    F_FILE):
	case _m(F_NOEXIST, F_FILE):
	case _m(F_NOEXIST, F_NOEXIST):
		return ERROR;
	}
#undef _m
	return NO_FILE;
}

enum options {
	OPT_AUTORESTART,
	OPT_HELP,
	OPT_OUTFILE,
	OPT_KEYCODES,
	OPT_MULTIPLE,
	OPT_ALL,
	OPT_LIBINPUT,
	OPT_HIDRAW,
	OPT_GRAB,
};

int
main(int argc, char **argv)
{
	struct record_context ctx = {
		.timeout = -1,
		.show_keycodes = false,
	};
	struct option opts[] = {
		{ "autorestart", required_argument, 0, OPT_AUTORESTART },
		{ "output-file", required_argument, 0, OPT_OUTFILE },
		{ "show-keycodes", no_argument, 0, OPT_KEYCODES },
		{ "multiple", no_argument, 0, OPT_MULTIPLE },
		{ "all", no_argument, 0, OPT_ALL },
		{ "help", no_argument, 0, OPT_HELP },
		{ "with-libinput", no_argument, 0, OPT_LIBINPUT },
		{ "with-hidraw", no_argument, 0, OPT_HIDRAW },
		{ "grab", no_argument, 0, OPT_GRAB },
		{ 0, 0, 0, 0 },
	};
	struct record_device *d;
	const char *output_arg = NULL;
	bool all = false,
	     with_libinput = false,
	     with_hidraw = false,
	     grab = false;
	int ndevices;
	int rc = EXIT_FAILURE;
	char **paths = NULL;

	list_init(&ctx.devices);
	list_init(&ctx.sources);

	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long(argc, argv, "ho:", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
		case OPT_HELP:
			usage();
			rc = EXIT_SUCCESS;
			goto out;
		case OPT_AUTORESTART:
			if (!safe_atoi(optarg, &ctx.timeout) ||
			    ctx.timeout <= 0) {
				usage();
				rc = EXIT_INVALID_USAGE;
				goto out;
			}
			ctx.timeout = ctx.timeout * 1000;
			break;
		case 'o':
		case OPT_OUTFILE:
			output_arg = optarg;
			break;
		case OPT_KEYCODES:
			ctx.show_keycodes = true;
			break;
		case OPT_MULTIPLE: /* deprecated */
			break;
		case OPT_ALL:
			all = true;
			break;
		case OPT_LIBINPUT:
			with_libinput = true;
			break;
		case OPT_HIDRAW:
			with_hidraw = true;
			fprintf(stderr, "# WARNING: do not type passwords while recording HID reports\n");
			break;
		case OPT_GRAB:
			grab = true;
			break;
		default:
			usage();
			rc = EXIT_INVALID_USAGE;
			goto out;
		}
	}

	ndevices = argc - optind;

	/* We allow for multiple arguments after the options, *one* of which
	 * may be the output file. That one must be the first or the last to
	 * prevent users from running
	 *   libinput record /dev/input/event0 output.yml /dev/input/event1
	 * because this will only backfire anyway.
	 */
	if (ndevices >= 1 && output_arg == NULL) {
		enum fposition pos = find_output_file(argc - optind,
						      &argv[optind],
						      &output_arg);
		if (pos == ERROR) {
			fprintf(stderr,
				"Ambiguous device vs output file list. "
				"Please use --output-file.\n");
			return EXIT_INVALID_USAGE;
		}

		if (pos == FIRST || pos == LAST)
			ndevices--;
		if (pos == FIRST)
			optind++;
	}

	if (ctx.timeout > 0 && output_arg == NULL) {
		fprintf(stderr,
			"Option --autorestart requires --output-file\n");
		rc = EXIT_INVALID_USAGE;
		goto out;
	}

	ctx.output_file.name = safe_strdup(output_arg);

	if (output_arg == NULL && (all || ndevices > 1)) {
		fprintf(stderr,
			"Recording multiple devices requires an output file\n");
		rc = EXIT_INVALID_USAGE;
		goto out;
	}

	/* Now collect all device paths and init our device struct */
	if (all) {
		paths = all_devices();
	} else if (ndevices >= 1) {
		paths = strv_from_argv(ndevices, &argv[optind]);
	} else {
		char *path = select_device();
		if (path == NULL) {
			goto out;
		}

		paths = strv_from_argv(1, &path);
		free(path);
	}

	for (char **p = paths; *p; p++) {
		if (!init_device(&ctx, *p, grab)) {
			goto out;
		}
	}

	if (with_libinput && !init_libinput(&ctx))
		goto out;

	if (with_hidraw && !init_hidraw(&ctx))
		goto out;

	rc = mainloop(&ctx);
out:
	strv_free(paths);
	list_for_each_safe(d, &ctx.devices, link) {
		struct hidraw *hidraw;

		list_for_each_safe(hidraw, &d->hidraw_devices, link) {
			close(hidraw->fd);
			list_remove(&hidraw->link);
			free(hidraw->name);
			free(hidraw);
		}

		if (d->device)
			libinput_device_unref(d->device);
		free(d->devnode);
		libevdev_free(d->evdev);
	}

	libinput_unref(ctx.libinput);

	return rc;
}
