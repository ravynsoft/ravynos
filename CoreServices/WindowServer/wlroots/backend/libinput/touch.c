#include <assert.h>
#include <libinput.h>
#include <wlr/interfaces/wlr_touch.h>
#include "backend/libinput.h"
#include "util/signal.h"

const struct wlr_touch_impl libinput_touch_impl = {
	.name = "libinput-touch",
};

void init_device_touch(struct wlr_libinput_input_device *dev) {
	const char *name = libinput_device_get_name(dev->handle);
	struct wlr_touch *wlr_touch = &dev->touch;
	wlr_touch_init(wlr_touch, &libinput_touch_impl, name);
	wlr_touch->base.vendor = libinput_device_get_id_vendor(dev->handle);
	wlr_touch->base.product = libinput_device_get_id_product(dev->handle);
}

struct wlr_libinput_input_device *device_from_touch(
		struct wlr_touch *wlr_touch) {
	assert(wlr_touch->impl == &libinput_touch_impl);

	struct wlr_libinput_input_device *dev =
		wl_container_of(wlr_touch, dev, touch);
	return dev;
}

void handle_touch_down(struct libinput_event *event,
		struct wlr_touch *touch) {
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_down wlr_event = { 0 };
	wlr_event.device = &touch->base;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_event.x = libinput_event_touch_get_x_transformed(tevent, 1);
	wlr_event.y = libinput_event_touch_get_y_transformed(tevent, 1);
	wlr_signal_emit_safe(&touch->events.down, &wlr_event);
}

void handle_touch_up(struct libinput_event *event,
		struct wlr_touch *touch) {
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_up wlr_event = { 0 };
	wlr_event.device = &touch->base;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_signal_emit_safe(&touch->events.up, &wlr_event);
}

void handle_touch_motion(struct libinput_event *event,
		struct wlr_touch *touch) {
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_motion wlr_event = { 0 };
	wlr_event.device = &touch->base;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_event.x = libinput_event_touch_get_x_transformed(tevent, 1);
	wlr_event.y = libinput_event_touch_get_y_transformed(tevent, 1);
	wlr_signal_emit_safe(&touch->events.motion, &wlr_event);
}

void handle_touch_cancel(struct libinput_event *event,
		struct wlr_touch *touch) {
	struct libinput_event_touch *tevent =
		libinput_event_get_touch_event(event);
	struct wlr_event_touch_cancel wlr_event = { 0 };
	wlr_event.device = &touch->base;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_touch_get_time_usec(tevent));
	wlr_event.touch_id = libinput_event_touch_get_seat_slot(tevent);
	wlr_signal_emit_safe(&touch->events.cancel, &wlr_event);
}

void handle_touch_frame(struct libinput_event *event,
		struct wlr_touch *touch) {
	wlr_signal_emit_safe(&touch->events.frame, NULL);
}
