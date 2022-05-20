#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_touch.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "util/signal.h"

struct wlr_cursor_device {
	struct wlr_cursor *cursor;
	struct wlr_input_device *device;
	struct wl_list link;
	struct wlr_output *mapped_output;
	struct wlr_box *mapped_box;

	struct wl_listener motion;
	struct wl_listener motion_absolute;
	struct wl_listener button;
	struct wl_listener axis;
	struct wl_listener frame;
	struct wl_listener swipe_begin;
	struct wl_listener swipe_update;
	struct wl_listener swipe_end;
	struct wl_listener pinch_begin;
	struct wl_listener pinch_update;
	struct wl_listener pinch_end;
	struct wl_listener hold_begin;
	struct wl_listener hold_end;

	struct wl_listener touch_down;
	struct wl_listener touch_up;
	struct wl_listener touch_motion;
	struct wl_listener touch_cancel;
	struct wl_listener touch_frame;

	struct wl_listener tablet_tool_axis;
	struct wl_listener tablet_tool_proximity;
	struct wl_listener tablet_tool_tip;
	struct wl_listener tablet_tool_button;

	struct wl_listener destroy;
};

struct wlr_cursor_output_cursor {
	struct wlr_cursor *cursor;
	struct wlr_output_cursor *output_cursor;
	struct wl_list link;

	struct wl_listener layout_output_destroy;
};

struct wlr_cursor_state {
	struct wlr_cursor *cursor;
	struct wl_list devices; // wlr_cursor_device::link
	struct wl_list output_cursors; // wlr_cursor_output_cursor::link
	struct wlr_output_layout *layout;
	struct wlr_output *mapped_output;
	struct wlr_box *mapped_box;

	struct wl_listener layout_add;
	struct wl_listener layout_change;
	struct wl_listener layout_destroy;
};

struct wlr_cursor *wlr_cursor_create(void) {
	struct wlr_cursor *cur = calloc(1, sizeof(struct wlr_cursor));
	if (!cur) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_cursor");
		return NULL;
	}

	cur->state = calloc(1, sizeof(struct wlr_cursor_state));
	if (!cur->state) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_cursor_state");
		free(cur);
		return NULL;
	}

	cur->state->cursor = cur;
	cur->state->mapped_output = NULL;

	wl_list_init(&cur->state->devices);
	wl_list_init(&cur->state->output_cursors);

	// pointer signals
	wl_signal_init(&cur->events.motion);
	wl_signal_init(&cur->events.motion_absolute);
	wl_signal_init(&cur->events.button);
	wl_signal_init(&cur->events.axis);
	wl_signal_init(&cur->events.frame);
	wl_signal_init(&cur->events.swipe_begin);
	wl_signal_init(&cur->events.swipe_update);
	wl_signal_init(&cur->events.swipe_end);
	wl_signal_init(&cur->events.pinch_begin);
	wl_signal_init(&cur->events.pinch_update);
	wl_signal_init(&cur->events.pinch_end);
	wl_signal_init(&cur->events.hold_begin);
	wl_signal_init(&cur->events.hold_end);

	// touch signals
	wl_signal_init(&cur->events.touch_up);
	wl_signal_init(&cur->events.touch_down);
	wl_signal_init(&cur->events.touch_motion);
	wl_signal_init(&cur->events.touch_cancel);
	wl_signal_init(&cur->events.touch_frame);

	// tablet tool signals
	wl_signal_init(&cur->events.tablet_tool_tip);
	wl_signal_init(&cur->events.tablet_tool_axis);
	wl_signal_init(&cur->events.tablet_tool_button);
	wl_signal_init(&cur->events.tablet_tool_proximity);

	cur->x = 100;
	cur->y = 100;

	return cur;
}

static void output_cursor_destroy(
		struct wlr_cursor_output_cursor *output_cursor) {
	wl_list_remove(&output_cursor->layout_output_destroy.link);
	wl_list_remove(&output_cursor->link);
	wlr_output_cursor_destroy(output_cursor->output_cursor);
	free(output_cursor);
}

static void cursor_detach_output_layout(struct wlr_cursor *cur) {
	if (!cur->state->layout) {
		return;
	}

	struct wlr_cursor_output_cursor *output_cursor, *tmp;
	wl_list_for_each_safe(output_cursor, tmp, &cur->state->output_cursors,
			link) {
		output_cursor_destroy(output_cursor);
	}

	wl_list_remove(&cur->state->layout_destroy.link);
	wl_list_remove(&cur->state->layout_change.link);
	wl_list_remove(&cur->state->layout_add.link);

	cur->state->layout = NULL;
}

static void cursor_device_destroy(struct wlr_cursor_device *c_device) {
	struct wlr_input_device *dev = c_device->device;
	if (dev->type == WLR_INPUT_DEVICE_POINTER) {
		wl_list_remove(&c_device->motion.link);
		wl_list_remove(&c_device->motion_absolute.link);
		wl_list_remove(&c_device->button.link);
		wl_list_remove(&c_device->axis.link);
		wl_list_remove(&c_device->frame.link);
		wl_list_remove(&c_device->swipe_begin.link);
		wl_list_remove(&c_device->swipe_update.link);
		wl_list_remove(&c_device->swipe_end.link);
		wl_list_remove(&c_device->pinch_begin.link);
		wl_list_remove(&c_device->pinch_update.link);
		wl_list_remove(&c_device->pinch_end.link);
		wl_list_remove(&c_device->hold_begin.link);
		wl_list_remove(&c_device->hold_end.link);
	} else if (dev->type == WLR_INPUT_DEVICE_TOUCH) {
		wl_list_remove(&c_device->touch_down.link);
		wl_list_remove(&c_device->touch_up.link);
		wl_list_remove(&c_device->touch_motion.link);
		wl_list_remove(&c_device->touch_cancel.link);
		wl_list_remove(&c_device->touch_frame.link);
	} else if (dev->type == WLR_INPUT_DEVICE_TABLET_TOOL) {
		wl_list_remove(&c_device->tablet_tool_axis.link);
		wl_list_remove(&c_device->tablet_tool_proximity.link);
		wl_list_remove(&c_device->tablet_tool_tip.link);
		wl_list_remove(&c_device->tablet_tool_button.link);
	}

	wl_list_remove(&c_device->link);
	wl_list_remove(&c_device->destroy.link);
	free(c_device);
}

void wlr_cursor_destroy(struct wlr_cursor *cur) {
	cursor_detach_output_layout(cur);

	struct wlr_cursor_device *device, *device_tmp = NULL;
	wl_list_for_each_safe(device, device_tmp, &cur->state->devices, link) {
		cursor_device_destroy(device);
	}

	free(cur->state);
	free(cur);
}

static struct wlr_cursor_device *get_cursor_device(struct wlr_cursor *cur,
		struct wlr_input_device *device) {
	struct wlr_cursor_device *c_device, *ret = NULL;
	wl_list_for_each(c_device, &cur->state->devices, link) {
		if (c_device->device == device) {
			ret = c_device;
			break;
		}
	}

	return ret;
}

static void cursor_warp_unchecked(struct wlr_cursor *cur,
		double lx, double ly) {
	assert(cur->state->layout);

	struct wlr_cursor_output_cursor *output_cursor;
	wl_list_for_each(output_cursor, &cur->state->output_cursors, link) {
		double output_x = lx, output_y = ly;
		wlr_output_layout_output_coords(cur->state->layout,
			output_cursor->output_cursor->output, &output_x, &output_y);
		wlr_output_cursor_move(output_cursor->output_cursor,
			output_x, output_y);
	}

	cur->x = lx;
	cur->y = ly;
}

/**
 * Get the most specific mapping box for the device in this order:
 *
 * 1. device geometry mapping
 * 2. device output mapping
 * 3. cursor geometry mapping
 * 4. cursor output mapping
 *
 * Absolute movement for touch and pen devices will be relative to this box and
 * pointer movement will be constrained to this box.
 *
 * If none of these are set, returns NULL and absolute movement should be
 * relative to the extents of the layout.
 */
static struct wlr_box *get_mapping(struct wlr_cursor *cur,
		struct wlr_input_device *dev) {
	assert(cur->state->layout);
	struct wlr_cursor_device *c_device = get_cursor_device(cur, dev);

	if (c_device) {
		if (c_device->mapped_box) {
			return c_device->mapped_box;
		}
		if (c_device->mapped_output) {
			return wlr_output_layout_get_box(cur->state->layout,
				c_device->mapped_output);
		}
	}

	if (cur->state->mapped_box) {
		return cur->state->mapped_box;
	}
	if (cur->state->mapped_output) {
		return wlr_output_layout_get_box(cur->state->layout,
			cur->state->mapped_output);
	}

	return NULL;
}

bool wlr_cursor_warp(struct wlr_cursor *cur, struct wlr_input_device *dev,
		double lx, double ly) {
	assert(cur->state->layout);

	bool result = false;
	struct wlr_box *mapping = get_mapping(cur, dev);
	if (mapping) {
		result = wlr_box_contains_point(mapping, lx, ly);
	} else {
		result = wlr_output_layout_contains_point(cur->state->layout, NULL,
			lx, ly);
	}

	if (result) {
		cursor_warp_unchecked(cur, lx, ly);
	}

	return result;
}

void wlr_cursor_warp_closest(struct wlr_cursor *cur,
		struct wlr_input_device *dev, double lx, double ly) {
	struct wlr_box *mapping = get_mapping(cur, dev);
	if (mapping) {
		wlr_box_closest_point(mapping, lx, ly, &lx, &ly);
		if (isnan(lx) || isnan(ly)) {
			lx = 0;
			ly = 0;
		}
	} else {
		wlr_output_layout_closest_point(cur->state->layout, NULL, lx, ly,
			&lx, &ly);
	}

	cursor_warp_unchecked(cur, lx, ly);
}

void wlr_cursor_absolute_to_layout_coords(struct wlr_cursor *cur,
		struct wlr_input_device *dev, double x, double y,
		double *lx, double *ly) {
	assert(cur->state->layout);

	struct wlr_box *mapping = get_mapping(cur, dev);
	if (!mapping) {
		mapping = wlr_output_layout_get_box(cur->state->layout, NULL);
	}

	*lx = !isnan(x) ? mapping->width * x + mapping->x : cur->x;
	*ly = !isnan(y) ? mapping->height * y + mapping->y : cur->y;
}

void wlr_cursor_warp_absolute(struct wlr_cursor *cur,
		struct wlr_input_device *dev, double x, double y) {
	assert(cur->state->layout);

	double lx, ly;
	wlr_cursor_absolute_to_layout_coords(cur, dev, x, y, &lx, &ly);

	wlr_cursor_warp_closest(cur, dev, lx, ly);
}

void wlr_cursor_move(struct wlr_cursor *cur, struct wlr_input_device *dev,
		double delta_x, double delta_y) {
	assert(cur->state->layout);

	double lx = !isnan(delta_x) ? cur->x + delta_x : cur->x;
	double ly = !isnan(delta_y) ? cur->y + delta_y : cur->y;

	wlr_cursor_warp_closest(cur, dev, lx, ly);
}

void wlr_cursor_set_image(struct wlr_cursor *cur, const uint8_t *pixels,
		int32_t stride, uint32_t width, uint32_t height, int32_t hotspot_x,
		int32_t hotspot_y, float scale) {
	struct wlr_cursor_output_cursor *output_cursor;
	wl_list_for_each(output_cursor, &cur->state->output_cursors, link) {
		float output_scale = output_cursor->output_cursor->output->scale;
		if (scale > 0 && output_scale != scale) {
			continue;
		}

		wlr_output_cursor_set_image(output_cursor->output_cursor, pixels,
			stride, width, height, hotspot_x, hotspot_y);
	}
}

void wlr_cursor_set_surface(struct wlr_cursor *cur, struct wlr_surface *surface,
		int32_t hotspot_x, int32_t hotspot_y) {
	struct wlr_cursor_output_cursor *output_cursor;
	wl_list_for_each(output_cursor, &cur->state->output_cursors, link) {
		wlr_output_cursor_set_surface(output_cursor->output_cursor, surface,
			hotspot_x, hotspot_y);
	}
}

static void handle_pointer_motion(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_motion *event = data;
	struct wlr_cursor_device *device =
		wl_container_of(listener, device, motion);
	wlr_signal_emit_safe(&device->cursor->events.motion, event);
}

static void apply_output_transform(double *x, double *y,
		enum wl_output_transform transform) {
	double dx = 0.0, dy = 0.0;
	double width = 1.0, height = 1.0;

	switch (transform) {
	case WL_OUTPUT_TRANSFORM_NORMAL:
		dx = *x;
		dy = *y;
		break;
	case WL_OUTPUT_TRANSFORM_90:
		dx = height - *y;
		dy = *x;
		break;
	case WL_OUTPUT_TRANSFORM_180:
		dx = width - *x;
		dy = height - *y;
		break;
	case WL_OUTPUT_TRANSFORM_270:
		dx = *y;
		dy = width - *x;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED:
		dx = width - *x;
		dy = *y;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		dx = *y;
		dy = *x;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_180:
		dx = *x;
		dy = height - *y;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_270:
		dx = height - *y;
		dy = width - *x;
		break;
	}
	*x = dx;
	*y = dy;
}


static struct wlr_output *get_mapped_output(struct wlr_cursor_device *cursor_device) {
	if (cursor_device->mapped_output) {
		return cursor_device->mapped_output;
	}

	struct wlr_cursor *cursor = cursor_device->cursor;
	assert(cursor);
	if (cursor->state->mapped_output) {
		return cursor->state->mapped_output;
	}
	return NULL;
}


static void handle_pointer_motion_absolute(struct wl_listener *listener,
		void *data) {
	struct wlr_event_pointer_motion_absolute *event = data;
	struct wlr_cursor_device *device =
		wl_container_of(listener, device, motion_absolute);

	struct wlr_output *output =
		get_mapped_output(device);
	if (output) {
		apply_output_transform(&event->x, &event->y, output->transform);
	}
	wlr_signal_emit_safe(&device->cursor->events.motion_absolute, event);
}

static void handle_pointer_button(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_button *event = data;
	struct wlr_cursor_device *device =
		wl_container_of(listener, device, button);
	wlr_signal_emit_safe(&device->cursor->events.button, event);
}

static void handle_pointer_axis(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_axis *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, axis);
	wlr_signal_emit_safe(&device->cursor->events.axis, event);
}

static void handle_pointer_frame(struct wl_listener *listener, void *data) {
	struct wlr_cursor_device *device = wl_container_of(listener, device, frame);
	wlr_signal_emit_safe(&device->cursor->events.frame, device->cursor);
}

static void handle_pointer_swipe_begin(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_swipe_begin *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, swipe_begin);
	wlr_signal_emit_safe(&device->cursor->events.swipe_begin, event);
}

static void handle_pointer_swipe_update(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_swipe_update *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, swipe_update);
	wlr_signal_emit_safe(&device->cursor->events.swipe_update, event);
}

static void handle_pointer_swipe_end(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_swipe_end *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, swipe_end);
	wlr_signal_emit_safe(&device->cursor->events.swipe_end, event);
}

static void handle_pointer_pinch_begin(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_pinch_begin *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, pinch_begin);
	wlr_signal_emit_safe(&device->cursor->events.pinch_begin, event);
}

static void handle_pointer_pinch_update(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_pinch_update *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, pinch_update);
	wlr_signal_emit_safe(&device->cursor->events.pinch_update, event);
}

static void handle_pointer_pinch_end(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_pinch_end *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, pinch_end);
	wlr_signal_emit_safe(&device->cursor->events.pinch_end, event);
}

static void handle_pointer_hold_begin(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_hold_begin *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, hold_begin);
	wlr_signal_emit_safe(&device->cursor->events.hold_begin, event);
}

static void handle_pointer_hold_end(struct wl_listener *listener, void *data) {
	struct wlr_event_pointer_hold_end *event = data;
	struct wlr_cursor_device *device = wl_container_of(listener, device, hold_end);
	wlr_signal_emit_safe(&device->cursor->events.hold_end, event);
}

static void handle_touch_up(struct wl_listener *listener, void *data) {
	struct wlr_event_touch_up *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, touch_up);
	wlr_signal_emit_safe(&device->cursor->events.touch_up, event);
}

static void handle_touch_down(struct wl_listener *listener, void *data) {
	struct wlr_event_touch_down *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, touch_down);

	struct wlr_output *output =
		get_mapped_output(device);
	if (output) {
		apply_output_transform(&event->x, &event->y, output->transform);
	}
	wlr_signal_emit_safe(&device->cursor->events.touch_down, event);
}

static void handle_touch_motion(struct wl_listener *listener, void *data) {
	struct wlr_event_touch_motion *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, touch_motion);

	struct wlr_output *output =
		get_mapped_output(device);
	if (output) {
		apply_output_transform(&event->x, &event->y, output->transform);
	}
	wlr_signal_emit_safe(&device->cursor->events.touch_motion, event);
}

static void handle_touch_cancel(struct wl_listener *listener, void *data) {
	struct wlr_event_touch_cancel *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, touch_cancel);
	wlr_signal_emit_safe(&device->cursor->events.touch_cancel, event);
}

static void handle_touch_frame(struct wl_listener *listener, void *data) {
	struct wlr_cursor_device *device =
		wl_container_of(listener, device, touch_frame);
	wlr_signal_emit_safe(&device->cursor->events.touch_frame, NULL);
}

static void handle_tablet_tool_tip(struct wl_listener *listener, void *data) {
	struct wlr_event_tablet_tool_tip *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, tablet_tool_tip);

	struct wlr_output *output =
		get_mapped_output(device);
	if (output) {
		apply_output_transform(&event->x, &event->y, output->transform);
	}
	wlr_signal_emit_safe(&device->cursor->events.tablet_tool_tip, event);
}

static void handle_tablet_tool_axis(struct wl_listener *listener, void *data) {
	struct wlr_event_tablet_tool_axis *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, tablet_tool_axis);

	struct wlr_output *output = get_mapped_output(device);
	if (output) {
		// In the case that only one axis received an event, rotating the input can
		// cause the change to actually happen on the other axis, as far as clients
		// are concerned.
		//
		// Here, we feed apply_output_transform NAN on the axis that didn't change,
		// and remap the axes flags based on whether it returns NAN itself.
		double x = event->updated_axes & WLR_TABLET_TOOL_AXIS_X ? event->x : NAN;
		double y = event->updated_axes & WLR_TABLET_TOOL_AXIS_Y ? event->y : NAN;

		apply_output_transform(&x, &y, output->transform);

		event->updated_axes &= ~(WLR_TABLET_TOOL_AXIS_X | WLR_TABLET_TOOL_AXIS_Y);
		event->x = event->y = 0;

		if (!isnan(x)) {
			event->updated_axes |= WLR_TABLET_TOOL_AXIS_X;
			event->x = x;
		}

		if (!isnan(y)) {
			event->updated_axes |= WLR_TABLET_TOOL_AXIS_Y;
			event->y = y;
		}
	}

	wlr_signal_emit_safe(&device->cursor->events.tablet_tool_axis, event);
}

static void handle_tablet_tool_button(struct wl_listener *listener,
		void *data) {
	struct wlr_event_tablet_tool_button *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, tablet_tool_button);
	wlr_signal_emit_safe(&device->cursor->events.tablet_tool_button, event);
}

static void handle_tablet_tool_proximity(struct wl_listener *listener,
		void *data) {
	struct wlr_event_tablet_tool_proximity *event = data;
	struct wlr_cursor_device *device;
	device = wl_container_of(listener, device, tablet_tool_proximity);

	struct wlr_output *output =
		get_mapped_output(device);
	if (output) {
		apply_output_transform(&event->x, &event->y, output->transform);
	}
	wlr_signal_emit_safe(&device->cursor->events.tablet_tool_proximity, event);
}

static void handle_device_destroy(struct wl_listener *listener, void *data) {
	struct wlr_cursor_device *c_device;
	c_device = wl_container_of(listener, c_device, destroy);
	wlr_cursor_detach_input_device(c_device->cursor, c_device->device);
}

static struct wlr_cursor_device *cursor_device_create(
		struct wlr_cursor *cursor, struct wlr_input_device *device) {
	struct wlr_cursor_device *c_device =
		calloc(1, sizeof(struct wlr_cursor_device));
	if (!c_device) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_cursor_device");
		return NULL;
	}

	c_device->cursor = cursor;
	c_device->device = device;

	// listen to events
	wl_signal_add(&device->events.destroy, &c_device->destroy);
	c_device->destroy.notify = handle_device_destroy;

	if (device->type == WLR_INPUT_DEVICE_POINTER) {
		wl_signal_add(&device->pointer->events.motion, &c_device->motion);
		c_device->motion.notify = handle_pointer_motion;

		wl_signal_add(&device->pointer->events.motion_absolute,
			&c_device->motion_absolute);
		c_device->motion_absolute.notify = handle_pointer_motion_absolute;

		wl_signal_add(&device->pointer->events.button, &c_device->button);
		c_device->button.notify = handle_pointer_button;

		wl_signal_add(&device->pointer->events.axis, &c_device->axis);
		c_device->axis.notify = handle_pointer_axis;

		wl_signal_add(&device->pointer->events.frame, &c_device->frame);
		c_device->frame.notify = handle_pointer_frame;

		wl_signal_add(&device->pointer->events.swipe_begin, &c_device->swipe_begin);
		c_device->swipe_begin.notify = handle_pointer_swipe_begin;

		wl_signal_add(&device->pointer->events.swipe_update, &c_device->swipe_update);
		c_device->swipe_update.notify = handle_pointer_swipe_update;

		wl_signal_add(&device->pointer->events.swipe_end, &c_device->swipe_end);
		c_device->swipe_end.notify = handle_pointer_swipe_end;

		wl_signal_add(&device->pointer->events.pinch_begin, &c_device->pinch_begin);
		c_device->pinch_begin.notify = handle_pointer_pinch_begin;

		wl_signal_add(&device->pointer->events.pinch_update, &c_device->pinch_update);
		c_device->pinch_update.notify = handle_pointer_pinch_update;

		wl_signal_add(&device->pointer->events.pinch_end, &c_device->pinch_end);
		c_device->pinch_end.notify = handle_pointer_pinch_end;

		wl_signal_add(&device->pointer->events.hold_begin, &c_device->hold_begin);
		c_device->hold_begin.notify = handle_pointer_hold_begin;

		wl_signal_add(&device->pointer->events.hold_end, &c_device->hold_end);
		c_device->hold_end.notify = handle_pointer_hold_end;
	} else if (device->type == WLR_INPUT_DEVICE_TOUCH) {
		wl_signal_add(&device->touch->events.motion, &c_device->touch_motion);
		c_device->touch_motion.notify = handle_touch_motion;

		wl_signal_add(&device->touch->events.down, &c_device->touch_down);
		c_device->touch_down.notify = handle_touch_down;

		wl_signal_add(&device->touch->events.up, &c_device->touch_up);
		c_device->touch_up.notify = handle_touch_up;

		wl_signal_add(&device->touch->events.cancel, &c_device->touch_cancel);
		c_device->touch_cancel.notify = handle_touch_cancel;

		wl_signal_add(&device->touch->events.frame, &c_device->touch_frame);
		c_device->touch_frame.notify = handle_touch_frame;
	} else if (device->type == WLR_INPUT_DEVICE_TABLET_TOOL) {
		wl_signal_add(&device->tablet->events.tip,
			&c_device->tablet_tool_tip);
		c_device->tablet_tool_tip.notify = handle_tablet_tool_tip;

		wl_signal_add(&device->tablet->events.proximity,
			&c_device->tablet_tool_proximity);
		c_device->tablet_tool_proximity.notify = handle_tablet_tool_proximity;

		wl_signal_add(&device->tablet->events.axis,
			&c_device->tablet_tool_axis);
		c_device->tablet_tool_axis.notify = handle_tablet_tool_axis;

		wl_signal_add(&device->tablet->events.button,
			&c_device->tablet_tool_button);
		c_device->tablet_tool_button.notify = handle_tablet_tool_button;
	}

	wl_list_insert(&cursor->state->devices, &c_device->link);

	return c_device;
}

void wlr_cursor_attach_input_device(struct wlr_cursor *cur,
		struct wlr_input_device *dev) {
	if (dev->type != WLR_INPUT_DEVICE_POINTER &&
			dev->type != WLR_INPUT_DEVICE_TOUCH &&
			dev->type != WLR_INPUT_DEVICE_TABLET_TOOL) {
		wlr_log(WLR_ERROR, "only device types of pointer, touch or tablet tool"
				"are supported");
		return;
	}

	// make sure it is not already attached
	struct wlr_cursor_device *_dev;
	wl_list_for_each(_dev, &cur->state->devices, link) {
		if (_dev->device == dev) {
			return;
		}
	}

	cursor_device_create(cur, dev);
}

void wlr_cursor_detach_input_device(struct wlr_cursor *cur,
		struct wlr_input_device *dev) {
	struct wlr_cursor_device *c_device, *tmp = NULL;
	wl_list_for_each_safe(c_device, tmp, &cur->state->devices, link) {
		if (c_device->device == dev) {
			cursor_device_destroy(c_device);
		}
	}
}

static void handle_layout_destroy(struct wl_listener *listener, void *data) {
	struct wlr_cursor_state *state =
		wl_container_of(listener, state, layout_destroy);
	cursor_detach_output_layout(state->cursor);
}

static void handle_layout_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_cursor_output_cursor *output_cursor =
		wl_container_of(listener, output_cursor, layout_output_destroy);
	//struct wlr_output_layout_output *l_output = data;
	output_cursor_destroy(output_cursor);
}

static void layout_add(struct wlr_cursor_state *state,
		struct wlr_output_layout_output *l_output) {
	struct wlr_cursor_output_cursor *output_cursor;
	wl_list_for_each(output_cursor, &state->output_cursors, link) {
		if (output_cursor->output_cursor->output == l_output->output) {
			return; // already added
		}
	}

	output_cursor = calloc(1, sizeof(struct wlr_cursor_output_cursor));
	if (output_cursor == NULL) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_cursor_output_cursor");
		return;
	}
	output_cursor->cursor = state->cursor;

	output_cursor->output_cursor = wlr_output_cursor_create(l_output->output);
	if (output_cursor->output_cursor == NULL) {
		wlr_log(WLR_ERROR, "Failed to create wlr_output_cursor");
		free(output_cursor);
		return;
	}

	output_cursor->layout_output_destroy.notify = handle_layout_output_destroy;
	wl_signal_add(&l_output->events.destroy,
		&output_cursor->layout_output_destroy);

	wl_list_insert(&state->output_cursors, &output_cursor->link);
}

static void handle_layout_add(struct wl_listener *listener, void *data) {
	struct wlr_cursor_state *state =
		wl_container_of(listener, state, layout_add);
	struct wlr_output_layout_output *l_output = data;
	layout_add(state, l_output);
}

static void handle_layout_change(struct wl_listener *listener, void *data) {
	struct wlr_cursor_state *state =
		wl_container_of(listener, state, layout_change);
	struct wlr_output_layout *layout = data;

	if (!wlr_output_layout_contains_point(layout, NULL, state->cursor->x,
			state->cursor->y)) {
		// the output we were on has gone away so go to the closest boundary
		// point
		double x, y;
		wlr_output_layout_closest_point(layout, NULL, state->cursor->x,
			state->cursor->y, &x, &y);

		cursor_warp_unchecked(state->cursor, x, y);
	}
}

void wlr_cursor_attach_output_layout(struct wlr_cursor *cur,
		struct wlr_output_layout *l) {
	cursor_detach_output_layout(cur);

	if (l == NULL) {
		return;
	}

	wl_signal_add(&l->events.add, &cur->state->layout_add);
	cur->state->layout_add.notify = handle_layout_add;
	wl_signal_add(&l->events.change, &cur->state->layout_change);
	cur->state->layout_change.notify = handle_layout_change;
	wl_signal_add(&l->events.destroy, &cur->state->layout_destroy);
	cur->state->layout_destroy.notify = handle_layout_destroy;

	cur->state->layout = l;

	struct wlr_output_layout_output *l_output;
	wl_list_for_each(l_output, &l->outputs, link) {
		layout_add(cur->state, l_output);
	}
}

void wlr_cursor_map_to_output(struct wlr_cursor *cur,
		struct wlr_output *output) {
	cur->state->mapped_output = output;
}

void wlr_cursor_map_input_to_output(struct wlr_cursor *cur,
		struct wlr_input_device *dev, struct wlr_output *output) {
	struct wlr_cursor_device *c_device = get_cursor_device(cur, dev);
	if (!c_device) {
		wlr_log(WLR_ERROR, "Cannot map device \"%s\" to output"
			" (not found in this cursor)", dev->name);
		return;
	}

	c_device->mapped_output = output;
}

void wlr_cursor_map_to_region(struct wlr_cursor *cur,
		struct wlr_box *box) {
	if (box && wlr_box_empty(box)) {
		wlr_log(WLR_ERROR, "cannot map cursor to an empty region");
		return;
	}

	cur->state->mapped_box = box;
}

void wlr_cursor_map_input_to_region(struct wlr_cursor *cur,
		struct wlr_input_device *dev, struct wlr_box *box) {
	if (box && wlr_box_empty(box)) {
		wlr_log(WLR_ERROR, "cannot map device \"%s\" input to an empty region",
			dev->name);
		return;
	}

	struct wlr_cursor_device *c_device = get_cursor_device(cur, dev);
	if (!c_device) {
		wlr_log(WLR_ERROR, "Cannot map device \"%s\" to geometry (not found in"
			"this cursor)", dev->name);
		return;
	}

	c_device->mapped_box = box;
}
