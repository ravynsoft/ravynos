#include <stdlib.h>

#include <wlr/config.h>

#include <linux/input-event-codes.h>

#include <wayland-server-protocol.h>

#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <xcb/xinput.h>

#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/util/log.h>

#include "backend/x11.h"
#include "util/signal.h"

static void send_key_event(struct wlr_x11_backend *x11, uint32_t key,
		enum wl_keyboard_key_state st, xcb_timestamp_t time) {
	struct wlr_event_keyboard_key ev = {
		.time_msec = time,
		.keycode = key,
		.state = st,
		.update_state = true,
	};
	wlr_keyboard_notify_key(&x11->keyboard, &ev);
}

static void send_button_event(struct wlr_x11_output *output, uint32_t key,
		enum wlr_button_state st, xcb_timestamp_t time) {
	struct wlr_event_pointer_button ev = {
		.device = &output->pointer.base,
		.time_msec = time,
		.button = key,
		.state = st,
	};
	wlr_signal_emit_safe(&output->pointer.events.button, &ev);
	wlr_signal_emit_safe(&output->pointer.events.frame, &output->pointer);
}

static void send_axis_event(struct wlr_x11_output *output, int32_t delta,
		xcb_timestamp_t time) {
	struct wlr_event_pointer_axis ev = {
		.device = &output->pointer.base,
		.time_msec = time,
		.source = WLR_AXIS_SOURCE_WHEEL,
		.orientation = WLR_AXIS_ORIENTATION_VERTICAL,
		// 15 is a typical value libinput sends for one scroll
		.delta = delta * 15,
		.delta_discrete = delta,
	};
	wlr_signal_emit_safe(&output->pointer.events.axis, &ev);
	wlr_signal_emit_safe(&output->pointer.events.frame, &output->pointer);
}

static void send_pointer_position_event(struct wlr_x11_output *output,
		int16_t x, int16_t y, xcb_timestamp_t time) {
	struct wlr_event_pointer_motion_absolute ev = {
		.device = &output->pointer.base,
		.time_msec = time,
		.x = (double)x / output->wlr_output.width,
		.y = (double)y / output->wlr_output.height,
	};
	wlr_signal_emit_safe(&output->pointer.events.motion_absolute, &ev);
	wlr_signal_emit_safe(&output->pointer.events.frame, &output->pointer);
}

static void send_touch_down_event(struct wlr_x11_output *output,
		int16_t x, int16_t y, int32_t touch_id, xcb_timestamp_t time) {
	struct wlr_event_touch_down ev = {
		.device = &output->touch.base,
		.time_msec = time,
		.x = (double)x / output->wlr_output.width,
		.y = (double)y / output->wlr_output.height,
		.touch_id = touch_id,
	};
	wlr_signal_emit_safe(&output->touch.events.down, &ev);
	wlr_signal_emit_safe(&output->touch.events.frame, NULL);
}

static void send_touch_motion_event(struct wlr_x11_output *output,
		int16_t x, int16_t y, int32_t touch_id, xcb_timestamp_t time) {
	struct wlr_event_touch_motion ev = {
		.device = &output->touch.base,
		.time_msec = time,
		.x = (double)x / output->wlr_output.width,
		.y = (double)y / output->wlr_output.height,
		.touch_id = touch_id,
	};
	wlr_signal_emit_safe(&output->touch.events.motion, &ev);
	wlr_signal_emit_safe(&output->touch.events.frame, NULL);
}

static void send_touch_up_event(struct wlr_x11_output *output,
		int32_t touch_id, xcb_timestamp_t time) {
	struct wlr_event_touch_up ev = {
		.device = &output->touch.base,
		.time_msec = time,
		.touch_id = touch_id,
	};
	wlr_signal_emit_safe(&output->touch.events.up, &ev);
	wlr_signal_emit_safe(&output->touch.events.frame, NULL);
}

static struct wlr_x11_touchpoint *get_touchpoint_from_x11_touch_id(
		struct wlr_x11_output *output, uint32_t id) {
	struct wlr_x11_touchpoint *touchpoint;
	wl_list_for_each(touchpoint, &output->touchpoints, link) {
		if (touchpoint->x11_id == id) {
			return touchpoint;
		}
	}
	return NULL;
}

void handle_x11_xinput_event(struct wlr_x11_backend *x11,
		xcb_ge_generic_event_t *event) {
	struct wlr_x11_output *output;

	switch (event->event_type) {
	case XCB_INPUT_KEY_PRESS: {
		xcb_input_key_press_event_t *ev =
			(xcb_input_key_press_event_t *)event;

		if (ev->flags & XCB_INPUT_KEY_EVENT_FLAGS_KEY_REPEAT) {
			return;
		}

		wlr_keyboard_notify_modifiers(&x11->keyboard, ev->mods.base,
			ev->mods.latched, ev->mods.locked, ev->mods.effective);
		send_key_event(x11, ev->detail - 8, WL_KEYBOARD_KEY_STATE_PRESSED, ev->time);
		x11->time = ev->time;
		break;
	}
	case XCB_INPUT_KEY_RELEASE: {
		xcb_input_key_release_event_t *ev =
			(xcb_input_key_release_event_t *)event;

		wlr_keyboard_notify_modifiers(&x11->keyboard, ev->mods.base,
			ev->mods.latched, ev->mods.locked, ev->mods.effective);
		send_key_event(x11, ev->detail - 8, WL_KEYBOARD_KEY_STATE_RELEASED, ev->time);
		x11->time = ev->time;
		break;
	}
	case XCB_INPUT_BUTTON_PRESS: {
		xcb_input_button_press_event_t *ev =
			(xcb_input_button_press_event_t *)event;

		output = get_x11_output_from_window_id(x11, ev->event);
		if (!output) {
			return;
		}

		switch (ev->detail) {
		case XCB_BUTTON_INDEX_1:
			send_button_event(output, BTN_LEFT, WLR_BUTTON_PRESSED,
				ev->time);
			break;
		case XCB_BUTTON_INDEX_2:
			send_button_event(output, BTN_MIDDLE, WLR_BUTTON_PRESSED,
				ev->time);
			break;
		case XCB_BUTTON_INDEX_3:
			send_button_event(output, BTN_RIGHT, WLR_BUTTON_PRESSED,
				ev->time);
			break;
		case XCB_BUTTON_INDEX_4:
			send_axis_event(output, -1, ev->time);
			break;
		case XCB_BUTTON_INDEX_5:
			send_axis_event(output, 1, ev->time);
			break;
		}

		x11->time = ev->time;
		break;
	}
	case XCB_INPUT_BUTTON_RELEASE: {
		xcb_input_button_release_event_t *ev =
			(xcb_input_button_release_event_t *)event;

		output = get_x11_output_from_window_id(x11, ev->event);
		if (!output) {
			return;
		}

		switch (ev->detail) {
		case XCB_BUTTON_INDEX_1:
			send_button_event(output, BTN_LEFT, WLR_BUTTON_RELEASED,
				ev->time);
			break;
		case XCB_BUTTON_INDEX_2:
			send_button_event(output, BTN_MIDDLE, WLR_BUTTON_RELEASED,
				ev->time);
			break;
		case XCB_BUTTON_INDEX_3:
			send_button_event(output, BTN_RIGHT, WLR_BUTTON_RELEASED,
				ev->time);
			break;
		}

		x11->time = ev->time;
		break;
	}
	case XCB_INPUT_MOTION: {
		xcb_input_motion_event_t *ev = (xcb_input_motion_event_t *)event;

		output = get_x11_output_from_window_id(x11, ev->event);
		if (!output) {
			return;
		}

		send_pointer_position_event(output, ev->event_x >> 16,
			ev->event_y >> 16, ev->time);
		x11->time = ev->time;
		break;
	}
	case XCB_INPUT_TOUCH_BEGIN: {
		xcb_input_touch_begin_event_t *ev = (xcb_input_touch_begin_event_t *)event;

		output = get_x11_output_from_window_id(x11, ev->event);
		if (!output) {
			return;
		}

		int32_t id = 0;
		if (!wl_list_empty(&output->touchpoints)) {
			struct wlr_x11_touchpoint *last_touchpoint = wl_container_of(
				output->touchpoints.next, last_touchpoint, link);
			id = last_touchpoint->wayland_id + 1;
		}

		struct wlr_x11_touchpoint *touchpoint = calloc(1, sizeof(struct wlr_x11_touchpoint));
		touchpoint->x11_id = ev->detail;
		touchpoint->wayland_id = id;
		wl_list_init(&touchpoint->link);
		wl_list_insert(&output->touchpoints, &touchpoint->link);

		send_touch_down_event(output, ev->event_x >> 16,
			ev->event_y >> 16, touchpoint->wayland_id, ev->time);
		x11->time = ev->time;
		break;
	}
	case XCB_INPUT_TOUCH_END: {
		xcb_input_touch_end_event_t *ev = (xcb_input_touch_end_event_t *)event;

		output = get_x11_output_from_window_id(x11, ev->event);
		if (!output) {
			return;
		}

		struct wlr_x11_touchpoint *touchpoint = get_touchpoint_from_x11_touch_id(output, ev->detail);
		if (!touchpoint) {
			return;
		}

		send_touch_up_event(output, touchpoint->wayland_id, ev->time);
		x11->time = ev->time;

		wl_list_remove(&touchpoint->link);
		free(touchpoint);
		break;
	}
	case XCB_INPUT_TOUCH_UPDATE: {
		xcb_input_touch_update_event_t *ev = (xcb_input_touch_update_event_t *)event;

		output = get_x11_output_from_window_id(x11, ev->event);
		if (!output) {
			return;
		}

		struct wlr_x11_touchpoint *touchpoint = get_touchpoint_from_x11_touch_id(output, ev->detail);
		if (!touchpoint) {
			return;
		}

		send_touch_motion_event(output, ev->event_x >> 16,
			ev->event_y >> 16, touchpoint->wayland_id, ev->time);
		x11->time = ev->time;
		break;
	}
	}
}

const struct wlr_keyboard_impl x11_keyboard_impl = {
	.name = "x11-keyboard",
};

const struct wlr_pointer_impl x11_pointer_impl = {
	.name = "x11-pointer",
};

const struct wlr_touch_impl x11_touch_impl = {
	.name = "x11-touch",
};

void update_x11_pointer_position(struct wlr_x11_output *output,
		xcb_timestamp_t time) {
	struct wlr_x11_backend *x11 = output->x11;

	xcb_query_pointer_cookie_t cookie =
		xcb_query_pointer(x11->xcb, output->win);
	xcb_query_pointer_reply_t *reply =
		xcb_query_pointer_reply(x11->xcb, cookie, NULL);
	if (!reply) {
		return;
	}

	send_pointer_position_event(output, reply->win_x, reply->win_y, time);

	free(reply);
}

bool wlr_input_device_is_x11(struct wlr_input_device *wlr_dev) {
	switch (wlr_dev->type) {
	case WLR_INPUT_DEVICE_KEYBOARD:
		return wlr_dev->keyboard->impl == &x11_keyboard_impl;
	case WLR_INPUT_DEVICE_POINTER:
		return wlr_dev->pointer->impl == &x11_pointer_impl;
	case WLR_INPUT_DEVICE_TOUCH:
		return wlr_dev->touch->impl == &x11_touch_impl;
	default:
		return false;
	}
}
