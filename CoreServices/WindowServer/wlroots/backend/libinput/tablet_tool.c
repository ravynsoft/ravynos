#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <string.h>
#include <assert.h>
#include <libinput.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/backend/session.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/array.h"
#include "util/signal.h"

static const struct wlr_tablet_impl tablet_impl;

static bool tablet_is_libinput(struct wlr_tablet *tablet) {
	return tablet->impl == &tablet_impl;
}

struct wlr_libinput_tablet_tool {
	struct wlr_tablet_tool wlr_tool;

	struct libinput_tablet_tool *libinput_tool;

	bool unique;
	// Refcount for destroy + release
	size_t pad_refs;
};

struct wlr_libinput_tablet {
	struct wlr_tablet wlr_tablet;
	struct wl_array tools; // struct wlr_libinput_tablet_tool *
};

static void destroy_tool(struct wlr_libinput_tablet_tool *tool) {
	wlr_signal_emit_safe(&tool->wlr_tool.events.destroy, &tool->wlr_tool);
	libinput_tablet_tool_ref(tool->libinput_tool);
	libinput_tablet_tool_set_user_data(tool->libinput_tool, NULL);
	free(tool);
}


static void destroy_tablet(struct wlr_tablet *wlr_tablet) {
	assert(tablet_is_libinput(wlr_tablet));
	struct wlr_libinput_tablet *tablet =
		wl_container_of(wlr_tablet, tablet, wlr_tablet);

	struct wlr_libinput_tablet_tool **tool_ptr;
	wl_array_for_each(tool_ptr, &tablet->tools) {
		struct wlr_libinput_tablet_tool *tool = *tool_ptr;
		if (--tool->pad_refs == 0) {
			destroy_tool(tool);
		}
	}
	wl_array_release(&tablet->tools);

	free(tablet);
}

static const struct wlr_tablet_impl tablet_impl = {
	.destroy = destroy_tablet,
};

struct wlr_tablet *create_libinput_tablet(
		struct libinput_device *libinput_dev) {
	assert(libinput_dev);
	struct wlr_libinput_tablet *libinput_tablet =
		calloc(1, sizeof(struct wlr_libinput_tablet));
	if (!libinput_tablet) {
		wlr_log(WLR_ERROR, "Unable to allocate wlr_tablet_tool");
		return NULL;
	}

	struct wlr_tablet *wlr_tablet = &libinput_tablet->wlr_tablet;
	wlr_tablet_init(wlr_tablet, &tablet_impl);

	struct udev_device *udev = libinput_device_get_udev_device(libinput_dev);
	char **dst = wl_array_add(&wlr_tablet->paths, sizeof(char *));
	*dst = strdup(udev_device_get_syspath(udev));

	wlr_tablet->name = strdup(libinput_device_get_name(libinput_dev));

	wl_array_init(&libinput_tablet->tools);

	return wlr_tablet;
}

static enum wlr_tablet_tool_type wlr_type_from_libinput_type(
		enum libinput_tablet_tool_type value) {
	switch (value) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:
		return WLR_TABLET_TOOL_TYPE_PEN;
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:
		return WLR_TABLET_TOOL_TYPE_ERASER;
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:
		return WLR_TABLET_TOOL_TYPE_BRUSH;
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:
		return WLR_TABLET_TOOL_TYPE_PENCIL;
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:
		return WLR_TABLET_TOOL_TYPE_AIRBRUSH;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
		return WLR_TABLET_TOOL_TYPE_MOUSE;
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:
		return WLR_TABLET_TOOL_TYPE_LENS;
	case LIBINPUT_TABLET_TOOL_TYPE_TOTEM:
		return WLR_TABLET_TOOL_TYPE_TOTEM;
	}
	abort(); // unreachable
}

static struct wlr_libinput_tablet_tool *get_wlr_tablet_tool(
		struct libinput_tablet_tool *tool) {
	struct wlr_libinput_tablet_tool *ret =
		libinput_tablet_tool_get_user_data(tool);

	if (ret) {
		return ret;
	}

	ret = calloc(1, sizeof(struct wlr_libinput_tablet_tool));
	if (!ret) {
		return NULL;
	}

	ret->libinput_tool = libinput_tablet_tool_ref(tool);
	ret->wlr_tool.pressure = libinput_tablet_tool_has_pressure(tool);
	ret->wlr_tool.distance = libinput_tablet_tool_has_distance(tool);
	ret->wlr_tool.tilt = libinput_tablet_tool_has_tilt(tool);
	ret->wlr_tool.rotation = libinput_tablet_tool_has_rotation(tool);
	ret->wlr_tool.slider = libinput_tablet_tool_has_slider(tool);
	ret->wlr_tool.wheel = libinput_tablet_tool_has_wheel(tool);

	ret->wlr_tool.hardware_serial = libinput_tablet_tool_get_serial(tool);
	ret->wlr_tool.hardware_wacom = libinput_tablet_tool_get_tool_id(tool);
	ret->wlr_tool.type = wlr_type_from_libinput_type(
		libinput_tablet_tool_get_type(tool));

	ret->unique = libinput_tablet_tool_is_unique(tool);

	wl_signal_init(&ret->wlr_tool.events.destroy);

	libinput_tablet_tool_set_user_data(tool, ret);
	return ret;
}

static void ensure_tool_reference(struct wlr_libinput_tablet_tool *tool,
		struct wlr_tablet *wlr_dev) {
	assert(tablet_is_libinput(wlr_dev));
	struct wlr_libinput_tablet *tablet =
		wl_container_of(wlr_dev, tablet, wlr_tablet);

	struct wlr_libinput_tablet_tool **tool_ptr;
	wl_array_for_each(tool_ptr, &tablet->tools) {
		if (*tool_ptr == tool) { // We already have a ref
			// XXX: We *could* optimize the tool to the front of
			// the list here, since we will probably get the next
			// couple of events from the same tool.
			// BUT the list should always be rather short (probably
			// single digit amount of tools) so it might be more
			// work than it saves
			return;
		}
	}

	struct wlr_libinput_tablet_tool **dst =
		wl_array_add(&tablet->tools, sizeof(tool));
	if (!dst) {
		wlr_log(WLR_ERROR, "Failed to allocate memory for tracking tablet tool");
		return;
	}
	*dst = tool;
	++tool->pad_refs;
}

void handle_tablet_tool_axis(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_TOOL, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet tool event for a device with no tablet tools?");
		return;
	}
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_event_tablet_tool_axis wlr_event = { 0 };
	struct wlr_libinput_tablet_tool *tool = get_wlr_tablet_tool(
		libinput_event_tablet_tool_get_tool(tevent));
	ensure_tool_reference(tool, wlr_dev->tablet);

	wlr_event.device = wlr_dev;
	wlr_event.tool = &tool->wlr_tool;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_tool_get_time_usec(tevent));
	if (libinput_event_tablet_tool_x_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_X;
		wlr_event.x = libinput_event_tablet_tool_get_x_transformed(tevent, 1);
		wlr_event.dx = libinput_event_tablet_tool_get_dx(tevent);
	}
	if (libinput_event_tablet_tool_y_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_Y;
		wlr_event.y = libinput_event_tablet_tool_get_y_transformed(tevent, 1);
		wlr_event.dy = libinput_event_tablet_tool_get_dy(tevent);
	}
	if (libinput_event_tablet_tool_pressure_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_PRESSURE;
		wlr_event.pressure = libinput_event_tablet_tool_get_pressure(tevent);
	}
	if (libinput_event_tablet_tool_distance_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_DISTANCE;
		wlr_event.distance = libinput_event_tablet_tool_get_distance(tevent);
	}
	if (libinput_event_tablet_tool_tilt_x_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_TILT_X;
		wlr_event.tilt_x = libinput_event_tablet_tool_get_tilt_x(tevent);
	}
	if (libinput_event_tablet_tool_tilt_y_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_TILT_Y;
		wlr_event.tilt_y = libinput_event_tablet_tool_get_tilt_y(tevent);
	}
	if (libinput_event_tablet_tool_rotation_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_ROTATION;
		wlr_event.rotation = libinput_event_tablet_tool_get_rotation(tevent);
	}
	if (libinput_event_tablet_tool_slider_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_SLIDER;
		wlr_event.slider = libinput_event_tablet_tool_get_slider_position(tevent);
	}
	if (libinput_event_tablet_tool_wheel_has_changed(tevent)) {
		wlr_event.updated_axes |= WLR_TABLET_TOOL_AXIS_WHEEL;
		wlr_event.wheel_delta = libinput_event_tablet_tool_get_wheel_delta(tevent);
	}
	wlr_signal_emit_safe(&wlr_dev->tablet->events.axis, &wlr_event);
}

void handle_tablet_tool_proximity(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_TOOL, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet tool event for a device with no tablet tools?");
		return;
	}
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_event_tablet_tool_proximity wlr_event = { 0 };
	struct wlr_libinput_tablet_tool *tool = get_wlr_tablet_tool(
		libinput_event_tablet_tool_get_tool(tevent));
	ensure_tool_reference(tool, wlr_dev->tablet);

	wlr_event.tool = &tool->wlr_tool;
	wlr_event.device = wlr_dev;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_tool_get_time_usec(tevent));
	wlr_event.x = libinput_event_tablet_tool_get_x_transformed(tevent, 1);
	wlr_event.y = libinput_event_tablet_tool_get_y_transformed(tevent, 1);

	switch (libinput_event_tablet_tool_get_proximity_state(tevent)) {
	case LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT:
		wlr_event.state = WLR_TABLET_TOOL_PROXIMITY_OUT;
		break;
	case LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN:
		wlr_event.state = WLR_TABLET_TOOL_PROXIMITY_IN;
		break;
	}
	wlr_signal_emit_safe(&wlr_dev->tablet->events.proximity, &wlr_event);

	if (libinput_event_tablet_tool_get_proximity_state(tevent) ==
			LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN) {
		handle_tablet_tool_axis(event, libinput_dev);
	}

	// If the tool is not unique, libinput will not find it again after the
	// proximity out, so we should destroy it
	if (!tool->unique &&
			libinput_event_tablet_tool_get_proximity_state(tevent) ==
			LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT) {
		// The tool isn't unique, it can't be on multiple tablets
		assert(tool->pad_refs == 1);
		assert(tablet_is_libinput(wlr_dev->tablet));
		struct wlr_libinput_tablet *tablet =
			wl_container_of(wlr_dev->tablet, tablet, wlr_tablet);

		size_t i = 0;
		struct wlr_libinput_tablet_tool **tool_ptr;
		wl_array_for_each(tool_ptr, &tablet->tools) {
			if (*tool_ptr == tool) {
				array_remove_at(&tablet->tools, i * sizeof(tool), sizeof(tool));
				break;
			}
			i++;
		}

		destroy_tool(tool);
	}
}

void handle_tablet_tool_tip(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_TOOL, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet tool event for a device with no tablet tools?");
		return;
	}
	handle_tablet_tool_axis(event, libinput_dev);
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_event_tablet_tool_tip wlr_event = { 0 };
	struct wlr_libinput_tablet_tool *tool = get_wlr_tablet_tool(
		libinput_event_tablet_tool_get_tool(tevent));
	ensure_tool_reference(tool, wlr_dev->tablet);

	wlr_event.device = wlr_dev;
	wlr_event.tool = &tool->wlr_tool;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_tool_get_time_usec(tevent));
	wlr_event.x = libinput_event_tablet_tool_get_x_transformed(tevent, 1);
	wlr_event.y = libinput_event_tablet_tool_get_y_transformed(tevent, 1);

	switch (libinput_event_tablet_tool_get_tip_state(tevent)) {
	case LIBINPUT_TABLET_TOOL_TIP_UP:
		wlr_event.state = WLR_TABLET_TOOL_TIP_UP;
		break;
	case LIBINPUT_TABLET_TOOL_TIP_DOWN:
		wlr_event.state = WLR_TABLET_TOOL_TIP_DOWN;
		break;
	}
	wlr_signal_emit_safe(&wlr_dev->tablet->events.tip, &wlr_event);
}

void handle_tablet_tool_button(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_TOOL, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet tool event for a device with no tablet tools?");
		return;
	}
	handle_tablet_tool_axis(event, libinput_dev);
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_event_tablet_tool_button wlr_event = { 0 };
	struct wlr_libinput_tablet_tool *tool = get_wlr_tablet_tool(
		libinput_event_tablet_tool_get_tool(tevent));
	ensure_tool_reference(tool, wlr_dev->tablet);

	wlr_event.device = wlr_dev;
	wlr_event.tool = &tool->wlr_tool;
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_tool_get_time_usec(tevent));
	wlr_event.button = libinput_event_tablet_tool_get_button(tevent);
	switch (libinput_event_tablet_tool_get_button_state(tevent)) {
	case LIBINPUT_BUTTON_STATE_RELEASED:
		wlr_event.state = WLR_BUTTON_RELEASED;
		break;
	case LIBINPUT_BUTTON_STATE_PRESSED:
		wlr_event.state = WLR_BUTTON_PRESSED;
		break;
	}
	wlr_signal_emit_safe(&wlr_dev->tablet->events.button, &wlr_event);
}
