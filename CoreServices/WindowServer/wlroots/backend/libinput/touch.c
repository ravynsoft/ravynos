#include <assert.h>
#include <libinput.h>
#include <stdlib.h>
#include <wlr/backend/session.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/signal.h"

struct wlr_touch *create_libinput_touch(
		struct libinput_device *libinput_dev) {
	assert(libinput_dev);
	struct wlr_touch *wlr_touch = calloc(1, sizeof(struct wlr_touch));
	if (!wlr_touch) {
		wlr_log(WLR_ERROR, "Unable to allocate wlr_touch");
		return NULL;
	}
	wlr_touch_init(wlr_touch, NULL);
	return wlr_touch;
}

void handle_touch_down(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TOUCH, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG, "Got a touch event for a device with no touch?");
		return;
	}
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_down wlr_event = { 0 };
	wlr_event.device = wlr_dev;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_event.x = libinput_event_touch_get_x_transformed(tevent, 1);
	wlr_event.y = libinput_event_touch_get_y_transformed(tevent, 1);
	wlr_signal_emit_safe(&wlr_dev->touch->events.down, &wlr_event);
}

void handle_touch_up(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TOUCH, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG, "Got a touch event for a device with no touch?");
		return;
	}
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_up wlr_event = { 0 };
	wlr_event.device = wlr_dev;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_signal_emit_safe(&wlr_dev->touch->events.up, &wlr_event);
}

void handle_touch_motion(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TOUCH, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG, "Got a touch event for a device with no touch?");
		return;
	}
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_motion wlr_event = { 0 };
	wlr_event.device = wlr_dev;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_event.x = libinput_event_touch_get_x_transformed(tevent, 1);
	wlr_event.y = libinput_event_touch_get_y_transformed(tevent, 1);
	wlr_signal_emit_safe(&wlr_dev->touch->events.motion, &wlr_event);
}

void handle_touch_cancel(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TOUCH, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG, "Got a touch event for a device with no touch?");
		return;
	}
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_cancel wlr_event = { 0 };
	wlr_event.device = wlr_dev;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_signal_emit_safe(&wlr_dev->touch->events.cancel, &wlr_event);
}

void handle_touch_frame(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TOUCH, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG, "Got a touch event for a device with no touch?");
		return;
	}
	wlr_signal_emit_safe(&wlr_dev->touch->events.frame, NULL);
}
