#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_touch.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

struct sample_state {
	struct wl_display *display;
	struct compositor_state *compositor;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wlr_xcursor_manager *xcursor_manager;
	struct wlr_cursor *cursor;
	double cur_x, cur_y;
	float default_color[4];
	float clear_color[4];
	struct wlr_output_layout *layout;
	struct wl_list devices;
	struct timespec last_frame;

	struct wl_listener new_output;
	struct wl_listener new_input;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;

	struct wl_listener touch_motion;
	struct wl_listener touch_up;
	struct wl_listener touch_down;
	struct wl_listener touch_cancel;
	struct wl_list touch_points;

	struct wl_listener tablet_tool_axis;
	struct wl_listener tablet_tool_proxmity;
	struct wl_listener tablet_tool_tip;
	struct wl_listener tablet_tool_button;
};

struct touch_point {
	int32_t touch_id;
	double x, y;
	struct wl_list link;
};

struct sample_output {
	struct sample_state *state;
	struct wlr_output *output;
	struct wl_listener frame;
	struct wl_listener destroy;
};

struct sample_keyboard {
	struct sample_state *state;
	struct wlr_input_device *device;
	struct wl_listener key;
	struct wl_listener destroy;
};

static void warp_to_touch(struct sample_state *state,
		struct wlr_input_device *dev) {
	if (wl_list_empty(&state->touch_points)) {
		return;
	}

	double x = 0, y = 0;
	size_t n = 0;
	struct touch_point *point;
	wl_list_for_each(point, &state->touch_points, link) {
		x += point->x;
		y += point->y;
		n++;
	}
	x /= n;
	y /= n;
	wlr_cursor_warp_absolute(state->cursor, dev, x, y);
}

static void output_frame_notify(struct wl_listener *listener, void *data) {
	struct sample_output *sample_output = wl_container_of(listener, sample_output, frame);
	struct sample_state *state = sample_output->state;
	struct wlr_output *wlr_output = sample_output->output;
	struct wlr_renderer *renderer = state->renderer;
	assert(renderer);

	wlr_output_attach_render(wlr_output, NULL);
	wlr_renderer_begin(renderer, wlr_output->width, wlr_output->height);
	wlr_renderer_clear(renderer, state->clear_color);
	wlr_output_render_software_cursors(wlr_output, NULL);
	wlr_renderer_end(renderer);
	wlr_output_commit(wlr_output);
}

static void handle_cursor_motion(struct wl_listener *listener, void *data) {
	struct sample_state *sample =
		wl_container_of(listener, sample, cursor_motion);
	struct wlr_event_pointer_motion *event = data;
	wlr_cursor_move(sample->cursor, event->device, event->delta_x,
			event->delta_y);
}

static void handle_cursor_motion_absolute(struct wl_listener *listener,
		void *data) {
	struct sample_state *sample =
		wl_container_of(listener, sample, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute *event = data;

	sample->cur_x = event->x;
	sample->cur_y = event->y;

	wlr_cursor_warp_absolute(sample->cursor, event->device, sample->cur_x,
		sample->cur_y);
}

static void handle_cursor_button(struct wl_listener *listener, void *data) {
	struct sample_state *sample =
		wl_container_of(listener, sample, cursor_button);
	struct wlr_event_pointer_button *event = data;

	float (*color)[4];
	if (event->state == WLR_BUTTON_RELEASED) {
		color = &sample->default_color;
		memcpy(&sample->clear_color, color, sizeof(*color));
	} else {
		float red[4] = { 0.25f, 0.25f, 0.25f, 1 };
		red[event->button % 3] = 1;
		color = &red;
		memcpy(&sample->clear_color, color, sizeof(*color));
	}
}

static void handle_cursor_axis(struct wl_listener *listener, void *data) {
	struct sample_state *sample =
		wl_container_of(listener, sample, cursor_axis);
	struct wlr_event_pointer_axis *event = data;

	for (size_t i = 0; i < 3; ++i) {
		sample->default_color[i] += event->delta > 0 ? -0.05f : 0.05f;
		if (sample->default_color[i] > 1.0f) {
			sample->default_color[i] = 1.0f;
		}
		if (sample->default_color[i] < 0.0f) {
			sample->default_color[i] = 0.0f;
		}
	}

	memcpy(&sample->clear_color, &sample->default_color,
			sizeof(sample->clear_color));
}

static void handle_touch_up(struct wl_listener *listener, void *data) {
	struct sample_state *sample = wl_container_of(listener, sample, touch_up);
	struct wlr_event_touch_up *event = data;

	struct touch_point *point, *tmp;
	wl_list_for_each_safe(point, tmp, &sample->touch_points, link) {
		if (point->touch_id == event->touch_id) {
			wl_list_remove(&point->link);
			break;
		}
	}

	warp_to_touch(sample, event->device);
}

static void handle_touch_down(struct wl_listener *listener, void *data) {
	struct sample_state *sample = wl_container_of(listener, sample, touch_down);
	struct wlr_event_touch_down *event = data;
	struct touch_point *point = calloc(1, sizeof(struct touch_point));
	point->touch_id = event->touch_id;
	point->x = event->x;
	point->y = event->y;
	wl_list_insert(&sample->touch_points, &point->link);

	warp_to_touch(sample, event->device);
}

static void handle_touch_motion(struct wl_listener *listener, void *data) {
	struct sample_state *sample =
		wl_container_of(listener, sample, touch_motion);
	struct wlr_event_touch_motion *event = data;

	struct touch_point *point;
	wl_list_for_each(point, &sample->touch_points, link) {
		if (point->touch_id == event->touch_id) {
			point->x = event->x;
			point->y = event->y;
			break;
		}
	}

	warp_to_touch(sample, event->device);
}

static void handle_touch_cancel(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "TODO: touch cancel");
}

static void handle_tablet_tool_axis(struct wl_listener *listener, void *data) {
	struct sample_state *sample =
		wl_container_of(listener, sample, tablet_tool_axis);
	struct wlr_event_tablet_tool_axis *event = data;
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_X) &&
			(event->updated_axes & WLR_TABLET_TOOL_AXIS_Y)) {
		wlr_cursor_warp_absolute(sample->cursor,
				event->device, event->x, event->y);
	}
}

static void keyboard_key_notify(struct wl_listener *listener, void *data) {
	struct sample_keyboard *keyboard = wl_container_of(listener, keyboard, key);
	struct sample_state *sample = keyboard->state;
	struct wlr_event_keyboard_key *event = data;
	uint32_t keycode = event->keycode + 8;
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(keyboard->device->keyboard->xkb_state,
			keycode, &syms);
	for (int i = 0; i < nsyms; i++) {
		xkb_keysym_t sym = syms[i];
		if (sym == XKB_KEY_Escape) {
			wl_display_terminate(sample->display);
		}
	}
}

static void output_remove_notify(struct wl_listener *listener, void *data) {
	struct sample_output *sample_output = wl_container_of(listener, sample_output, destroy);
	struct sample_state *sample = sample_output->state;
	wlr_output_layout_remove(sample->layout, sample_output->output);
	wl_list_remove(&sample_output->frame.link);
	wl_list_remove(&sample_output->destroy.link);
	free(sample_output);
}

static void new_output_notify(struct wl_listener *listener, void *data) {
	struct wlr_output *output = data;
	struct sample_state *sample = wl_container_of(listener, sample, new_output);

	wlr_output_init_render(output, sample->allocator, sample->renderer);

	struct sample_output *sample_output = calloc(1, sizeof(struct sample_output));
	sample_output->output = output;
	sample_output->state = sample;
	wl_signal_add(&output->events.frame, &sample_output->frame);
	sample_output->frame.notify = output_frame_notify;
	wl_signal_add(&output->events.destroy, &sample_output->destroy);
	sample_output->destroy.notify = output_remove_notify;
	wlr_output_layout_add_auto(sample->layout, sample_output->output);

	wlr_xcursor_manager_load(sample->xcursor_manager, output->scale);
	wlr_xcursor_manager_set_cursor_image(sample->xcursor_manager, "left_ptr",
		sample->cursor);

	struct wlr_output_mode *mode = wlr_output_preferred_mode(output);
	if (mode != NULL) {
		wlr_output_set_mode(output, mode);
	}

	wlr_output_commit(output);
}


static void keyboard_destroy_notify(struct wl_listener *listener, void *data) {
	struct sample_keyboard *keyboard = wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->key.link);
	free(keyboard);
}

static void new_input_notify(struct wl_listener *listener, void *data) {
	struct wlr_input_device *device = data;
	struct sample_state *state = wl_container_of(listener, state, new_input);
	switch (device->type) {
	case WLR_INPUT_DEVICE_POINTER:
	case WLR_INPUT_DEVICE_TOUCH:
	case WLR_INPUT_DEVICE_TABLET_TOOL:
		wlr_cursor_attach_input_device(state->cursor, device);
		break;

	case WLR_INPUT_DEVICE_KEYBOARD:;
		struct sample_keyboard *keyboard = calloc(1, sizeof(struct sample_keyboard));
		keyboard->device = device;
		keyboard->state = state;
		wl_signal_add(&device->events.destroy, &keyboard->destroy);
		keyboard->destroy.notify = keyboard_destroy_notify;
		wl_signal_add(&device->keyboard->events.key, &keyboard->key);
		keyboard->key.notify = keyboard_key_notify;
		struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		if (!context) {
			wlr_log(WLR_ERROR, "Failed to create XKB context");
			exit(1);
		}
		struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
			XKB_KEYMAP_COMPILE_NO_FLAGS);
		if (!keymap) {
			wlr_log(WLR_ERROR, "Failed to create XKB keymap");
			exit(1);
		}
		wlr_keyboard_set_keymap(device->keyboard, keymap);
		xkb_keymap_unref(keymap);
		xkb_context_unref(context);
		break;
	default:
		break;
	}
}


int main(int argc, char *argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);
	struct wl_display *display = wl_display_create();
	struct sample_state state = {
		.default_color = { 0.25f, 0.25f, 0.25f, 1 },
		.clear_color = { 0.25f, 0.25f, 0.25f, 1 },
		.display = display
	};

	struct wlr_backend *wlr = wlr_backend_autocreate(display);
	if (!wlr) {
		exit(1);
	}

	state.renderer = wlr_renderer_autocreate(wlr);
	state.allocator = wlr_allocator_autocreate(wlr, state.renderer);

	state.cursor = wlr_cursor_create();
	state.layout = wlr_output_layout_create();
	wlr_cursor_attach_output_layout(state.cursor, state.layout);
	//wlr_cursor_map_to_region(state.cursor, state.config->cursor.mapped_box);
	wl_list_init(&state.devices);
	wl_list_init(&state.touch_points);

	// pointer events
	wl_signal_add(&state.cursor->events.motion, &state.cursor_motion);
	state.cursor_motion.notify = handle_cursor_motion;

	wl_signal_add(&state.cursor->events.motion_absolute,
		&state.cursor_motion_absolute);
	state.cursor_motion_absolute.notify = handle_cursor_motion_absolute;

	wl_signal_add(&state.cursor->events.button, &state.cursor_button);
	state.cursor_button.notify = handle_cursor_button;

	wl_signal_add(&state.cursor->events.axis, &state.cursor_axis);
	state.cursor_axis.notify = handle_cursor_axis;

	// touch events
	wl_signal_add(&state.cursor->events.touch_up, &state.touch_up);
	state.touch_up.notify = handle_touch_up;

	wl_signal_add(&state.cursor->events.touch_down, &state.touch_down);
	state.touch_down.notify = handle_touch_down;

	wl_signal_add(&state.cursor->events.touch_motion, &state.touch_motion);
	state.touch_motion.notify = handle_touch_motion;

	wl_signal_add(&state.cursor->events.touch_cancel, &state.touch_cancel);
	state.touch_cancel.notify = handle_touch_cancel;

	wl_signal_add(&wlr->events.new_input, &state.new_input);
	state.new_input.notify = new_input_notify;

	wl_signal_add(&wlr->events.new_output, &state.new_output);
	state.new_output.notify = new_output_notify;

	// tool events
	wl_signal_add(&state.cursor->events.tablet_tool_axis,
		&state.tablet_tool_axis);
	state.tablet_tool_axis.notify = handle_tablet_tool_axis;

	state.xcursor_manager = wlr_xcursor_manager_create("default", 24);
	if (!state.xcursor_manager) {
		wlr_log(WLR_ERROR, "Failed to load left_ptr cursor");
		return 1;
	}

	wlr_xcursor_manager_set_cursor_image(state.xcursor_manager, "left_ptr",
		state.cursor);

	clock_gettime(CLOCK_MONOTONIC, &state.last_frame);

	if (!wlr_backend_start(wlr)) {
		wlr_log(WLR_ERROR, "Failed to start backend");
		wlr_backend_destroy(wlr);
		exit(1);
	}
	wl_display_run(display);
	wl_display_destroy(display);

	wlr_xcursor_manager_destroy(state.xcursor_manager);
	wlr_cursor_destroy(state.cursor);
	wlr_output_layout_destroy(state.layout);
}
