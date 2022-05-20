#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <assert.h>
#include <libinput.h>
#include <stdlib.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/signal.h"

struct tablet_tool {
	struct wlr_tablet_tool wlr_tool;
	struct libinput_tablet_tool *handle;
	struct wl_list link; // wlr_libinput_input_device::tablet_tools
};

const struct wlr_tablet_impl libinput_tablet_impl = {
	.name = "libinput-tablet-tool",
};

void init_device_tablet(struct wlr_libinput_input_device *dev) {
	const char *name = libinput_device_get_name(dev->handle);
	struct wlr_tablet *wlr_tablet = &dev->tablet;
	wlr_tablet_init(wlr_tablet, &libinput_tablet_impl, name);
	wlr_tablet->base.vendor = libinput_device_get_id_vendor(dev->handle);
	wlr_tablet->base.product = libinput_device_get_id_product(dev->handle);

	struct udev_device *udev = libinput_device_get_udev_device(dev->handle);
	char **dst = wl_array_add(&wlr_tablet->paths, sizeof(char *));
	*dst = strdup(udev_device_get_syspath(udev));

	wl_list_init(&dev->tablet_tools);
}

static void tool_destroy(struct tablet_tool *tool) {
	wlr_signal_emit_safe(&tool->wlr_tool.events.destroy, &tool->wlr_tool);
	libinput_tablet_tool_unref(tool->handle);
	libinput_tablet_tool_set_user_data(tool->handle, NULL);
	wl_list_remove(&tool->link);
	free(tool);
}

void finish_device_tablet(struct wlr_libinput_input_device *dev) {
	struct tablet_tool *tool, *tmp;
	wl_list_for_each_safe(tool, tmp, &dev->tablet_tools, link) {
		tool_destroy(tool);
	}

	wlr_tablet_finish(&dev->tablet);
}

struct wlr_libinput_input_device *device_from_tablet(
		struct wlr_tablet *wlr_tablet) {
	assert(wlr_tablet->impl == &libinput_tablet_impl);

	struct wlr_libinput_input_device *dev =
		wl_container_of(wlr_tablet, dev, tablet);
	return dev;
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

static struct tablet_tool *get_tablet_tool(
		struct wlr_libinput_input_device *dev,
		struct libinput_tablet_tool *libinput_tool) {
	struct tablet_tool *tool =
		libinput_tablet_tool_get_user_data(libinput_tool);
	if (tool) {
		return tool;
	}

	tool = calloc(1, sizeof(struct tablet_tool));
	if (tool == NULL) {
		wlr_log_errno(WLR_ERROR, "failed to allocate wlr_libinput_tablet_tool");
		return NULL;
	}

	tool->wlr_tool.type = wlr_type_from_libinput_type(
		libinput_tablet_tool_get_type(libinput_tool));
	tool->wlr_tool.hardware_serial =
		libinput_tablet_tool_get_serial(libinput_tool);
	tool->wlr_tool.hardware_wacom =
		libinput_tablet_tool_get_tool_id(libinput_tool);

	tool->wlr_tool.pressure = libinput_tablet_tool_has_pressure(libinput_tool);
	tool->wlr_tool.distance = libinput_tablet_tool_has_distance(libinput_tool);
	tool->wlr_tool.tilt = libinput_tablet_tool_has_tilt(libinput_tool);
	tool->wlr_tool.rotation = libinput_tablet_tool_has_rotation(libinput_tool);
	tool->wlr_tool.slider = libinput_tablet_tool_has_slider(libinput_tool);
	tool->wlr_tool.wheel = libinput_tablet_tool_has_wheel(libinput_tool);

	wl_signal_init(&tool->wlr_tool.events.destroy);

	tool->handle = libinput_tablet_tool_ref(libinput_tool);
	libinput_tablet_tool_set_user_data(libinput_tool, tool);

	wl_list_insert(&dev->tablet_tools, &tool->link);
	return tool;
}

void handle_tablet_tool_axis(struct libinput_event *event,
		struct wlr_tablet *wlr_tablet) {
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_libinput_input_device *dev = device_from_tablet(wlr_tablet);
	struct tablet_tool *tool =
		get_tablet_tool(dev, libinput_event_tablet_tool_get_tool(tevent));

	struct wlr_event_tablet_tool_axis wlr_event = { 0 };

	wlr_event.device = &wlr_tablet->base;
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
	wlr_signal_emit_safe(&wlr_tablet->events.axis, &wlr_event);
}

void handle_tablet_tool_proximity(struct libinput_event *event,
		struct wlr_tablet *wlr_tablet) {
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_libinput_input_device *dev = device_from_tablet(wlr_tablet);
	struct tablet_tool *tool =
		get_tablet_tool(dev, libinput_event_tablet_tool_get_tool(tevent));

	struct wlr_event_tablet_tool_proximity wlr_event = { 0 };
	wlr_event.tool = &tool->wlr_tool;
	wlr_event.device = &wlr_tablet->base;
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
	wlr_signal_emit_safe(&wlr_tablet->events.proximity, &wlr_event);

	if (libinput_event_tablet_tool_get_proximity_state(tevent) ==
			LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN) {
		handle_tablet_tool_axis(event, wlr_tablet);
	}

	// If the tool is not unique, libinput will not find it again after the
	// proximity out, so we should destroy it
	if (!libinput_tablet_tool_is_unique(tool->handle)
			&& libinput_event_tablet_tool_get_proximity_state(tevent) ==
			LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT) {
		// The tool isn't unique, it can't be on multiple tablets
		tool_destroy(tool);
	}
}

void handle_tablet_tool_tip(struct libinput_event *event,
		struct wlr_tablet *wlr_tablet) {
	handle_tablet_tool_axis(event, wlr_tablet);
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_libinput_input_device *dev = device_from_tablet(wlr_tablet);
	struct tablet_tool *tool =
		get_tablet_tool(dev, libinput_event_tablet_tool_get_tool(tevent));

	struct wlr_event_tablet_tool_tip wlr_event = { 0 };
	wlr_event.device = &wlr_tablet->base;
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
	wlr_signal_emit_safe(&wlr_tablet->events.tip, &wlr_event);
}

void handle_tablet_tool_button(struct libinput_event *event,
		struct wlr_tablet *wlr_tablet) {
	handle_tablet_tool_axis(event, wlr_tablet);
	struct libinput_event_tablet_tool *tevent =
		libinput_event_get_tablet_tool_event(event);
	struct wlr_libinput_input_device *dev = device_from_tablet(wlr_tablet);
	struct tablet_tool *tool =
		get_tablet_tool(dev, libinput_event_tablet_tool_get_tool(tevent));

	struct wlr_event_tablet_tool_button wlr_event = { 0 };
	wlr_event.device = &wlr_tablet->base;
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
	wlr_signal_emit_safe(&wlr_tablet->events.button, &wlr_event);
}
