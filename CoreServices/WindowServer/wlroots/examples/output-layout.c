#define _POSIX_C_SOURCE 200112L
#include <drm_fourcc.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>
#include "cat.h"

struct sample_state {
	struct wl_display *display;
	struct wl_listener new_output;
	struct wl_listener new_input;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wlr_texture *cat_texture;
	struct wlr_output_layout *layout;
	float x_offs, y_offs;
	float x_vel, y_vel;
	struct timespec ts_last;
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

static void animate_cat(struct sample_state *sample,
		struct wlr_output *output) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	long ms = (ts.tv_sec - sample->ts_last.tv_sec) * 1000 +
		(ts.tv_nsec - sample->ts_last.tv_nsec) / 1000000;
	// how many seconds have passed since the last time we animated
	float seconds = ms / 1000.0f;

	if (seconds > 0.1f) {
		// XXX when we switch vt, the rendering loop stops so try to detect
		// that and pause when it happens.
		seconds = 0.0f;
	}

	// check for collisions and bounce
	bool ur_collision = !wlr_output_layout_output_at(sample->layout,
			sample->x_offs + 128, sample->y_offs);
	bool ul_collision = !wlr_output_layout_output_at(sample->layout,
			sample->x_offs, sample->y_offs);
	bool ll_collision = !wlr_output_layout_output_at(sample->layout,
			sample->x_offs, sample->y_offs + 128);
	bool lr_collision = !wlr_output_layout_output_at(sample->layout,
			sample->x_offs + 128, sample->y_offs + 128);

	if (ur_collision && ul_collision && ll_collision && lr_collision) {
		// oops we went off the screen somehow
		struct wlr_output_layout_output *l_output =
			wlr_output_layout_get(sample->layout, output);
		sample->x_offs = l_output->x + 20;
		sample->y_offs = l_output->y + 20;
	} else if (ur_collision && ul_collision) {
		sample->y_vel = fabs(sample->y_vel);
	} else if (lr_collision && ll_collision) {
		sample->y_vel = -fabs(sample->y_vel);
	} else if (ll_collision && ul_collision) {
		sample->x_vel = fabs(sample->x_vel);
	} else if (ur_collision && lr_collision) {
		sample->x_vel = -fabs(sample->x_vel);
	} else {
		if (ur_collision || lr_collision) {
			sample->x_vel = -fabs(sample->x_vel);
		}
		if (ul_collision || ll_collision) {
			sample->x_vel = fabs(sample->x_vel);
		}
		if (ul_collision || ur_collision) {
			sample->y_vel = fabs(sample->y_vel);
		}
		if (ll_collision || lr_collision) {
			sample->y_vel = -fabs(sample->y_vel);
		}
	}

	sample->x_offs += sample->x_vel * seconds;
	sample->y_offs += sample->y_vel * seconds;
	sample->ts_last = ts;
}

static void output_frame_notify(struct wl_listener *listener, void *data) {
	struct sample_output *output = wl_container_of(listener, output, frame);
	struct sample_state *sample = output->sample;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	struct wlr_output *wlr_output = output->output;

	wlr_output_attach_render(wlr_output, NULL);
	wlr_renderer_begin(sample->renderer, wlr_output->width, wlr_output->height);
	wlr_renderer_clear(sample->renderer, (float[]){0.25f, 0.25f, 0.25f, 1});

	animate_cat(sample, output->output);

	struct wlr_box box = {
		.x = sample->x_offs, .y = sample->y_offs,
		.width = 128, .height = 128,
	};
	if (wlr_output_layout_intersects(sample->layout, output->output, &box)) {
		// transform global coordinates to local coordinates
		double local_x = sample->x_offs;
		double local_y = sample->y_offs;
		wlr_output_layout_output_coords(sample->layout, output->output,
			&local_x, &local_y);

		wlr_render_texture(sample->renderer, sample->cat_texture,
			wlr_output->transform_matrix, local_x, local_y, 1.0f);
	}

	wlr_renderer_end(sample->renderer);
	wlr_output_commit(wlr_output);
}

static void update_velocities(struct sample_state *sample,
		float x_diff, float y_diff) {
	sample->x_vel += x_diff;
	sample->y_vel += y_diff;
}

static void output_remove_notify(struct wl_listener *listener, void *data) {
	struct sample_output *sample_output = wl_container_of(listener, sample_output, destroy);
	struct sample_state *sample = sample_output->sample;
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
	wlr_output_layout_add_auto(sample->layout, output);
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
		// NOTE: It may be better to simply refer to our key state during each frame
		// and make this change in pixels/sec^2
		// Also, key repeat
		int delta = 75;
		if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			switch (sym) {
			case XKB_KEY_Left:
				update_velocities(sample, -delta, 0);
				break;
			case XKB_KEY_Right:
				update_velocities(sample, delta, 0);
				break;
			case XKB_KEY_Up:
				update_velocities(sample, 0, -delta);
				break;
			case XKB_KEY_Down:
				update_velocities(sample, 0, delta);
				break;
			}
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
	default:
		break;
	}
}


int main(int argc, char *argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);
	struct wl_display *display = wl_display_create();
	struct sample_state state = {
		.x_vel = 500,
		.y_vel = 500,
		.display = display,
	};

	state.layout = wlr_output_layout_create();
	clock_gettime(CLOCK_MONOTONIC, &state.ts_last);

	struct wlr_backend *wlr = wlr_backend_autocreate(display);
	if (!wlr) {
		exit(1);
	}

	wl_signal_add(&wlr->events.new_output, &state.new_output);
	state.new_output.notify = new_output_notify;
	wl_signal_add(&wlr->events.new_input, &state.new_input);
	state.new_input.notify = new_input_notify;

	state.renderer = wlr_renderer_autocreate(wlr);
	state.cat_texture = wlr_texture_from_pixels(state.renderer,
		DRM_FORMAT_ABGR8888, cat_tex.width * 4, cat_tex.width, cat_tex.height,
		cat_tex.pixel_data);

	state.allocator = wlr_allocator_autocreate(wlr, state.renderer);

	if (!wlr_backend_start(wlr)) {
		wlr_log(WLR_ERROR, "Failed to start backend");
		wlr_backend_destroy(wlr);
		exit(1);
	}
	wl_display_run(display);

	wlr_texture_destroy(state.cat_texture);

	wl_display_destroy(state.display);
	wlr_output_layout_destroy(state.layout);
}
