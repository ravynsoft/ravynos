#define _XOPEN_SOURCE 600
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

struct sample_state {
	struct wl_display *display;
	struct wl_listener new_output;
	struct wl_listener new_input;
	struct timespec last_frame;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wl_list outputs;
};

struct sample_output {
	struct sample_state *sample;
	struct wlr_output *output;
	struct wl_listener frame;
	struct wl_listener destroy;
	struct wl_list link;
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

	static float rotation = 0.f;
	for (int y = -128; y < height; y += 128) {
		for (int x = -128; x < width; x += 128) {
			struct wlr_box box = {
				.x = x,
				.y = y,
				.width = 128,
				.height = 128,
			};

			float color[] = {
				255.f / x,
				255.f / y,
				255.f / (x + y),
				1.f
			};

			float matrix[9];
			wlr_matrix_project_box(matrix, &box, WL_OUTPUT_TRANSFORM_NORMAL,
					rotation, wlr_output->transform_matrix);

			wlr_render_quad_with_matrix(sample->renderer, color, matrix);
		}
	}

	wlr_renderer_end(sample->renderer);
	wlr_output_commit(wlr_output);

	//TODO rotate with a delta time
	rotation += 0.05;
	if (rotation > 2 * M_PI) {
		rotation = 0.f;
	}

	sample->last_frame = now;
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

	struct wlr_output_mode *mode = wlr_output_preferred_mode(output);
	if (mode != NULL) {
		wlr_output_set_mode(output, mode);
	}

	sample_output->output = output;
	sample_output->sample = sample;
	wl_signal_add(&output->events.frame, &sample_output->frame);
	sample_output->frame.notify = output_frame_notify;
	wl_signal_add(&output->events.destroy, &sample_output->destroy);
	sample_output->destroy.notify = output_remove_notify;
	wl_list_insert(&sample->outputs, &sample_output->link);

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
		struct xkb_keymap *keymap = xkb_map_new_from_names(context, NULL,
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
		.display = display,
	};
	wl_list_init(&state.outputs);

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
		wlr_backend_destroy(wlr);
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
