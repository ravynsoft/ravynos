#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_virtual_pointer_v1.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "wlr-virtual-pointer-unstable-v1-protocol.h"

static void input_device_destroy(struct wlr_input_device *dev) {
	struct wlr_virtual_pointer_v1 *pointer =
		(struct wlr_virtual_pointer_v1 *)dev;
	wl_resource_set_user_data(pointer->resource, NULL);
	wlr_signal_emit_safe(&pointer->events.destroy, pointer);
	wl_list_remove(&pointer->link);
	free(pointer);
}

static const struct wlr_input_device_impl input_device_impl = {
	.destroy = input_device_destroy
};

static const struct zwlr_virtual_pointer_v1_interface virtual_pointer_impl;

static struct wlr_virtual_pointer_v1 *virtual_pointer_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_virtual_pointer_v1_interface, &virtual_pointer_impl));
	return wl_resource_get_user_data(resource);
}

static void virtual_pointer_motion(struct wl_client *client,
		struct wl_resource *resource, uint32_t time,
		wl_fixed_t dx, wl_fixed_t dy) {
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	struct wlr_event_pointer_motion event = {
		.device = wlr_dev,
		.time_msec = time,
		.delta_x = wl_fixed_to_double(dx),
		.delta_y = wl_fixed_to_double(dy),
		.unaccel_dx = wl_fixed_to_double(dx),
		.unaccel_dy = wl_fixed_to_double(dy),
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.motion, &event);
}

static void virtual_pointer_motion_absolute(struct wl_client *client,
		struct wl_resource *resource, uint32_t time, uint32_t x, uint32_t y,
		uint32_t x_extent, uint32_t y_extent) {
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	if (x_extent == 0 || y_extent == 0) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	struct wlr_event_pointer_motion_absolute event = {
		.device = wlr_dev,
		.time_msec = time,
		.x = (double)x / x_extent,
		.y = (double)y / y_extent,
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.motion_absolute, &event);
}

static void virtual_pointer_button(struct wl_client *client,
		struct wl_resource *resource, uint32_t time, uint32_t button,
		uint32_t state) {
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	struct wlr_event_pointer_button event = {
		.device = wlr_dev,
		.time_msec = time,
		.button = button,
		.state = state ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED
	};
	wlr_signal_emit_safe(&wlr_dev->pointer->events.button, &event);
}

static void virtual_pointer_axis(struct wl_client *client,
		struct wl_resource *resource, uint32_t time, uint32_t axis,
		wl_fixed_t value) {
	if (axis > WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
		wl_resource_post_error(resource,
				ZWLR_VIRTUAL_POINTER_V1_ERROR_INVALID_AXIS,
				"Invalid enumeration value %" PRIu32, axis);
		return;
	}
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	pointer->axis = axis;
	pointer->axis_valid[pointer->axis] = true;
	pointer->axis_event[pointer->axis].device = wlr_dev;
	pointer->axis_event[pointer->axis].time_msec = time;
	pointer->axis_event[pointer->axis].orientation = axis;
	pointer->axis_event[pointer->axis].delta = wl_fixed_to_double(value);
}

static void virtual_pointer_frame(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;

	for (size_t i = 0;
			i < sizeof(pointer->axis_valid) / sizeof(pointer->axis_valid[0]);
			++i) {
		if (pointer->axis_valid[i]) {
			/* Deliver pending axis event */
			wlr_signal_emit_safe(&wlr_dev->pointer->events.axis,
					&pointer->axis_event[i]);
			memset(&pointer->axis_event[i], 0, sizeof(pointer->axis_event[i]));
			pointer->axis_valid[i] = false;
		}
	}

	wlr_signal_emit_safe(&wlr_dev->pointer->events.frame, wlr_dev->pointer);
}

static void virtual_pointer_axis_source(struct wl_client *client,
		struct wl_resource *resource, uint32_t source) {
	if (source > WL_POINTER_AXIS_SOURCE_WHEEL_TILT) {
		wl_resource_post_error(resource,
				ZWLR_VIRTUAL_POINTER_V1_ERROR_INVALID_AXIS_SOURCE,
				"Invalid enumeration value %" PRIu32, source);
		return;
	}
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	pointer->axis_event[pointer->axis].device = wlr_dev;
	pointer->axis_event[pointer->axis].source = source;
}

static void virtual_pointer_axis_stop(struct wl_client *client,
		struct wl_resource *resource, uint32_t time, uint32_t axis) {
	if (axis > WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
		wl_resource_post_error(resource,
				ZWLR_VIRTUAL_POINTER_V1_ERROR_INVALID_AXIS,
				"Invalid enumeration value %" PRIu32, axis);
		return;
	}
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	pointer->axis = axis;
	pointer->axis_valid[pointer->axis] = true;
	pointer->axis_event[pointer->axis].device = wlr_dev;
	pointer->axis_event[pointer->axis].time_msec = time;
	pointer->axis_event[pointer->axis].orientation = axis;
	pointer->axis_event[pointer->axis].delta = 0;
	pointer->axis_event[pointer->axis].delta_discrete = 0;
}

static void virtual_pointer_axis_discrete(struct wl_client *client,
		struct wl_resource *resource, uint32_t time, uint32_t axis,
		wl_fixed_t value, int32_t discrete) {
	if (axis > WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
		wl_resource_post_error(resource,
				ZWLR_VIRTUAL_POINTER_V1_ERROR_INVALID_AXIS,
				"Invalid enumeration value %" PRIu32, axis);
		return;
	}
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer == NULL) {
		return;
	}
	struct wlr_input_device *wlr_dev = &pointer->input_device;
	pointer->axis = axis;
	pointer->axis_valid[pointer->axis] = true;
	pointer->axis_event[pointer->axis].device = wlr_dev;
	pointer->axis_event[pointer->axis].time_msec = time;
	pointer->axis_event[pointer->axis].orientation = axis;
	pointer->axis_event[pointer->axis].delta = wl_fixed_to_double(value);
	pointer->axis_event[pointer->axis].delta_discrete = discrete;
}

static void virtual_pointer_destroy_resource(struct wl_resource *resource) {
	struct wlr_virtual_pointer_v1 *pointer =
		virtual_pointer_from_resource(resource);
	if (pointer != NULL) {
		wlr_input_device_destroy(&pointer->input_device);
	}
}

static void virtual_pointer_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwlr_virtual_pointer_v1_interface virtual_pointer_impl = {
	.motion = virtual_pointer_motion,
	.motion_absolute = virtual_pointer_motion_absolute,
	.button = virtual_pointer_button,
	.axis = virtual_pointer_axis,
	.frame = virtual_pointer_frame,
	.axis_source = virtual_pointer_axis_source,
	.axis_stop = virtual_pointer_axis_stop,
	.axis_discrete = virtual_pointer_axis_discrete,
	.destroy = virtual_pointer_destroy,
};

static const struct zwlr_virtual_pointer_manager_v1_interface manager_impl;

static struct wlr_virtual_pointer_manager_v1 *manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_virtual_pointer_manager_v1_interface, &manager_impl));
	return wl_resource_get_user_data(resource);
}

static void virtual_pointer_manager_create_virtual_pointer_with_output(
		struct wl_client *client, struct wl_resource *resource,
		struct wl_resource *seat, struct wl_resource *output,
		uint32_t id) {
	struct wlr_virtual_pointer_manager_v1 *manager = manager_from_resource(resource);

	struct wlr_virtual_pointer_v1 *virtual_pointer = calloc(1,
		sizeof(struct wlr_virtual_pointer_v1));
	if (!virtual_pointer) {
		wl_client_post_no_memory(client);
		return;
	}

	struct wlr_pointer *pointer = calloc(1, sizeof(struct wlr_pointer));
	if (!pointer) {
		wlr_log(WLR_ERROR, "Cannot allocate wlr_pointer");
		free(virtual_pointer);
		wl_client_post_no_memory(client);
		return;
	}
	wlr_pointer_init(pointer, NULL);

	struct wl_resource *pointer_resource = wl_resource_create(client,
		&zwlr_virtual_pointer_v1_interface, wl_resource_get_version(resource),
		id);
	if (!pointer_resource) {
		free(pointer);
		free(virtual_pointer);
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(pointer_resource, &virtual_pointer_impl,
		virtual_pointer, virtual_pointer_destroy_resource);

	wlr_input_device_init(&virtual_pointer->input_device,
		WLR_INPUT_DEVICE_POINTER, &input_device_impl, "virtual pointer",
		0x0, 0x0);

	struct wlr_virtual_pointer_v1_new_pointer_event event = {
		.new_pointer = virtual_pointer,
	};

	if (seat) {
		struct wlr_seat_client *seat_client =
			wlr_seat_client_from_resource(seat);
		event.suggested_seat = seat_client->seat;
	}

	if (output) {
		struct wlr_output *wlr_output = wlr_output_from_resource(output);
		event.suggested_output = wlr_output;
	}

	virtual_pointer->input_device.pointer = pointer;
	virtual_pointer->resource = pointer_resource;
	wl_signal_init(&virtual_pointer->events.destroy);

	wl_list_insert(&manager->virtual_pointers, &virtual_pointer->link);
	wlr_signal_emit_safe(&manager->events.new_virtual_pointer, &event);
}

static void virtual_pointer_manager_create_virtual_pointer(
		struct wl_client *client, struct wl_resource *resource,
		struct wl_resource *seat, uint32_t id) {
	virtual_pointer_manager_create_virtual_pointer_with_output(client,
			resource, seat, NULL, id);
}
static void virtual_pointer_manager_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwlr_virtual_pointer_manager_v1_interface manager_impl = {
	.create_virtual_pointer = virtual_pointer_manager_create_virtual_pointer,
	.create_virtual_pointer_with_output = virtual_pointer_manager_create_virtual_pointer_with_output,
	.destroy = virtual_pointer_manager_destroy,
};

static void virtual_pointer_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_virtual_pointer_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_virtual_pointer_manager_v1_interface, version, id);

	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource, &manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_virtual_pointer_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	struct wlr_virtual_pointer_v1 *pointer, *pointer_tmp;
	wl_list_for_each_safe(pointer, pointer_tmp,
			&manager->virtual_pointers, link) {
		wl_resource_destroy(pointer->resource);
	}
	free(manager);
}

struct wlr_virtual_pointer_manager_v1* wlr_virtual_pointer_manager_v1_create(
		struct wl_display *display) {
	struct wlr_virtual_pointer_manager_v1 *manager = calloc(1,
		sizeof(struct wlr_virtual_pointer_manager_v1));
	if (!manager) {
		return NULL;
	}

	wl_list_init(&manager->virtual_pointers);

	wl_signal_init(&manager->events.new_virtual_pointer);
	wl_signal_init(&manager->events.destroy);
	manager->global = wl_global_create(display,
		&zwlr_virtual_pointer_manager_v1_interface, 2, manager,
		virtual_pointer_manager_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);
	return manager;
}
