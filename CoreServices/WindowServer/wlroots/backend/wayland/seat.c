#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <wayland-client.h>

#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/util/log.h>

#include "pointer-gestures-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
#include "backend/wayland.h"
#include "util/signal.h"
#include "util/time.h"

static struct wlr_wl_pointer *output_get_pointer(
		struct wlr_wl_output *output,
		const struct wl_pointer *wl_pointer) {
	struct wlr_wl_input_device *dev;
	wl_list_for_each(dev, &output->backend->devices, link) {
		if (dev->wlr_input_device.type != WLR_INPUT_DEVICE_POINTER) {
			continue;
		}
		struct wlr_wl_pointer *pointer =
			pointer_get_wl(dev->wlr_input_device.pointer);
		if (pointer->output == output && pointer->wl_pointer == wl_pointer) {
			return pointer;
		}
	}

	return NULL;
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface, wl_fixed_t sx,
		wl_fixed_t sy) {
	struct wlr_wl_seat *seat = data;
	if (surface == NULL) {
		return;
	}

	struct wlr_wl_output *output = wl_surface_get_user_data(surface);
	assert(output);
	struct wlr_wl_pointer *pointer = output_get_pointer(output, wl_pointer);
	seat->active_pointer = pointer;

	// Manage cursor icon/rendering on output
	struct wlr_wl_pointer *current_pointer = output->cursor.pointer;
	if (current_pointer && current_pointer != pointer) {
		wlr_log(WLR_INFO, "Ignoring seat %s pointer cursor in favor of seat %s",
			seat->name, current_pointer->input_device->seat->name);
		return;
	}

	output->enter_serial = serial;
	output->cursor.pointer = pointer;
	update_wl_output_cursor(output);
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	struct wlr_wl_seat *seat = data;
	if (surface == NULL) {
		return;
	}

	struct wlr_wl_output *output = wl_surface_get_user_data(surface);
	assert(output);

	if (seat->active_pointer != NULL &&
			seat->active_pointer->output == output) {
		seat->active_pointer = NULL;
	}

	if (output->cursor.pointer == seat->active_pointer) {
		output->enter_serial = 0;
		output->cursor.pointer = NULL;
	}
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_output *wlr_output = &pointer->output->wlr_output;
	struct wlr_event_pointer_motion_absolute event = {
		.device = &pointer->input_device->wlr_input_device,
		.time_msec = time,
		.x = wl_fixed_to_double(sx) / wlr_output->width,
		.y = wl_fixed_to_double(sy) / wlr_output->height,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.motion_absolute, &event);
}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_event_pointer_button event = {
		.device = &pointer->input_device->wlr_input_device,
		.button = button,
		.state = state,
		.time_msec = time,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.button, &event);
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_event_pointer_axis event = {
		.device = &pointer->input_device->wlr_input_device,
		.delta = wl_fixed_to_double(value),
		.delta_discrete = pointer->axis_discrete,
		.orientation = axis,
		.time_msec = time,
		.source = pointer->axis_source,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.axis, &event);

	pointer->axis_discrete = 0;
}

static void pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	wlr_signal_emit_safe(&pointer->wlr_pointer.events.frame,
		&pointer->wlr_pointer);
}

static void pointer_handle_axis_source(void *data,
		struct wl_pointer *wl_pointer, uint32_t axis_source) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	pointer->axis_source = axis_source;
}

static void pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_event_pointer_axis event = {
		.device = &pointer->input_device->wlr_input_device,
		.delta = 0,
		.delta_discrete = 0,
		.orientation = axis,
		.time_msec = time,
		.source = pointer->axis_source,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.axis, &event);
}

static void pointer_handle_axis_discrete(void *data,
		struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	pointer->axis_discrete = discrete;
}

static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
	.frame = pointer_handle_frame,
	.axis_source = pointer_handle_axis_source,
	.axis_stop = pointer_handle_axis_stop,
	.axis_discrete = pointer_handle_axis_discrete,
};

static void keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t format, int32_t fd, uint32_t size) {
	close(fd);
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	struct wlr_input_device *dev = data;

	uint32_t time = get_current_time_msec();

	uint32_t *keycode_ptr;
	wl_array_for_each(keycode_ptr, keys) {
		struct wlr_event_keyboard_key event = {
			.keycode = *keycode_ptr,
			.state = WL_KEYBOARD_KEY_STATE_PRESSED,
			.time_msec = time,
			.update_state = false,
		};
		wlr_keyboard_notify_key(dev->keyboard, &event);
	}
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface) {
	struct wlr_input_device *dev = data;

	uint32_t time = get_current_time_msec();

	size_t num_keycodes = dev->keyboard->num_keycodes;
	uint32_t pressed[num_keycodes + 1];
	memcpy(pressed, dev->keyboard->keycodes,
		num_keycodes * sizeof(uint32_t));

	for (size_t i = 0; i < num_keycodes; ++i) {
		uint32_t keycode = pressed[i];

		struct wlr_event_keyboard_key event = {
			.keycode = keycode,
			.state = WL_KEYBOARD_KEY_STATE_RELEASED,
			.time_msec = time,
			.update_state = false,
		};
		wlr_keyboard_notify_key(dev->keyboard, &event);
	}
}

static void keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct wlr_input_device *dev = data;
	assert(dev && dev->keyboard);

	struct wlr_event_keyboard_key wlr_event = {
		.keycode = key,
		.state = state,
		.time_msec = time,
		.update_state = false,
	};
	wlr_keyboard_notify_key(dev->keyboard, &wlr_event);
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	struct wlr_input_device *dev = data;
	assert(dev && dev->keyboard);
	wlr_keyboard_notify_modifiers(dev->keyboard, mods_depressed, mods_latched,
		mods_locked, group);
}

static void keyboard_handle_repeat_info(void *data,
		struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
	// This space is intentionally left blank
}

static const struct wl_keyboard_listener keyboard_listener = {
	.keymap = keyboard_handle_keymap,
	.enter = keyboard_handle_enter,
	.leave = keyboard_handle_leave,
	.key = keyboard_handle_key,
	.modifiers = keyboard_handle_modifiers,
	.repeat_info = keyboard_handle_repeat_info
};

static void touch_coordinates_to_absolute(struct wlr_wl_input_device *device,
		wl_fixed_t x, wl_fixed_t y, double *sx, double *sy) {
	// TODO: each output needs its own touch
	struct wlr_wl_output *output, *tmp;
	wl_list_for_each_safe(output, tmp, &device->backend->outputs, link) {
		*sx = wl_fixed_to_double(x) / output->wlr_output.width;
		*sy = wl_fixed_to_double(y) / output->wlr_output.height;
		return; // Choose the first output in the list
	}

	*sx = *sy = 0;
}

static void touch_handle_down(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, struct wl_surface *surface,
		int32_t id, wl_fixed_t x, wl_fixed_t y) {
	struct wlr_wl_input_device *device = data;
	assert(device && device->wlr_input_device.touch);

	double sx, sy;
	touch_coordinates_to_absolute(device, x, y, &sx, &sy);
	struct wlr_event_touch_down event = {
		.device = &device->wlr_input_device,
		.time_msec = time,
		.touch_id = id,
		.x = sx,
		.y = sy
	};
	wlr_signal_emit_safe(&device->wlr_input_device.touch->events.down, &event);
}

static void touch_handle_up(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, int32_t id) {
	struct wlr_wl_input_device *device = data;
	assert(device && device->wlr_input_device.touch);

	struct wlr_event_touch_up event = {
		.device = &device->wlr_input_device,
		.time_msec = time,
		.touch_id = id,
	};
	wlr_signal_emit_safe(&device->wlr_input_device.touch->events.up, &event);
}

static void touch_handle_motion(void *data, struct wl_touch *wl_touch,
		uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y) {
	struct wlr_wl_input_device *device = data;
	assert(device && device->wlr_input_device.touch);

	double sx, sy;
	touch_coordinates_to_absolute(device, x, y, &sx, &sy);
	struct wlr_event_touch_motion event = {
		.device = &device->wlr_input_device,
		.time_msec = time,
		.touch_id = id,
		.x = sx,
		.y = sy
	};
	wlr_signal_emit_safe(&device->wlr_input_device.touch->events.motion, &event);
}

static void touch_handle_frame(void *data, struct wl_touch *wl_touch) {
	struct wlr_wl_input_device *device = data;
	assert(device && device->wlr_input_device.touch);

	wlr_signal_emit_safe(&device->wlr_input_device.touch->events.frame, NULL);
}

static void touch_handle_cancel(void *data, struct wl_touch *wl_touch) {
	// no-op
}

static void touch_handle_shape(void *data, struct wl_touch *wl_touch,
		int32_t id, wl_fixed_t major, wl_fixed_t minor) {
	// no-op
}

static void touch_handle_orientation(void *data, struct wl_touch *wl_touch,
		int32_t id, wl_fixed_t orientation) {
	// no-op
}

static const struct wl_touch_listener touch_listener = {
	.down = touch_handle_down,
	.up = touch_handle_up,
	.motion = touch_handle_motion,
	.frame = touch_handle_frame,
	.cancel = touch_handle_cancel,
	.shape = touch_handle_shape,
	.orientation = touch_handle_orientation,
};

static struct wlr_wl_input_device *get_wl_input_device_from_input_device(
		struct wlr_input_device *wlr_dev) {
	assert(wlr_input_device_is_wl(wlr_dev));
	return (struct wlr_wl_input_device *)wlr_dev;
}

bool create_wl_seat(struct wl_seat *wl_seat, struct wlr_wl_backend *wl) {
	struct wlr_wl_seat *seat = calloc(1, sizeof(struct wlr_wl_seat));
	if (!seat) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return false;
	}
	seat->wl_seat = wl_seat;
	seat->backend = wl;
	wl_list_insert(&wl->seats, &seat->link);
	wl_seat_add_listener(wl_seat, &seat_listener, seat);
	return true;
}

void destroy_wl_seats(struct wlr_wl_backend *wl) {
	struct wlr_wl_seat *seat, *tmp_seat;
	wl_list_for_each_safe(seat, tmp_seat, &wl->seats, link) {
		if (seat->touch) {
			wl_touch_destroy(seat->touch);
		}
		if (seat->pointer) {
			wl_pointer_destroy(seat->pointer);
		}
		if (seat->keyboard && !wl->started) {
			// early termination will not be handled by input_device_destroy
			wl_keyboard_destroy(seat->keyboard);
		}
		free(seat->name);
		assert(seat->wl_seat);
		wl_seat_destroy(seat->wl_seat);

		wl_list_remove(&seat->link);
		free(seat);
	}
}

static struct wlr_wl_seat *input_device_get_seat(struct wlr_input_device *wlr_dev) {
	struct wlr_wl_input_device *dev =
		get_wl_input_device_from_input_device(wlr_dev);
	assert(dev->seat);
	return dev->seat;
}

static void input_device_destroy(struct wlr_input_device *wlr_dev) {
	struct wlr_wl_input_device *dev =
		get_wl_input_device_from_input_device(wlr_dev);
	if (dev->wlr_input_device.type == WLR_INPUT_DEVICE_KEYBOARD) {
		struct wlr_wl_seat *seat = input_device_get_seat(wlr_dev);
		wl_keyboard_release(seat->keyboard);
		seat->keyboard = NULL;
	}
	// We can't destroy pointer here because we might have multiple devices
	// exposing it to compositor.
	wl_list_remove(&dev->link);
	free(dev);
}

static const struct wlr_input_device_impl input_device_impl = {
	.destroy = input_device_destroy,
};

bool wlr_input_device_is_wl(struct wlr_input_device *dev) {
	return dev->impl == &input_device_impl;
}

struct wlr_wl_input_device *create_wl_input_device(
		struct wlr_wl_seat *seat, enum wlr_input_device_type type) {
	struct wlr_wl_input_device *dev =
		calloc(1, sizeof(struct wlr_wl_input_device));
	if (dev == NULL) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	dev->backend = seat->backend;
	dev->seat = seat;

	struct wlr_input_device *wlr_dev = &dev->wlr_input_device;

	unsigned int vendor = 0, product = 0;

	const char *type_name = "unknown";

	switch (type) {
	case WLR_INPUT_DEVICE_KEYBOARD:
		type_name = "keyboard";
		break;
	case WLR_INPUT_DEVICE_POINTER:
		type_name = "pointer";
		break;
	case WLR_INPUT_DEVICE_TOUCH:
		type_name = "touch";
		break;
	case WLR_INPUT_DEVICE_TABLET_TOOL:
		type_name = "tablet-tool";
		break;
	case WLR_INPUT_DEVICE_TABLET_PAD:
		type_name = "tablet-pad";
		break;
	case WLR_INPUT_DEVICE_SWITCH:
		type_name = "switch";
		break;
	}

	size_t name_size = 8 + strlen(type_name) + strlen(seat->name) + 1;
	char name[name_size];
	(void) snprintf(name, name_size, "wayland-%s-%s", type_name, seat->name);

	wlr_input_device_init(wlr_dev, type, &input_device_impl, name, vendor,
		product);
	wl_list_insert(&seat->backend->devices, &dev->link);
	return dev;
}

static const struct wlr_pointer_impl pointer_impl;

struct wlr_wl_pointer *pointer_get_wl(struct wlr_pointer *wlr_pointer) {
	assert(wlr_pointer->impl == &pointer_impl);
	return (struct wlr_wl_pointer *)wlr_pointer;
}

static void pointer_destroy(struct wlr_pointer *wlr_pointer) {
	struct wlr_wl_pointer *pointer = pointer_get_wl(wlr_pointer);

	if (pointer->output->cursor.pointer == pointer) {
		pointer->output->cursor.pointer = NULL;
	}

	struct wlr_wl_seat *seat = pointer->input_device->seat;
	if (seat->active_pointer == pointer) {
		seat->active_pointer = NULL;
	}

	// pointer->wl_pointer belongs to the wlr_wl_seat

	if (pointer->gesture_swipe != NULL) {
		zwp_pointer_gesture_swipe_v1_destroy(pointer->gesture_swipe);
	}
	if (pointer->gesture_pinch != NULL) {
		zwp_pointer_gesture_pinch_v1_destroy(pointer->gesture_pinch);
	}
	if (pointer->gesture_hold != NULL) {
		zwp_pointer_gesture_hold_v1_destroy(pointer->gesture_hold);
	}
	if (pointer->relative_pointer != NULL) {
		zwp_relative_pointer_v1_destroy(pointer->relative_pointer);
	}

	wl_list_remove(&pointer->output_destroy.link);
	free(pointer);
}

static const struct wlr_pointer_impl pointer_impl = {
	.destroy = pointer_destroy,
};

static void gesture_swipe_begin(void *data,
		struct zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
		uint32_t serial, uint32_t time,
		struct wl_surface *surface, uint32_t fingers) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_swipe_begin wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.fingers = fingers,
	};
	input_device->fingers = fingers;
	wlr_signal_emit_safe(&wlr_dev->pointer->events.swipe_begin, &wlr_event);
}

static void gesture_swipe_update(void *data,
		struct zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
		uint32_t time, wl_fixed_t dx, wl_fixed_t dy) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_swipe_update wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.fingers = input_device->fingers,
		.dx = wl_fixed_to_double(dx),
		.dy = wl_fixed_to_double(dy),
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.swipe_update, &wlr_event);
}

static void gesture_swipe_end(void *data,
		struct zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
		uint32_t serial, uint32_t time, int32_t cancelled) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_swipe_end wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.cancelled = cancelled,
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.swipe_end, &wlr_event);
}

static const struct zwp_pointer_gesture_swipe_v1_listener gesture_swipe_impl = {
	.begin = gesture_swipe_begin,
	.update = gesture_swipe_update,
	.end = gesture_swipe_end,
};

static void gesture_pinch_begin(void *data,
		struct zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
		uint32_t serial, uint32_t time,
		struct wl_surface *surface, uint32_t fingers) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_pinch_begin wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.fingers = fingers,
	};
	input_device->fingers = fingers;
	wlr_signal_emit_safe(&wlr_dev->pointer->events.pinch_begin, &wlr_event);
}

static void gesture_pinch_update(void *data,
		struct zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
		uint32_t time, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t scale, wl_fixed_t rotation) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_pinch_update wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.fingers = input_device->fingers,
		.dx = wl_fixed_to_double(dx),
		.dy = wl_fixed_to_double(dy),
		.scale = wl_fixed_to_double(scale),
		.rotation = wl_fixed_to_double(rotation),
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.pinch_update, &wlr_event);
}

static void gesture_pinch_end(void *data,
		struct zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
		uint32_t serial, uint32_t time, int32_t cancelled) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_pinch_end wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.cancelled = cancelled,
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.pinch_end, &wlr_event);
}

static const struct zwp_pointer_gesture_pinch_v1_listener gesture_pinch_impl = {
	.begin = gesture_pinch_begin,
	.update = gesture_pinch_update,
	.end = gesture_pinch_end,
};

static void gesture_hold_begin(void *data,
		struct zwp_pointer_gesture_hold_v1 *zwp_pointer_gesture_hold_v1,
		uint32_t serial, uint32_t time,
		struct wl_surface *surface, uint32_t fingers) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_hold_begin wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.fingers = fingers,
	};
	input_device->fingers = fingers;
	wlr_signal_emit_safe(&wlr_dev->pointer->events.hold_begin, &wlr_event);
}

static void gesture_hold_end(void *data,
		struct zwp_pointer_gesture_hold_v1 *zwp_pointer_gesture_hold_v1,
		uint32_t serial, uint32_t time, int32_t cancelled) {
	struct wlr_wl_input_device *input_device = (struct wlr_wl_input_device *)data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	struct wlr_event_pointer_hold_end wlr_event = {
		.device = wlr_dev,
		.time_msec = time,
		.cancelled = cancelled,
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.hold_end, &wlr_event);
}

static const struct zwp_pointer_gesture_hold_v1_listener gesture_hold_impl = {
	.begin = gesture_hold_begin,
	.end = gesture_hold_end,
};

static void relative_pointer_handle_relative_motion(void *data,
		struct zwp_relative_pointer_v1 *relative_pointer, uint32_t utime_hi,
		uint32_t utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel,
		wl_fixed_t dy_unaccel) {
	struct wlr_wl_input_device *input_device = data;
	struct wlr_input_device *wlr_dev = &input_device->wlr_input_device;
	if (pointer_get_wl(wlr_dev->pointer) != input_device->seat->active_pointer) {
		return;
	}

	uint64_t time_usec = (uint64_t)utime_hi << 32 | utime_lo;

	struct wlr_event_pointer_motion wlr_event = {
		.device = wlr_dev,
		.time_msec = (uint32_t)(time_usec / 1000),
		.delta_x = wl_fixed_to_double(dx),
		.delta_y = wl_fixed_to_double(dy),
		.unaccel_dx = wl_fixed_to_double(dx_unaccel),
		.unaccel_dy = wl_fixed_to_double(dy_unaccel),
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.motion, &wlr_event);
}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
	.relative_motion = relative_pointer_handle_relative_motion,
};


static void pointer_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_wl_pointer *pointer =
		wl_container_of(listener, pointer, output_destroy);
	wlr_input_device_destroy(&pointer->input_device->wlr_input_device);
}

void create_wl_pointer(struct wlr_wl_seat *seat, struct wlr_wl_output *output) {
	assert(seat->pointer);
	struct wl_pointer *wl_pointer = seat->pointer;
	struct wlr_wl_backend *backend = output->backend;

	if (output_get_pointer(output, wl_pointer)) {
		wlr_log(WLR_DEBUG,
			"Pointer for seat %s and output %s already exists (ignoring)",
			seat->name, output->wlr_output.name);
		return;
	}

	struct wlr_wl_pointer *pointer = calloc(1, sizeof(struct wlr_wl_pointer));
	if (pointer == NULL) {
		wlr_log(WLR_ERROR, "Allocation failed");
		return;
	}
	pointer->wl_pointer = wl_pointer;
	pointer->output = output;  // we need output to map absolute coordinates onto

	struct wlr_wl_input_device *dev =
		create_wl_input_device(seat, WLR_INPUT_DEVICE_POINTER);
	if (dev == NULL) {
		free(pointer);
		wlr_log(WLR_ERROR, "Allocation failed");
		return;
	}
	pointer->input_device = dev;

	wl_signal_add(&output->wlr_output.events.destroy, &pointer->output_destroy);
	pointer->output_destroy.notify = pointer_handle_output_destroy;

	struct wlr_input_device *wlr_dev = &dev->wlr_input_device;
	wlr_dev->pointer = &pointer->wlr_pointer;
	wlr_dev->output_name = strdup(output->wlr_output.name);
	wlr_pointer_init(wlr_dev->pointer, &pointer_impl);

	if (backend->zwp_pointer_gestures_v1) {
		uint32_t version = zwp_pointer_gestures_v1_get_version(
				backend->zwp_pointer_gestures_v1);

		pointer->gesture_swipe = zwp_pointer_gestures_v1_get_swipe_gesture(
				backend->zwp_pointer_gestures_v1, wl_pointer);
		zwp_pointer_gesture_swipe_v1_add_listener(pointer->gesture_swipe, &gesture_swipe_impl, dev);
		pointer->gesture_pinch = zwp_pointer_gestures_v1_get_pinch_gesture(
				backend->zwp_pointer_gestures_v1, wl_pointer);
		zwp_pointer_gesture_pinch_v1_add_listener(pointer->gesture_pinch, &gesture_pinch_impl, dev);

		if (version >= ZWP_POINTER_GESTURES_V1_GET_HOLD_GESTURE) {
			pointer->gesture_hold = zwp_pointer_gestures_v1_get_hold_gesture(
					backend->zwp_pointer_gestures_v1, wl_pointer);
			zwp_pointer_gesture_hold_v1_add_listener(pointer->gesture_hold, &gesture_hold_impl, dev);
		}
	}

	if (backend->zwp_relative_pointer_manager_v1) {
		pointer->relative_pointer =
			zwp_relative_pointer_manager_v1_get_relative_pointer(
			backend->zwp_relative_pointer_manager_v1, wl_pointer);
		zwp_relative_pointer_v1_add_listener(pointer->relative_pointer,
			&relative_pointer_listener, dev);
	}

	wl_pointer_add_listener(wl_pointer, &pointer_listener, seat);
	wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
}

void create_wl_keyboard(struct wlr_wl_seat *seat) {
	assert(seat->keyboard);
	struct wl_keyboard *wl_keyboard = seat->keyboard;
	struct wlr_wl_input_device *dev =
		create_wl_input_device(seat, WLR_INPUT_DEVICE_KEYBOARD);
	if (!dev) {
		return;
	}

	struct wlr_input_device *wlr_dev = &dev->wlr_input_device;

	wlr_dev->keyboard = calloc(1, sizeof(*wlr_dev->keyboard));
	if (!wlr_dev->keyboard) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		wlr_input_device_destroy(wlr_dev);
		return;
	}
	wlr_keyboard_init(wlr_dev->keyboard, NULL);

	wl_keyboard_add_listener(wl_keyboard, &keyboard_listener, wlr_dev);
	wlr_signal_emit_safe(&seat->backend->backend.events.new_input, wlr_dev);
}

void create_wl_touch(struct wlr_wl_seat *seat) {
	assert(seat->touch);
	struct wl_touch *wl_touch = seat->touch;
	struct wlr_wl_input_device *dev =
		create_wl_input_device(seat, WLR_INPUT_DEVICE_TOUCH);
	if (!dev) {
		return;
	}

	struct wlr_input_device *wlr_dev = &dev->wlr_input_device;

	wlr_dev->touch = calloc(1, sizeof(*wlr_dev->touch));
	if (!wlr_dev->touch) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		wlr_input_device_destroy(wlr_dev);
		return;
	}
	wlr_touch_init(wlr_dev->touch, NULL);

	wl_touch_add_listener(wl_touch, &touch_listener, dev);
	wlr_signal_emit_safe(&seat->backend->backend.events.new_input, wlr_dev);
}


static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
		enum wl_seat_capability caps) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_backend *backend = seat->backend;

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && seat->pointer == NULL) {
		wlr_log(WLR_DEBUG, "seat %p offered pointer", (void *)wl_seat);

		struct wl_pointer *wl_pointer = wl_seat_get_pointer(wl_seat);
		seat->pointer = wl_pointer;

		struct wlr_wl_output *output;
		wl_list_for_each(output, &backend->outputs, link) {
			create_wl_pointer(seat, output);
		}
	}
	if (!(caps & WL_SEAT_CAPABILITY_POINTER) && seat->pointer != NULL) {
		wlr_log(WLR_DEBUG, "seat %p dropped pointer", (void *)wl_seat);

		struct wl_pointer *wl_pointer = seat->pointer;

		struct wlr_wl_input_device *device, *tmp;
		wl_list_for_each_safe(device, tmp, &backend->devices, link) {
			if (device->wlr_input_device.type != WLR_INPUT_DEVICE_POINTER) {
				continue;
			}
			struct wlr_wl_pointer *pointer =
				pointer_get_wl(device->wlr_input_device.pointer);
			if (pointer->wl_pointer != wl_pointer) {
				continue;
			}
			wlr_log(WLR_DEBUG, "dropping pointer %s",
				pointer->input_device->wlr_input_device.name);
			struct wlr_wl_output *output = pointer->output;
			wlr_input_device_destroy(&device->wlr_input_device);
			assert(seat->active_pointer != pointer);
			assert(output->cursor.pointer != pointer);
		}

		wl_pointer_release(seat->pointer);
		seat->pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && seat->keyboard == NULL) {
		wlr_log(WLR_DEBUG, "seat %p offered keyboard", (void *)wl_seat);

		struct wl_keyboard *wl_keyboard = wl_seat_get_keyboard(wl_seat);
		seat->keyboard = wl_keyboard;

		if (backend->started) {
			create_wl_keyboard(seat);
		}
	}
	if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && seat->keyboard != NULL) {
		wlr_log(WLR_DEBUG, "seat %p dropped keyboard", (void *)wl_seat);

		struct wlr_wl_input_device *device, *tmp;
		wl_list_for_each_safe(device, tmp, &backend->devices, link) {
			if (device->wlr_input_device.type != WLR_INPUT_DEVICE_KEYBOARD) {
				continue;
			}

			if (device->seat != seat) {
				continue;
			}
			wlr_input_device_destroy(&device->wlr_input_device);
		}
		assert(seat->keyboard == NULL); // free'ed by input_device_destroy
	}

	if ((caps & WL_SEAT_CAPABILITY_TOUCH) && seat->touch == NULL) {
		wlr_log(WLR_DEBUG, "seat %p offered touch", (void *)wl_seat);

		seat->touch = wl_seat_get_touch(wl_seat);
		if (backend->started) {
			create_wl_touch(seat);
		}
	}
	if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && seat->touch != NULL) {
		wlr_log(WLR_DEBUG, "seat %p dropped touch", (void *)wl_seat);

		struct wlr_wl_input_device *device, *tmp;
		wl_list_for_each_safe(device, tmp, &backend->devices, link) {
			if (device->wlr_input_device.type == WLR_INPUT_DEVICE_TOUCH) {
				wlr_input_device_destroy(&device->wlr_input_device);
			}
		}

		wl_touch_release(seat->touch);
		seat->touch = NULL;
	}
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat,
		const char *name) {
	struct wlr_wl_seat *seat = data;
	free(seat->name);
	seat->name = strdup(name);
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

struct wl_seat *wlr_wl_input_device_get_seat(struct wlr_input_device *wlr_dev) {
	return input_device_get_seat(wlr_dev)->wl_seat;
}
