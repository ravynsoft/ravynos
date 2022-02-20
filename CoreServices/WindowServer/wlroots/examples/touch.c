#define _POSIX_C_SOURCE 200112L
#include <drm_fourcc.h>
#include <math.h>
#include <stdint.h>
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
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_touch.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>
#include "cat.h"

struct sample_state {
	struct wl_display *display;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wlr_texture *cat_texture;
	struct wl_list touch_points;
	struct timespec last_frame;
	struct wl_listener new_output;
	struct wl_listener new_input;
	struct wl_list touch;
};

struct touch_point {
	int32_t touch_id;
	double x, y;
	struct wl_list link;
};

struct touch_state {
	struct sample_state *sample;
	struct wlr_input_device *device;
	struct wl_listener destroy;
	struct wl_listener down;
	struct wl_listener up;
	struct wl_listener motion;
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
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	struct wlr_output *wlr_output = sample_output->output;

	int32_t width, height;
	wlr_output_effective_resolution(wlr_output, &width, &height);

	wlr_output_attach_render(wlr_output, NULL);
	wlr_renderer_begin(sample->renderer, wlr_output->width, wlr_output->height);
	wlr_renderer_clear(sample->renderer, (float[]){0.25f, 0.25f, 0.25f, 1});

	struct touch_point *p;
	wl_list_for_each(p, &sample->touch_points, link) {
		int x = (int)(p->x * width) - sample->cat_texture->width / 2;
		int y = (int)(p->y * height) - sample->cat_texture->height / 2;
		wlr_render_texture(sample->renderer, sample->cat_texture,
			wlr_output->transform_matrix, x, y, 1.0f);
	}

	wlr_renderer_end(sample->renderer);
	wlr_output_commit(wlr_output);
	sample->last_frame = now;
}

static void touch_down_notify(struct wl_listener *listener, void *data) {
	struct wlr_event_touch_motion *event = data;
	struct touch_state *tstate = wl_container_of(listener, tstate, down);
	struct sample_state *sample = tstate->sample;
	struct touch_point *point = calloc(1, sizeof(struct touch_point));
	point->touch_id = event->touch_id;
	point->x = event->x;
	point->y = event->y;
	wl_list_insert(&sample->touch_points, &point->link);
}

static void touch_up_notify(struct wl_listener *listener, void *data ) {
	struct wlr_event_touch_up *event = data;
	struct touch_state *tstate = wl_container_of(listener, tstate, up);
	struct sample_state *sample = tstate->sample;
	struct touch_point *point, *tmp;
	wl_list_for_each_safe(point, tmp, &sample->touch_points, link) {
		if (point->touch_id == event->touch_id) {
			wl_list_remove(&point->link);
			break;
		}
	}
}

static void touch_motion_notify(struct wl_listener *listener, void *data) {
	struct wlr_event_touch_motion *event = data;
	struct touch_state *tstate = wl_container_of(listener, tstate, motion);
	struct sample_state *sample = tstate->sample;
	struct touch_point *point;
	wl_list_for_each(point, &sample->touch_points, link) {
		if (point->touch_id == event->touch_id) {
			point->x = event->x;
			point->y = event->y;
			break;
		}
	}
}

static void touch_destroy_notify(struct wl_listener *listener, void *data) {
	struct touch_state *tstate = wl_container_of(listener, tstate, destroy);
	wl_list_remove(&tstate->link);
	wl_list_remove(&tstate->destroy.link);
	wl_list_remove(&tstate->down.link);
	wl_list_remove(&tstate->up.link);
	wl_list_remove(&tstate->motion.link);
	free(tstate);
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
	case WLR_INPUT_DEVICE_TOUCH:;
		struct touch_state *tstate = calloc(sizeof(struct touch_state), 1);
		tstate->device = device;
		tstate->sample = sample;
		tstate->destroy.notify = touch_destroy_notify;
		wl_signal_add(&device->events.destroy, &tstate->destroy);
		tstate->down.notify = touch_down_notify;
		wl_signal_add(&device->touch->events.down, &tstate->down);
		tstate->motion.notify = touch_motion_notify;
		wl_signal_add(&device->touch->events.motion, &tstate->motion);
		tstate->up.notify = touch_up_notify;
		wl_signal_add(&device->touch->events.up, &tstate->up);
		wl_list_insert(&sample->touch, &tstate->link);
		break;
	default:
		break;
	}
}


int main(int argc, char *argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);
	struct wl_display *display = wl_display_create();
	struct sample_state state = {
		.display = display
	};
	wl_list_init(&state.touch_points);
	wl_list_init(&state.touch);

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
	state.cat_texture = wlr_texture_from_pixels(state.renderer,
		DRM_FORMAT_ARGB8888, cat_tex.width * 4, cat_tex.width, cat_tex.height,
		cat_tex.pixel_data);
	if (!state.cat_texture) {
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

	wlr_texture_destroy(state.cat_texture);
	wl_display_destroy(display);
}
