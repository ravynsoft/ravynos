#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdlib.h>
#include <wayland-client.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/util/log.h>

#include "backend/wayland.h"
#include "util/signal.h"

#include "pointer-gestures-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"

static struct wlr_wl_pointer *output_get_pointer(struct wlr_wl_output *output,
		const struct wl_pointer *wl_pointer) {
	struct wlr_wl_seat *seat;
	wl_list_for_each(seat, &output->backend->seats, link) {
		if (seat->wl_pointer != wl_pointer) {
			continue;
		}

		struct wlr_wl_pointer *pointer;
		wl_list_for_each(pointer, &seat->pointers, link) {
			if (pointer->output == output) {
				return pointer;
			}
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
	struct wlr_wl_pointer *current = output->cursor.pointer;
	if (current && current != pointer) {
		wlr_log(WLR_INFO, "Ignoring seat '%s' pointer in favor of seat '%s'",
			seat->name, current->seat->name);
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
	struct wlr_pointer_motion_absolute_event event = {
		.pointer = &pointer->wlr_pointer,
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

	struct wlr_pointer_button_event event = {
		.pointer = &pointer->wlr_pointer,
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

	struct wlr_pointer_axis_event event = {
		.pointer = &pointer->wlr_pointer,
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

	struct wlr_pointer_axis_event event = {
		.pointer = &pointer->wlr_pointer,
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

static void gesture_swipe_begin(void *data,
		struct zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
		uint32_t serial, uint32_t time, struct wl_surface *surface,
		uint32_t fingers) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	pointer->fingers = fingers;

	struct wlr_pointer_swipe_begin_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.fingers = fingers,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.swipe_begin, &wlr_event);
}

static void gesture_swipe_update(void *data,
		struct zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
		uint32_t time, wl_fixed_t dx, wl_fixed_t dy) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_pointer_swipe_update_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.fingers = pointer->fingers,
		.dx = wl_fixed_to_double(dx),
		.dy = wl_fixed_to_double(dy),
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.swipe_update, &wlr_event);
}

static void gesture_swipe_end(void *data,
		struct zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
		uint32_t serial, uint32_t time, int32_t cancelled) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_pointer_swipe_end_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.cancelled = cancelled,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.swipe_end, &wlr_event);
}

static const struct zwp_pointer_gesture_swipe_v1_listener gesture_swipe_impl = {
	.begin = gesture_swipe_begin,
	.update = gesture_swipe_update,
	.end = gesture_swipe_end,
};

static void gesture_pinch_begin(void *data,
		struct zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
		uint32_t serial, uint32_t time, struct wl_surface *surface,
		uint32_t fingers) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	pointer->fingers = fingers;

	struct wlr_pointer_pinch_begin_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.fingers = pointer->fingers,
	};

	wlr_signal_emit_safe(&pointer->wlr_pointer.events.pinch_begin, &wlr_event);
}

static void gesture_pinch_update(void *data,
		struct zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
		uint32_t time, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t scale,
		wl_fixed_t rotation) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_pointer_pinch_update_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.fingers = pointer->fingers,
		.dx = wl_fixed_to_double(dx),
		.dy = wl_fixed_to_double(dy),
		.scale = wl_fixed_to_double(scale),
		.rotation = wl_fixed_to_double(rotation),
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.pinch_update, &wlr_event);
}

static void gesture_pinch_end(void *data,
		struct zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
		uint32_t serial, uint32_t time, int32_t cancelled) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_pointer_pinch_end_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.cancelled = cancelled,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.pinch_end, &wlr_event);
}

static const struct zwp_pointer_gesture_pinch_v1_listener gesture_pinch_impl = {
	.begin = gesture_pinch_begin,
	.update = gesture_pinch_update,
	.end = gesture_pinch_end,
};

static void gesture_hold_begin(void *data,
		struct zwp_pointer_gesture_hold_v1 *zwp_pointer_gesture_hold_v1,
		uint32_t serial, uint32_t time, struct wl_surface *surface,
		uint32_t fingers) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	pointer->fingers = fingers;

	struct wlr_pointer_hold_begin_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.fingers = fingers,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.hold_begin, &wlr_event);
}

static void gesture_hold_end(void *data,
		struct zwp_pointer_gesture_hold_v1 *zwp_pointer_gesture_hold_v1,
		uint32_t serial, uint32_t time, int32_t cancelled) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	struct wlr_pointer_hold_end_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = time,
		.cancelled = cancelled,
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.hold_end, &wlr_event);
}

static const struct zwp_pointer_gesture_hold_v1_listener gesture_hold_impl = {
	.begin = gesture_hold_begin,
	.end = gesture_hold_end,
};

static void relative_pointer_handle_relative_motion(void *data,
		struct zwp_relative_pointer_v1 *relative_pointer, uint32_t utime_hi,
		uint32_t utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel,
		wl_fixed_t dy_unaccel) {
	struct wlr_wl_seat *seat = data;
	struct wlr_wl_pointer *pointer = seat->active_pointer;
	if (pointer == NULL) {
		return;
	}

	uint64_t time_usec = (uint64_t)utime_hi << 32 | utime_lo;

	struct wlr_pointer_motion_event wlr_event = {
		.pointer = &pointer->wlr_pointer,
		.time_msec = (uint32_t)(time_usec / 1000),
		.delta_x = wl_fixed_to_double(dx),
		.delta_y = wl_fixed_to_double(dy),
		.unaccel_dx = wl_fixed_to_double(dx_unaccel),
		.unaccel_dy = wl_fixed_to_double(dy_unaccel),
	};
	wlr_signal_emit_safe(&pointer->wlr_pointer.events.motion, &wlr_event);
}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
	.relative_motion = relative_pointer_handle_relative_motion,
};

const struct wlr_pointer_impl wl_pointer_impl = {
	.name = "wl-pointer",
};

static void destroy_pointer(struct wlr_wl_pointer *pointer) {
	if (pointer->output->cursor.pointer == pointer) {
		pointer->output->cursor.pointer = NULL;
	}
	if (pointer->seat->active_pointer == pointer) {
		pointer->seat->active_pointer = NULL;
	}

	wlr_pointer_finish(&pointer->wlr_pointer);
	wl_list_remove(&pointer->output_destroy.link);
	wl_list_remove(&pointer->link);
	free(pointer);
}

static void pointer_output_destroy(struct wl_listener *listener, void *data) {
	struct wlr_wl_pointer *pointer =
		wl_container_of(listener, pointer, output_destroy);
	destroy_pointer(pointer);
}

void create_pointer(struct wlr_wl_seat *seat, struct wlr_wl_output *output) {
	assert(seat->wl_pointer);

	if (output_get_pointer(output, seat->wl_pointer)) {
		wlr_log(WLR_DEBUG,
			"pointer for output '%s' from seat '%s' already exists",
			output->wlr_output.name, seat->name);
		return;
	}

	wlr_log(WLR_DEBUG, "creating pointer for output '%s' from seat '%s'",
		output->wlr_output.name, seat->name);

	struct wlr_wl_pointer *pointer = calloc(1, sizeof(struct wlr_wl_pointer));
	if (pointer == NULL) {
		wlr_log(WLR_ERROR, "failed to allocate wlr_wl_pointer");
		return;
	}

	char name[64] = {0};
	snprintf(name, sizeof(name), "wayland-pointer-%s", seat->name);
	wlr_pointer_init(&pointer->wlr_pointer, &wl_pointer_impl, name);

	pointer->wlr_pointer.output_name = strdup(output->wlr_output.name);

	pointer->seat = seat;
	pointer->output = output;

	wl_signal_add(&output->wlr_output.events.destroy, &pointer->output_destroy);
	pointer->output_destroy.notify = pointer_output_destroy;

	wlr_signal_emit_safe(&seat->backend->backend.events.new_input,
		&pointer->wlr_pointer.base);

	wl_list_insert(&seat->pointers, &pointer->link);
}

void init_seat_pointer(struct wlr_wl_seat *seat) {
	assert(seat->wl_pointer);

	struct wlr_wl_backend *backend = seat->backend;

	wl_list_init(&seat->pointers);

	struct wlr_wl_output *output;
	wl_list_for_each(output, &backend->outputs, link) {
		create_pointer(seat, output);
	}

	if (backend->zwp_pointer_gestures_v1) {
		uint32_t version = zwp_pointer_gestures_v1_get_version(
			backend->zwp_pointer_gestures_v1);

		seat->gesture_swipe = zwp_pointer_gestures_v1_get_swipe_gesture(
			backend->zwp_pointer_gestures_v1, seat->wl_pointer);
		zwp_pointer_gesture_swipe_v1_add_listener(seat->gesture_swipe,
			&gesture_swipe_impl, seat);

		seat->gesture_pinch = zwp_pointer_gestures_v1_get_pinch_gesture(
			backend->zwp_pointer_gestures_v1, seat->wl_pointer);
		zwp_pointer_gesture_pinch_v1_add_listener(seat->gesture_pinch,
			&gesture_pinch_impl, seat);

		if (version >= ZWP_POINTER_GESTURES_V1_GET_HOLD_GESTURE) {
			seat->gesture_hold = zwp_pointer_gestures_v1_get_hold_gesture(
				backend->zwp_pointer_gestures_v1, seat->wl_pointer);
			zwp_pointer_gesture_hold_v1_add_listener(seat->gesture_hold,
				&gesture_hold_impl, seat);
		}
	}

	if (backend->zwp_relative_pointer_manager_v1) {
		seat->relative_pointer =
			zwp_relative_pointer_manager_v1_get_relative_pointer(
				backend->zwp_relative_pointer_manager_v1, seat->wl_pointer);
		zwp_relative_pointer_v1_add_listener(seat->relative_pointer,
			&relative_pointer_listener, seat);
	}

	wl_pointer_add_listener(seat->wl_pointer, &pointer_listener, seat);
}

void finish_seat_pointer(struct wlr_wl_seat *seat) {
	assert(seat->wl_pointer);

	wl_pointer_release(seat->wl_pointer);

	struct wlr_wl_pointer *pointer, *tmp;
	wl_list_for_each_safe(pointer, tmp, &seat->pointers, link) {
		destroy_pointer(pointer);
	}

	if (seat->gesture_swipe != NULL) {
		zwp_pointer_gesture_swipe_v1_destroy(seat->gesture_swipe);
	}
	if (seat->gesture_pinch != NULL) {
		zwp_pointer_gesture_pinch_v1_destroy(seat->gesture_pinch);
	}
	if (seat->gesture_hold != NULL) {
		zwp_pointer_gesture_hold_v1_destroy(seat->gesture_hold);
	}
	if (seat->relative_pointer != NULL) {
		zwp_relative_pointer_v1_destroy(seat->relative_pointer);
	}

	seat->wl_pointer = NULL;
	seat->active_pointer = NULL;
}
