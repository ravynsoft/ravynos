#define _XOPEN_SOURCE 600
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
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_tablet_pad.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

struct sample_state {
	struct wl_display *display;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	bool proximity, tap, button;
	double distance;
	double pressure;
	double x, y;
	double x_tilt, y_tilt;
	double width_mm, height_mm;
	double ring;
	struct wl_list link;
	float tool_color[4];
	float pad_color[4];
	struct timespec last_frame;
	struct wl_listener new_output;
	struct wl_listener new_input;
	struct wl_list tablet_tools;
	struct wl_list tablet_pads;
};

struct tablet_tool_state {
	struct sample_state *sample;
	struct wlr_input_device *device;
	struct wl_listener destroy;
	struct wl_listener axis;
	struct wl_listener proximity;
	struct wl_listener tip;
	struct wl_listener button;
	struct wl_list link;
	void *data;
};

struct tablet_pad_state {
	struct sample_state *sample;
	struct wlr_input_device *device;
	struct wl_listener destroy;
	struct wl_listener button;
	struct wl_listener ring;
	struct wl_list link;
	void *data;
};

struct sample_output {
	struct sample_state *sample;
	struct wlr_output *output;
	struct wl_listener frame;
	struct wl_listener destroy;
};

struct sample_keyboard {
	struct sample_state *sample;
	struct wlr_input_device *device;
	struct wl_listener key;
	struct wl_listener destroy;
};

static void output_frame_notify(struct wl_listener *listener, void *data) {
	struct sample_output *sample_output = wl_container_of(listener, sample_output, frame);
	struct sample_state *sample = sample_output->sample;
	struct wlr_output *wlr_output = sample_output->output;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	int32_t width, height;
	wlr_output_effective_resolution(wlr_output, &width, &height);

	wlr_output_attach_render(wlr_output, NULL);
	wlr_renderer_begin(sample->renderer, wlr_output->width, wlr_output->height);
	wlr_renderer_clear(sample->renderer, (float[]){0.25f, 0.25f, 0.25f, 1});

	float distance = 0.8f * (1 - sample->distance);
	float tool_color[4] = { distance, distance, distance, 1 };
	for (size_t i = 0; sample->button && i < 4; ++i) {
		tool_color[i] = sample->tool_color[i];
	}
	float scale = 4;

	float pad_width = sample->width_mm * scale;
	float pad_height = sample->height_mm * scale;
	float left = width / 2.0f - pad_width / 2.0f;
	float top = height / 2.0f - pad_height / 2.0f;
	const struct wlr_box box = {
		.x = left, .y = top,
		.width = pad_width, .height = pad_height,
	};
	wlr_render_rect(sample->renderer, &box, sample->pad_color,
		wlr_output->transform_matrix);

	if (sample->proximity) {
		struct wlr_box box = {
			.x = (sample->x * pad_width) - 8 * (sample->pressure + 1) + left,
			.y = (sample->y * pad_height) - 8 * (sample->pressure + 1) + top,
			.width = 16 * (sample->pressure + 1),
			.height = 16 * (sample->pressure + 1),
		};
		float matrix[9];
		wlr_matrix_project_box(matrix, &box, WL_OUTPUT_TRANSFORM_NORMAL,
			sample->ring, wlr_output->transform_matrix);
		wlr_render_quad_with_matrix(sample->renderer, tool_color, matrix);

		box.x += sample->x_tilt;
		box.y += sample->y_tilt;
		box.width /= 2;
		box.height /= 2;
		wlr_render_rect(sample->renderer, &box, tool_color,
			wlr_output->transform_matrix);
	}

	wlr_renderer_end(sample->renderer);
	wlr_output_commit(wlr_output);
	sample->last_frame = now;
}

static void tablet_tool_axis_notify(struct wl_listener *listener, void *data) {
	struct tablet_tool_state *tstate = wl_container_of(listener, tstate, axis);
	struct wlr_event_tablet_tool_axis *event = data;
	struct sample_state *sample = tstate->sample;
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_X)) {
		sample->x = event->x;
	}
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_Y)) {
		sample->y = event->y;
	}
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_DISTANCE)) {
		sample->distance = event->distance;
	}
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_PRESSURE)) {
		sample->pressure = event->pressure;
	}
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_TILT_X)) {
		sample->x_tilt = event->tilt_x;
	}
	if ((event->updated_axes & WLR_TABLET_TOOL_AXIS_TILT_Y)) {
		sample->y_tilt = event->tilt_y;
	}
}

static void tablet_tool_proximity_notify(struct wl_listener *listener, void *data) {
	struct tablet_tool_state *tstate = wl_container_of(listener, tstate, proximity);
	struct wlr_event_tablet_tool_proximity *event = data;
	struct sample_state *sample = tstate->sample;
	sample->proximity = event->state == WLR_TABLET_TOOL_PROXIMITY_IN;
}

static void tablet_tool_button_notify(struct wl_listener *listener, void *data) {
	struct tablet_tool_state *tstate = wl_container_of(listener, tstate, button);
	struct wlr_event_tablet_tool_button *event = data;
	struct sample_state *sample = tstate->sample;
	if (event->state == WLR_BUTTON_RELEASED) {
		sample->button = false;
	} else {
		sample->button = true;
		for (size_t i = 0; i < 3; ++i) {
			if (event->button % 3 == i) {
				sample->tool_color[i] = 0;
			} else {
				sample->tool_color[i] = 1;
			}
		}
	}
}

static void tablet_pad_button_notify(struct wl_listener *listener, void *data) {
	struct tablet_pad_state *pstate = wl_container_of(listener, pstate, button);
	struct wlr_event_tablet_pad_button *event = data;
	struct sample_state *sample = pstate->sample;
	float default_color[4] = { 0.5, 0.5, 0.5, 1.0 };
	if (event->state == WLR_BUTTON_RELEASED) {
		memcpy(sample->pad_color, default_color, sizeof(default_color));
	} else {
		for (size_t i = 0; i < 3; ++i) {
			if (event->button % 3 == i) {
				sample->pad_color[i] = 0;
			} else {
				sample->pad_color[i] = 1;
			}
		}
	}
}

static void tablet_pad_ring_notify(struct wl_listener *listener, void *data) {
	struct tablet_pad_state *pstate = wl_container_of(listener, pstate, ring);
	struct wlr_event_tablet_pad_ring *event = data;
	struct sample_state *sample = pstate->sample;
	if (event->position != -1) {
		sample->ring = -(event->position * (M_PI / 180.0));
	}
}

static void tablet_tool_destroy_notify(struct wl_listener *listener, void *data) {
	struct tablet_tool_state *tstate = wl_container_of(listener, tstate, destroy);
	wl_list_remove(&tstate->link);
	wl_list_remove(&tstate->destroy.link);
	wl_list_remove(&tstate->axis.link);
	wl_list_remove(&tstate->proximity.link);
	wl_list_remove(&tstate->button.link);
	free(tstate);
}

static void tablet_pad_destroy_notify(struct wl_listener *listener, void *data) {
	struct tablet_pad_state *pstate = wl_container_of(listener, pstate, destroy);
	wl_list_remove(&pstate->link);
	wl_list_remove(&pstate->destroy.link);
	wl_list_remove(&pstate->ring.link);
	wl_list_remove(&pstate->button.link);
	free(pstate);
}

static void output_remove_notify(struct wl_listener *listener, void *data) {
	struct sample_output *sample_output = wl_container_of(listener, sample_output, destroy);
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
	sample_output->sample = sample;
	wl_signal_add(&output->events.frame, &sample_output->frame);
	sample_output->frame.notify = output_frame_notify;
	wl_signal_add(&output->events.destroy, &sample_output->destroy);
	sample_output->destroy.notify = output_remove_notify;

	struct wlr_output_mode *mode = wlr_output_preferred_mode(output);
	if (mode != NULL) {
		wlr_output_set_mode(output, mode);
	}

	wlr_output_commit(output);
}

static void keyboard_key_notify(struct wl_listener *listener, void *data) {
	struct sample_keyboard *keyboard = wl_container_of(listener, keyboard, key);
	struct sample_state *sample = keyboard->sample;
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

static void keyboard_destroy_notify(struct wl_listener *listener, void *data) {
	struct sample_keyboard *keyboard = wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->key.link);
	free(keyboard);
}

static void new_input_notify(struct wl_listener *listener, void *data) {
	struct wlr_input_device *device = data;
	struct sample_state *sample = wl_container_of(listener, sample, new_input);
	switch (device->type) {
	case WLR_INPUT_DEVICE_KEYBOARD:;
		struct sample_keyboard *keyboard = calloc(1, sizeof(struct sample_keyboard));
		keyboard->device = device;
		keyboard->sample = sample;
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
	case WLR_INPUT_DEVICE_TABLET_PAD:;
		struct tablet_pad_state *pstate = calloc(sizeof(struct tablet_pad_state), 1);
		pstate->device = device;
		pstate->sample = sample;
		pstate->destroy.notify = tablet_pad_destroy_notify;
		wl_signal_add(&device->events.destroy, &pstate->destroy);
		pstate->button.notify = tablet_pad_button_notify;
		wl_signal_add(&device->tablet_pad->events.button, &pstate->button);
		pstate->ring.notify = tablet_pad_ring_notify;
		wl_signal_add(&device->tablet_pad->events.ring, &pstate->ring);
		wl_list_insert(&sample->tablet_pads, &pstate->link);
		break;
	case WLR_INPUT_DEVICE_TABLET_TOOL:
		sample->width_mm = device->width_mm == 0 ?
			20 : device->width_mm;
		sample->height_mm = device->height_mm == 0 ?
			10 : device->height_mm;

		struct tablet_tool_state *tstate = calloc(sizeof(struct tablet_tool_state), 1);
		tstate->device = device;
		tstate->sample = sample;
		tstate->destroy.notify = tablet_tool_destroy_notify;
		wl_signal_add(&device->events.destroy, &tstate->destroy);
		tstate->axis.notify = tablet_tool_axis_notify;
		wl_signal_add(&device->tablet->events.axis, &tstate->axis);
		tstate->proximity.notify = tablet_tool_proximity_notify;
		wl_signal_add(&device->tablet->events.proximity, &tstate->proximity);
		tstate->button.notify = tablet_tool_button_notify;
		wl_signal_add(&device->tablet->events.button, &tstate->button);
		wl_list_insert(&sample->tablet_tools, &tstate->link);
		break;
	default:
		break;
	}
}


int main(int argc, char *argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);
	struct wl_display *display = wl_display_create();
	struct sample_state state = {
		.display = display,
		.tool_color = { 1, 1, 1, 1 },
		.pad_color = { 0.5, 0.5, 0.5, 1.0 }
	};
	wl_list_init(&state.tablet_pads);
	wl_list_init(&state.tablet_tools);
	struct wlr_backend *wlr = wlr_backend_autocreate(display);
	if (!wlr) {
		exit(1);
	}

	wl_signal_add(&wlr->events.new_output, &state.new_output);
	state.new_output.notify = new_output_notify;
	wl_signal_add(&wlr->events.new_input, &state.new_input);
	state.new_input.notify = new_input_notify;
	clock_gettime(CLOCK_MONOTONIC, &state.last_frame);

	state.renderer = wlr_renderer_autocreate(wlr);
	if (!state.renderer) {
		wlr_log(WLR_ERROR, "Could not start compositor, OOM");
		exit(EXIT_FAILURE);
	}

	state.allocator = wlr_allocator_autocreate(wlr, state.renderer);

	if (!wlr_backend_start(wlr)) {
		wlr_log(WLR_ERROR, "Failed to start backend");
		wlr_backend_destroy(wlr);
		exit(1);
	}
	wl_display_run(display);

	wl_display_destroy(display);
}
